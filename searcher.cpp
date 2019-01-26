#include "searcher.h"
#include "indexes.h"
#include <fstream>
#include <set>

searcher::searcher(std::string inputString) : inputString(inputString), flagStop(false) {}

bool searcher::findInputStringInFile(std::string &inputString, fs::path &p) {
    std::ifstream file(p);
    if (!file.is_open()) {
        return false;
    }
    std::vector<int> prefInputString(inputString.length(), 0);
    for (size_t i = 1; i < inputString.length(); i++) {
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
    unsigned char buf[SIZE_BUF];
    do {
        int sz = 0;
        try {
            file.read((char *) buf, sizeof(buf));
            sz = (int)file.gcount();
        } catch (...) {
            file.close();
            return false;
        }
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
    std::set<std::tuple<unsigned char, unsigned char, unsigned char>> trigrams;
    for (size_t i = 0; i < inputString.length() - 2; i++) {
        trigrams.insert(std::tuple<unsigned char, unsigned char, unsigned char>(inputString[i], inputString[i + 1], inputString[i + 2]));
    }
    std::vector<size_t> cntTrigrams(ind.mapPaths.size(), 0);
    std::ifstream listFiles(LIST_FILES);
    unsigned char buf[SIZE_BUF];
    size_t isDone = 0;
    for (auto &t : trigrams) {
        if (flagStop) {
            listFiles.close();
            return;
        }
        int pos = ind.allTrigrams[t].first * 2;
        int sz = ind.allTrigrams[t].second * 2;
        try {
            listFiles.seekg(pos, std::ios_base::beg);
            listFiles.read((char *)buf, sz);
        } catch (...) {
            continue;
        }
        for (int i = 0; i < sz; i += 2) {
            size_t num = size_t((buf[i] << 8) + buf[i + 1]);
            if (num < ind.mapPaths.size()) {
                cntTrigrams[num]++;
            }
        }
        isDone++;
        send(fs::path(), int((50.0 * isDone) / trigrams.size()));
    }
    listFiles.close();
    for (size_t i = 0; i < ind.mapPaths.size(); i++) {
        if (flagStop) {
            return;
        }
        if (cntTrigrams[i] == trigrams.size()) {
            fs::path p = ind.mapPaths[(short)i];
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
