#include "indexes.h"
#include <fstream>
#include <set>
#include <tuple>
#include <vector>
#include <deque>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

const size_t SIZE_BUF = (1 << 20); //1048576
const size_t MAX_COUNT_TRIGRAMS = 100000;
const std::string LIST_PAIRS = "listPairs.ind";
const std::string LIST_FILES = "listFiles.ind";

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
            if (buf[i] < 10) {
                file.close();
                return false;
            }
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

void createListFiles(std::map<std::tuple<char, char, char>, std::pair<int, int>> allTrigrams) {
    fs::remove(LIST_FILES);
    std::ofstream listFilesStream(LIST_FILES);
    std::ifstream listPairsStream(LIST_PAIRS);
    char buf[SIZE_BUF * 5];
    int szListFiles = 0;
    do {
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
            szListFiles++;
        }
        listFilesStream.seekp(szListFiles * 2, std::ios_base::beg);
        listFilesStream << char(0);
        //listFilesStream.flush();
        for (auto &t : mapListFiles) {
            listFilesStream.seekp(allTrigrams[t.first].first * 2, std::ios_base::beg);
            for (auto &el : t.second) {
                allTrigrams[t.first].first++;
                listFilesStream << char(el >> 8) << char(el & ((1 << 8) - 1));
                //listFilesStream.flush();
            }
        }
    } while (listPairsStream);
    listFilesStream.close();
    listPairsStream.close();
}

bool findInputStringInFile(std::string &inputString, fs::path p) {
    std::ifstream file(p);
    if (!file.is_open()) {
        return false;
    }
    std::vector<int> prefInputString(inputString.length(), 0);
    for (int i = 1; i <inputString.length(); i++) {
        int k = prefInputString[i - 1];
        while (k > 0 && inputString[i] != inputString[k]) {
            k = prefInputString[k - 1];
        }
        if (inputString[i] == inputString[k]) {
            k++;
        }
        prefInputString[i] = k;
    }
    int prevPref = 0;
    int ind = 0;
    char buf[SIZE_BUF];
    do {
        file.read(buf, sizeof(buf));
        int sz = (int)file.gcount();
        for (int i = 0; i < sz; i++, ind++) {
            int k = prevPref;
            while (k > 0 && buf[i] != inputString[k]) {
                k = prefInputString[k - 1];
            }
            if (buf[i] == inputString[k]) {
                k++;
            }
            prevPref = k;
            if (prevPref == (int)inputString.length()) {
                file.close();
                return true;
            }
        }
    } while (file);
    file.close();
    return false;
}

void indexes::buildIndex(std::string &directory) {
    std::vector<fs::path> paths;
    getListFiles(paths, directory);
    std::ofstream listPairsStream(LIST_PAIRS);
    short num = 0;
    allTrigrams.clear();
    mapPaths.clear();
    for (auto &p : paths) {
        if (createFileIndex(listPairsStream, p, num, allTrigrams, mapPaths)) {
            num++;
        }
    }
    listPairsStream.close();
    int cnt = 0;
    for (auto &t : allTrigrams) {
        t.second.first = cnt;
        cnt += t.second.second;
    }
    createListFiles(allTrigrams);
    indexes &ind = indexes::instance();
    ind.directory = directory;
    ind.lastChange = fs::last_write_time(directory);
}


void indexes::findInputString(std::string &inputString, std::vector<fs::path> &paths) {
    std::set<std::tuple<char, char, char>> trigrams;
    for (size_t i = 0; i < inputString.length() - 2; i++) {
        trigrams.insert(std::tuple<char, char, char>(inputString[i], inputString[i + 1], inputString[i + 2]));
    }
    std::vector<int> cntTrigrams(1 << 16, 0);
    std::ifstream listFiles(LIST_FILES);
    char buf[SIZE_BUF];
    for (auto &t : trigrams) {
        int pos = allTrigrams[t].first * 2;
        int sz = allTrigrams[t].second * 2;
        listFiles.seekg(pos, std::ios_base::beg);
        listFiles.read(buf, sz);
        for (int i = 0; i < sz; i += 2) {
            cntTrigrams[int((buf[i] << 8) + buf[i + 1])]++;
        }
    }
    for (size_t i = 0; i < (1 << 16); i++) {
        if (cntTrigrams[i] == (inputString.length() - 2)) {
            fs::path p = mapPaths[i];
            std::vector<int> pos;
            if (findInputStringInFile(inputString, p)) {
                paths.push_back(p);
            }
        }
    }
}


