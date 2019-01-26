#ifndef SEARCHER_H
#define SEARCHER_H

#include <QObject>
#include <vector>
#include <string>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

class searcher : public QObject
{
    Q_OBJECT
public:
    searcher(std::string inputString);
public slots:
    void doWork();
    void toStop();
signals:
    void send(fs::path, int);
private:
    bool findInputStringInFile(std::string &inputString, fs::path &p);

    std::string inputString;
    bool flagStop;
};

#endif // SEARCHER_H
