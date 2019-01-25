#include "searcher.h"
#include "indexes.h"
#include <fstream>
#include <set>

searcher::searcher(std::string inputString) : inputString(inputString), flagStop(false) {}

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

void searcher::doWork() {
    indexes &ind = indexes::instance();
    std::set<std::tuple<char, char, char>> trigrams;
    for (size_t i = 0; i < inputString.length() - 2; i++) {
        trigrams.insert(std::tuple<char, char, char>(inputString[i], inputString[i + 1], inputString[i + 2]));
    }
    std::vector<int> cntTrigrams(ind.mapPaths.size(), 0);
    std::ifstream listFiles(LIST_FILES);
    char buf[SIZE_BUF];
    size_t isDone = 0;
    for (auto &t : trigrams) {
        if (flagStop) {
            return;
        }
        int pos = ind.allTrigrams[t].first * 2;
        int sz = ind.allTrigrams[t].second * 2;
        listFiles.seekg(pos, std::ios_base::beg);
        listFiles.read(buf, sz);
        for (int i = 0; i < sz; i += 2) {
            size_t num = size_t((buf[i] << 8) + buf[i + 1]);
            if (num < ind.mapPaths.size()) {
                cntTrigrams[num]++;
            }
        }
        isDone++;
        send(fs::path(), int((50.0 * isDone) / trigrams.size()));
    }
    for (size_t i = 0; i < ind.mapPaths.size(); i++) {
        if (flagStop) {
            return;
        }
        if (cntTrigrams[i] == (inputString.length() - 2)) {
            fs::path p = ind.mapPaths[i];
            std::vector<int> pos;
            if (findInputStringInFile(inputString, p)) {
                send(p, 50 + int((50.0 * i) / ind.mapPaths.size()));
            }
        }
    }
    send(fs::path(), 100);
}

void searcher::toStop() {
    flagStop = true;
}
