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
    flagProcessing(NOP)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::addListFiles(fs::path &p) {
    if (p.empty()) {
        return;
    }
    QListWidgetItem *item = new QListWidgetItem(QIcon(":/icons/file"), QString::fromStdString(p));
    ui->listFiles->addItem(item);
}

void MainWindow::callbackBuilder(int progressValue) {
    bar->setValue(progressValue);
    if (progressValue == 100) {
        ui->statusBar->removeWidget(bar);
        ui->statusString->setText("Done");
        if (flagProcessing == BUILD_SEARCH) {
            flagProcessing = NOP;
            on_actionRun_triggered();
        } else {
            flagProcessing = NOP;
        }
    }
}

void MainWindow::callbackSearcher(fs::path p, int progressValue) {
    addListFiles(p);
    bar->setValue(progressValue);
    if (progressValue == 100) {
        ui->statusBar->removeWidget(bar);
        ui->statusString->setText("Done");
        flagProcessing = NOP;
    }
}

void MainWindow::callBuilder(std::string &directory, enumProcessing flag) {
    bar = new QProgressBar;
    ui->statusBar->addWidget(bar);
    bar->setValue(0);
    ui->listFiles->clear();
    ui->statusString->setText("Calculating indexes");
    QThread *thread = new QThread;
    builderIndex *builder = new builderIndex(directory);
    builder->moveToThread(thread);
    connect(builder, SIGNAL(send(int)), this, SLOT(callbackBuilder(int)));
    connect(thread, SIGNAL(started()), builder, SLOT(doWork()));
    connect(this, SIGNAL(intentStop()), builder, SLOT(toStop()), Qt::DirectConnection);
    flagProcessing = flag;
    thread->start();
}

void MainWindow::callSearcher(std::string &inputString) {
    bar = new QProgressBar;
    ui->statusBar->addWidget(bar);
    bar->setValue(0);
    ui->listFiles->clear();
    ui->statusString->setText("Searching");
    QThread *thread = new QThread;
    searcher *search = new searcher(inputString);
    search->moveToThread(thread);
    qRegisterMetaType<fs::path>("Type");
    connect(search, SIGNAL(send(Type, int)), this, SLOT(callbackSearcher(Type, int)));
    connect(thread, SIGNAL(started()), search, SLOT(doWork()));
    connect(this, SIGNAL(intentStop()), search, SLOT(toStop()), Qt::DirectConnection);
    flagProcessing = SEARCH;
    thread->start();
}

void MainWindow::on_actionIndex_triggered() {
    if (flagProcessing != NOP) {
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
        if (directory.empty() || (ind.directory == directory && ind.lastChange == fs::last_write_time(directory))) {
            return;
        }
        callBuilder(directory);
    } catch (...) {
        ui->directory->setText("Directory isn't selected");
        ui->statusString->setText("Can't index directory");
    }
}

void MainWindow::on_actionRun_triggered() {
    std::string inputString = ui->inputString->text().toStdString();
    indexes &ind = indexes::instance();
    if (flagProcessing != NOP || inputString.length() <= 2 || ind.directory.empty()) {
        return;
    }
    try {
        if (ind.lastChange < fs::last_write_time(ind.directory)) {
            callBuilder(ind.directory, BUILD_SEARCH);
        } else {
            callSearcher(inputString);
        }
    } catch (...) {
        ui->statusString->setText("Can't find");
    }
}

void MainWindow::on_actionOpenDirectory_triggered() {
    if (flagProcessing != NOP || !ui->listFiles->currentItem()) {
        return;
    }
    try {
        fs::path filePath = ui->listFiles->currentItem()->text().toStdString();
        fs::path directoryPath = filePath.parent_path();
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(directoryPath)));
    } catch (...) {
        ui->statusString->setText("Can't open directory");
    }
}

void MainWindow::on_actionOpenFile_triggered() {
    if (flagProcessing != NOP || !ui->listFiles->currentItem()) {
        return;
    }
    try {
        fs::path filePath = ui->listFiles->currentItem()->text().toStdString();
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(filePath)));
    } catch (...) {
        ui->statusString->setText("Can't open file");
    }
}

void MainWindow::on_actionStop_triggered() {
    emit intentStop();
    ui->statusString->setText("Stopped");
    if (flagProcessing == BUILD) {
        indexes &ind = indexes::instance();
        ind.clear();
        ui->directory->setText("Directory isn't selected");
    }
    flagProcessing = NOP;
    ui->statusBar->removeWidget(bar);
}
