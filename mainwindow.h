#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <vector>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

Q_DECLARE_METATYPE(fs::path)


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum enumProcessing {NOP, BUILD, SEARCH, BUILD_SEARCH};

    void addListFiles(fs::path &p);

    void callBuilder(std::string &directory, enumProcessing flag = BUILD);

    void callSearcher(std::string &inputString);

private slots:
    void on_actionIndex_triggered();

    void on_actionOpenDirectory_triggered();

    void on_actionOpenFile_triggered();

    void on_actionRun_triggered();

    void callbackBuilder(int);

    void callbackSearcher(fs::path, int);

    void on_actionStop_triggered();

signals:
    void intentStop();

private:
    Ui::MainWindow *ui;

    QProgressBar* bar;
    enumProcessing flagProcessing;
};

#endif // MAINWINDOW_H
