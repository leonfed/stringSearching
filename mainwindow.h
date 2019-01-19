#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

Q_DECLARE_METATYPE(std::vector<fs::path>)


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void fillListFiles(std::vector<fs::path>);

    void callBuilder(std::string &directory);

    void callSearcher(std::string &inputString);

private slots:
    void on_actionBrowse_triggered();

    void on_actionOpenDirectory_triggered();

    void on_actionOpenFile_triggered();

    void on_actionRun_triggered();

    void callbackBuilder();

    void callbackSearcher(std::vector<fs::path>);

    void on_actionStop_triggered();

private:
    Ui::MainWindow *ui;
    bool flagProcessing;
};

#endif // MAINWINDOW_H
