#ifndef INDEXES_H
#define INDEXES_H

#include <string>
#include <map>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

const size_t SIZE_BUF = (1 << 20); //1048576
const size_t MAX_COUNT_TRIGRAMS = 100000;
const std::string LIST_PAIRS = "listPairs.ind";
const std::string LIST_FILES = "listFiles.ind";

class indexes {
public:
    static indexes& instance() {
        static indexes singleton;
        return singleton;
    }

    std::string directory;
    fs::file_time_type lastChange;
    std::map<std::tuple<unsigned char, unsigned char, unsigned char>, std::pair<int, int>> allTrigrams;
    std::map<short, fs::path> mapPaths;

    void clear() {
        directory.clear();
        allTrigrams.clear();
        mapPaths.clear();
    }

private:
    indexes() {}                                  // Private constructor
    ~indexes() {}
    indexes(const indexes&);                 // Prevent copy-construction
    indexes& operator=(const indexes&);      // Prevent assignment
};

#endif // INDEXES_H
