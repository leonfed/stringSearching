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
    void createListFiles(std::map<std::tuple<char, char, char>, std::pair<int, int>> allTrigrams);

public slots:
    void doWork();
    void toStop();

signals:
    void send(int);

private:
    std::string directory;
    bool flagStop;

};

#endif // BUILDERINDEX_H
