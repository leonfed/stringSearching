#include "builderindex.h"
#include "indexes.h"
#include <fstream>
#include <set>

builderIndex::builderIndex(std::string directory) : directory(directory), flagStop(false) {}

void getListFiles(std::vector<fs::path> &paths, const std::string &directory) {
    for(const auto &p: fs::recursive_directory_iterator(directory)) {
        if (!fs::is_directory(p.path()) && !fs::is_symlink(p.path())) {
            paths.push_back(p.path());
        }
    }
}

bool createFileIndex(std::ofstream &listPairsStream, fs::path &p, short num, std::map<std::tuple<char, char, char>, std::pair<int, int>> &allTrigrams, std::map<short, fs::path> &mapPaths) {
    std::ifstream file(p);
    if (!file.is_open()) {
        return false;
    }
    char buf[SIZE_BUF];
    std::set<std::tuple<char, char, char>> trigrams;
    char firstChar = 0, secondChar = 0;
    do {
        file.read(buf, sizeof(buf));
        int sz = (int)file.gcount();
        if (firstChar != 0) {
            trigrams.insert(std::tuple<char, char, char>(firstChar, secondChar, buf[0]));
        }
        if (secondChar != 0 && sz > 1) {
            trigrams.insert(std::tuple<char, char, char>(secondChar, buf[0], buf[1]));
        }
        firstChar = secondChar = 0;
        for (int i = 0; i < sz - 2; i++) {
            trigrams.insert(std::tuple<char, char, char>(buf[i], buf[i + 1], buf[i + 2]));
        }
        if (sz > 1) {
            secondChar = buf[sz - 1];
        }
        if (sz > 2) {
            firstChar = buf[sz - 2];
        }
    } while (file);
    file.close();
    if (trigrams.size() > MAX_COUNT_TRIGRAMS) {
        return false;
    }
    mapPaths[num] = p.string();
    for (auto &t : trigrams) {
        if (allTrigrams.find(t) == allTrigrams.end()) {
            allTrigrams[t] = {0, 1};
        } else {
            allTrigrams[t].second++;
        }
        listPairsStream << std::get<0>(t) << std::get<1>(t) << std::get<2>(t) << char(num >> 8) << char(num & ((1 << 8) - 1));
    }
    return true;
}

void builderIndex::createListFiles(std::map<std::tuple<char, char, char>, std::pair<int, int>> allTrigrams) {
    fs::remove(LIST_FILES);
    std::ofstream listFilesStream(LIST_FILES);
    std::ifstream listPairsStream(LIST_PAIRS);
    char buf[SIZE_BUF * 5];
    size_t szListFiles = 0, isDone = 0;
    for (auto e : allTrigrams) {
        szListFiles += e.second.second;
    }
    listFilesStream.seekp(szListFiles * 2, std::ios_base::beg);
    do {
        if (flagStop) {
            return;
        }
        std::map<std::tuple<char, char, char>, std::vector<short>> mapListFiles;
        listPairsStream.read(buf, sizeof(buf));
        int sz = (int)listPairsStream.gcount();
        for (int i = 0; i < sz - 4; i += 5) {
            std::tuple<char, char, char> trig = std::tuple<char, char, char>(buf[i], buf[i + 1], buf[i + 2]);
            short num = (buf[i + 3] << 8) + buf[i + 4];
            if (mapListFiles.find(trig) == mapListFiles.end()) {
                mapListFiles[trig] = std::vector<short>(1, num);
            } else {
                mapListFiles[trig].push_back(num);
            }
        }
        listFilesStream << char(0);
        //listFilesStream.flush();
        for (auto &t : mapListFiles) {
            listFilesStream.seekp(allTrigrams[t.first].first * 2, std::ios_base::beg);
            for (auto &el : t.second) {
                allTrigrams[t.first].first++;
                listFilesStream << char(el >> 8) << char(el & ((1 << 8) - 1));
                //listFilesStream.flush();
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
