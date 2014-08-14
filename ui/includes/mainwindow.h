#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QtWidgets/QMainWindow>
#include <boost/thread.hpp>
#include "pollerthread.h"

//namespace isis{

class Ui_MainWindow;
class AboutDialog;


class MainWindow : public QMainWindow
{
    typedef std::unique_ptr<boost::thread> pThread;
    Q_OBJECT

public:
    MainWindow(int argc, char** argv, QWidget *parent = 0);
    virtual ~MainWindow();

private slots:
    void updateImage(const QImage&);

private:
    Ui_MainWindow*  ui_;
    AboutDialog *   aboutDialog_;
    PollerThread*   pPoller_;

    pThread         pIsisThread_;

};

//}
#endif // MAINWINDOW_H
