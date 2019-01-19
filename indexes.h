#ifndef INDEXES_H
#define INDEXES_H

#include <string>
#include <map>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

class indexes {
public:
    static indexes& instance() {
        static indexes singleton;
        return singleton;
    }

    void buildIndex(std::string &directory);
    void findInputString(std::string &inputString, std::vector<fs::path> &paths);
    std::string directory;
    fs::file_time_type lastChange;

private:
    indexes() {}                                  // Private constructor
    ~indexes() {}
    indexes(const indexes&);                 // Prevent copy-construction
    indexes& operator=(const indexes&);      // Prevent assignment

    std::map<std::tuple<char, char, char>, std::pair<int, int>> allTrigrams;
    std::map<short, fs::path> mapPaths;
};

#endif // INDEXES_H
