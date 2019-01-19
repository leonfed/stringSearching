#include "builderindex.h"
#include "indexes.h"

builderIndex::builderIndex(std::string directory) : directory(directory) {}

void builderIndex::doWork() {
    indexes &ind = indexes::instance();
    ind.buildIndex(directory);
    emit send();
}
