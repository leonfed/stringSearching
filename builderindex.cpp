#include "builderindex.h"
#include "indexes.h"
#include <fstream>
#include <set>

builderIndex::builderIndex(std::string directory) : directory(directory), flagStop(false) {}

void builderIndex::getListFiles(std::vector<fs::path> &paths, const std::string &directory) {
    for(const auto &p: fs::recursive_directory_iterator(directory)) {
        if (!fs::is_directory(p.path()) && !fs::is_symlink(p.path())) {
            paths.push_back(p.path());
        }
    }
}

bool builderIndex::createFileIndex(std::ofstream &listPairsStream, fs::path &p, short num, std::map<std::tuple<unsigned char, unsigned char, unsigned char>, std::pair<int, int>> &allTrigrams, std::map<short, fs::path> &mapPaths) {
    std::ifstream file(p);
    if (!file.is_open()) {
        file.close();
        return false;
    }
    unsigned char buf[SIZE_BUF];
    std::set<std::tuple<unsigned char, unsigned char, unsigned char>> trigrams;
    unsigned char firstChar = 0, secondChar = 0;
    bool flagFirstChar = false, flagSecondChar = false;
    do {
        int sz = 0;
        try {
            file.read((char *)buf, sizeof(buf));
            sz = (int)file.gcount();
        } catch (...) {
            file.close();
            return false;
        }
        if (flagFirstChar) {
            trigrams.insert(std::tuple<unsigned char, unsigned char, unsigned char>(firstChar, secondChar, buf[0]));
        }
        if (flagSecondChar && sz > 1) {
            trigrams.insert(std::tuple<unsigned char, unsigned char, unsigned char>(secondChar, buf[0], buf[1]));
        }
        flagFirstChar = flagSecondChar = false;
        for (int i = 0; i < sz - 2; i++) {
            trigrams.insert(std::tuple<unsigned char, unsigned char, unsigned char>(buf[i], buf[i + 1], buf[i + 2]));
        }
        if (sz > 1) {
            flagSecondChar = true;
            secondChar = buf[sz - 1];
        }
        if (sz > 2) {
            flagFirstChar = true;
            firstChar = buf[sz - 2];
        }
        if (trigrams.size() > MAX_COUNT_TRIGRAMS) {
            file.close();
            return false;
        }
    } while (file);
    file.close();
    mapPaths[num] = p.string();
    for (auto &t : trigrams) {
        if (allTrigrams.find(t) == allTrigrams.end()) {
            allTrigrams[t] = {0, 1};
        } else {
            allTrigrams[t].second++;
        }
        listPairsStream << std::get<0>(t) << std::get<1>(t) << std::get<2>(t) << (unsigned char)(num >> 8) << (unsigned char)(num & ((1 << 8) - 1));
    }
    return true;
}

void builderIndex::createListFiles(std::map<std::tuple<unsigned char, unsigned char, unsigned char>, std::pair<int, int>> allTrigrams) { //allTrigrams is copied, because is changed in this function
    fs::remove(LIST_FILES);
    std::ofstream listFilesStream(LIST_FILES);
    std::ifstream listPairsStream(LIST_PAIRS);
    unsigned char buf[SIZE_BUF * 5];
    size_t szListFiles = 0, isDone = 0;
    for (auto e : allTrigrams) {
        szListFiles += e.second.second;
    }
    listFilesStream.seekp(szListFiles * 2, std::ios_base::beg);
    do {
        if (flagStop) {
            listFilesStream.close();
            listPairsStream.close();
            return;
        }
        std::map<std::tuple<unsigned char, unsigned char, unsigned char>, std::vector<short>> mapListFiles;
        int sz = 0;
        try {
            listPairsStream.read((char *)buf, sizeof(buf));
            sz = (int)listPairsStream.gcount();
        } catch (...) {
            listFilesStream.close();
            listPairsStream.close();
            return;
        }
        for (int i = 0; i < sz - 4; i += 5) {
            std::tuple<unsigned char, unsigned char, unsigned char> trig = std::tuple<unsigned char, unsigned char, unsigned char>(buf[i], buf[i + 1], buf[i + 2]);
            short num = (short)((buf[i + 3] << 8) + buf[i + 4]);
            if (mapListFiles.find(trig) == mapListFiles.end()) {
                mapListFiles[trig] = std::vector<short>(1, num);
            } else {
                mapListFiles[trig].push_back(num);
            }
        }
        listFilesStream << (unsigned char)0;
        for (auto &t : mapListFiles) {
            listFilesStream.seekp(allTrigrams[t.first].first * 2, std::ios_base::beg);
            for (auto &el : t.second) {
                allTrigrams[t.first].first++;
                listFilesStream << (unsigned char)(el >> 8) << (unsigned char)(el & ((1 << 8) - 1));
            }
            isDone += t.second.size();
        }
        send(50 + int((50.0 * isDone) / szListFiles));
    } while (listPairsStream);
    listFilesStream.close();
    listPairsStream.close();
}

void builderIndex::doWork() {
    indexes &ind = indexes::instance();
    std::vector<fs::path> paths;
    getListFiles(paths, directory);
    std::ofstream listPairsStream(LIST_PAIRS);
    short num = 0;
    ind.allTrigrams.clear();
    ind.mapPaths.clear();
    for (size_t i = 0; i < paths.size(); i++) {
        if (flagStop) {
            listPairsStream.close();
            return;
        }
        if (createFileIndex(listPairsStream, paths[i], num, ind.allTrigrams, ind.mapPaths)) {
            num++;
        }
        send(int((50.0 * i) / paths.size()));
    }
    listPairsStream.close();
    int cnt = 0;
    for (auto &t : ind.allTrigrams) {
        t.second.first = cnt;
        cnt += t.second.second;
    }
    createListFiles(ind.allTrigrams);
    ind.directory = directory;
    ind.lastChange = fs::last_write_time(directory);
    send(100);
}

void builderIndex::toStop() {
    flagStop = true;
}















