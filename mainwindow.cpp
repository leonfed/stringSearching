#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "indexes.h"
#include "builderindex.h"
#include "searcher.h"
#include <QFileDialog>
#include <QDesktopServices>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    flagProcessing(false)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::fillListFiles(std::vector<fs::path> paths) {
    ui->listFiles->clear();
    for (auto &p : paths) {
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/icons/file"), QString::fromStdString(p));
        ui->listFiles->addItem(item);
    }
}

void MainWindow::callbackBuilder() {
    ui->statusString->setText("Done");
    flagProcessing = false;
}

void MainWindow::callbackSearcher(std::vector<fs::path> paths) {
    fillListFiles(paths);
    ui->statusString->setText("Done");
    flagProcessing = false;
}

void MainWindow::callBuilder(std::string &directory) {
    ui->listFiles->clear();
    ui->statusString->setText("Calculating indexes");
    QThread *thread = new QThread;
    builderIndex *builder = new builderIndex(directory);
    builder->moveToThread(thread);
    connect(builder, SIGNAL(send()), this, SLOT(callbackBuilder()));
    connect(thread, SIGNAL(started()), builder, SLOT(doWork()));
    connect(this, SIGNAL(intentStop()), builder, SLOT(toStop()), Qt::DirectConnection);
    flagProcessing = true;
    thread->start();
}

void MainWindow::callSearcher(std::string &inputString) {
    ui->listFiles->clear();
    ui->statusString->setText("Searching");
    QThread *thread = new QThread;
    searcher *search = new searcher(inputString);
    search->moveToThread(thread);
    qRegisterMetaType<std::vector<fs::path>>("Type");
    connect(search, SIGNAL(send(Type)), this, SLOT(callbackSearcher(Type)));
    connect(thread, SIGNAL(started()), search, SLOT(doWork()));
    connect(this, SIGNAL(intentStop()), search, SLOT(toStop()), Qt::DirectConnection);
    flagProcessing = true;
    thread->start();
}

void MainWindow::on_actionBrowse_triggered() {
    if (flagProcessing) {
        return;
    }
    QString Qdirectory;
    std::string directory;
    try {
        Qdirectory = QFileDialog::getExistingDirectory(nullptr, "Directory Dialog", "");
        directory = Qdirectory.toStdString();
        if (Qdirectory != "") {
            ui->directory->setText(Qdirectory);
        }
        indexes &ind = indexes::instance();
        if (ind.directory == directory && ind.lastChange == fs::last_write_time(directory)) {
            return;
        }
        callBuilder(directory);
    } catch (...) {
        ui->directory->setText("Directory isn't selected");
    }
}

void MainWindow::on_actionRun_triggered() {
    if (flagProcessing) {
        return;
    }
    std::string inputString = ui->inputString->text().toStdString();
    if (inputString.length() <= 2) {
        return;
    }
    std::vector<fs::path> paths;
    indexes &ind = indexes::instance();
    if (!ind.directory.empty() && ind.lastChange < fs::last_write_time(ind.directory)) {
        callBuilder(ind.directory);
    }
    callSearcher(inputString);
}

void MainWindow::on_actionOpenDirectory_triggered() {
    if (flagProcessing || !ui->listFiles->currentItem()) {
        return;
    }
    fs::path filePath = ui->listFiles->currentItem()->text().toStdString();
    fs::path directoryPath = filePath.parent_path();
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(directoryPath)));
}

void MainWindow::on_actionOpenFile_triggered() {
    if (flagProcessing || !ui->listFiles->currentItem()) {
        return;
    }
    fs::path filePath = ui->listFiles->currentItem()->text().toStdString();
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(filePath)));
}

void MainWindow::on_actionStop_triggered() {
    emit intentStop();
    ui->statusString->setText("Stopped");
    flagProcessing = false;
}
