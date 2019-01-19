#include "searcher.h"
#include "indexes.h"

searcher::searcher(std::string inputString) : inputString(inputString) {}

void searcher::doWork() {
    std::vector<fs::path> paths;
    indexes &ind = indexes::instance();
    ind.findInputString(inputString, paths);
    send(paths);
}
