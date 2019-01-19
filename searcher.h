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
signals:
    void send(std::vector<fs::path>);
private:
    std::string inputString;
};

#endif // SEARCHER_H
