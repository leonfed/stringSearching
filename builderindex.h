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

signals:
    void send();

private:
    std::string directory;

};

#endif // BUILDERINDEX_H
