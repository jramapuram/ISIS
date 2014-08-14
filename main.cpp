#include <stdexcept>
#include <boost/exception/all.hpp>
#include "mainwindow.h"

int main(int argc, char** argv) {
  try {

    auto pQapp        = new QApplication(argc,argv);
    auto pQMainWindow = new MainWindow(argc,argv);
    pQapp->connect(pQapp, SIGNAL(lastWindowClosed()),    // Now wait for application to finish
                   pQapp, SLOT(quit()));
    pQMainWindow->showNormal();
    pQapp->exec();// MUST be called by the Main Thread

  } catch (const std::exception& e) {
    std::cerr << "Uncaught exception in main(): " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception in main()" << std::endl;
    return 1;
  }
  return 0;
}
