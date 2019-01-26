#ifndef BUILDERINDEX_H
#define BUILDERINDEX_H

#include <QObject>
#include <vector>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

class builderIndex : public QObject
{
    Q_OBJECT

public:

    builderIndex(std::string directory);

public slots:
    void doWork();
    void toStop();

signals:
    void send(int);

private:
    void getListFiles(std::vector<fs::path> &paths, const std::string &directory);
    void createListFiles(std::map<std::tuple<unsigned char, unsigned char, unsigned char>, std::pair<int, int>> allTrigrams);
    bool createFileIndex(std::ofstream &listPairsStream, fs::path &p, short num, std::map<std::tuple<unsigned char, unsigned char, unsigned char>, std::pair<int, int>> &allTrigrams, std::map<short, fs::path> &mapPaths);

    std::string directory;
    bool flagStop;

};

#endif // BUILDERINDEX_H
