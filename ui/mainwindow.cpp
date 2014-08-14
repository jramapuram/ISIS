#include "includes/mainwindow.h"
#include "ui_mainwindow.h"
#include "Isis.h"

MainWindow::MainWindow(int argc, char **argv, QWidget *parent)
    : QMainWindow(parent)
{
    ui_ = new Ui_MainWindow();
    ui_->setupUi(this);
    ui_->qMainLabel->show();

    pPoller_ = new PollerThread(this);
    connect(  pPoller_
            , SIGNAL(pollImage(QImage))
            , this
            , SLOT(updateImage(QImage)));

    auto pIsis = new isis::Isis(argc,argv);
    pIsisThread_.reset(new boost::thread(&isis::Isis::initialize,pIsis));
}

void MainWindow::updateImage(const QImage& img){
    ui_->qMainLabel->setPixmap(QPixmap::fromImage(img));
}

MainWindow::~MainWindow()
{
    pIsisThread_->interrupt();
    delete ui_;
}
