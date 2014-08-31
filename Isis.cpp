#include "Isis.h"
#include "trace.h"
#include "VisionManager.h"
#include "speech.h"
#include "COM.h"
#include <boost/thread.hpp>

using namespace isis;

/**************STATIC DEFNS****************/
trace*               Isis::logging;
Config*              Isis::config;
LogModule*           Isis::main;
boost::thread_group  Isis::IsisThreads;
bool                 Isis::isRunning;
/**************STATIC DEFNS****************/

int Isis::CreateThreads() {
    //object definitions
    speech* listen      =   speech::instance();
    COM* communication  =   COM::instance();

    cv::startWindowThread(); //necessary to fix window close issue
    VisionManager* vision     =   VisionManager::instance();

    if(Isis::config->singleCam == -1){
        for (int i = 0; i < Isis::config->numCams; i++) {
            boost::shared_ptr<cv::VideoCapture> cvInputObj(new cv::VideoCapture(i));
            if (cvInputObj->isOpened()) vision->addCAM(cvInputObj);
            else Isis::main->error("Error opening camera [%d]", i);
        }
    }else {
        boost::shared_ptr<cv::VideoCapture> cvInputObj(new cv::VideoCapture(Isis::config->singleCam));
        if (cvInputObj->isOpened()) vision->addCAM(cvInputObj);
        else Isis::main->error("Error opening single camera [%d]", Isis::config->singleCam);
    }

    IsisThreads.create_thread(boost::bind(&speech::RunApp, listen));
    IsisThreads.create_thread(boost::bind(&COM::RunApp, communication));
    IsisThreads.create_thread(boost::bind(&VisionManager::RunApp, vision));
    //IsisThreads.join_all();
    return 0;
}

void Isis::initialize(){

    Isis::isRunning = true;

    CreateThreads();

    std::vector<int> blockedSignals = { SIGABRT, SIGSEGV, SIGCHLD };
    std::vector<int> caughtSignals  = { SIGHUP, SIGQUIT, SIGINT, SIGABRT, SIGTERM, SIGCHLD };

    sigset_t sigsToBlock;
    sigset_t sigsToCatch;
    sigemptyset(&sigsToCatch);
    sigfillset(&sigsToBlock);

    for (auto s : blockedSignals) {
      sigdelset(&sigsToBlock, s);
    }
    for (auto s : caughtSignals) {
      sigaddset(&sigsToCatch, s);
    }

    pthread_sigmask(SIG_BLOCK, &sigsToBlock, 0);
    Isis::main->info("entering signal handler");
    while (true) {
        int caught = 0;
        sigwait(&sigsToCatch, &caught);
        if (caught > 0) {
            IsisThreads.interrupt_all();
            break;
        }
    }
}

Isis::Isis(int argc, char* argv[])
    : argc_(argc)
    , argv_(argv)
{

    pConfig_  = std::unique_ptr<Config>(new Config(argc_, argv_));
    Isis::config  = pConfig_.get();

    pLogging_ = std::unique_ptr<trace>(new trace());
    Isis::logging = pLogging_.get();

    pMain_    = std::unique_ptr<LogModule>(Isis::logging->getModule("main"));
    Isis::main    = pMain_.get();

    Isis::main->info("Starting Isis v0.0.1  ");
    Isis::main->info(".___       .__        ");
    Isis::main->info("|   | _____|__| ______");
    Isis::main->info("|   |/  ___/  |/  ___/");
    Isis::main->info("|   |\\___ \\|  |\\___ \\ ");
    Isis::main->info("|___/____  >__/____  >");
    Isis::main->info("\\/        \\/          ");
}
