#include "includes/pollerthread.h"
#include "VisionManager.h"
#include "QtOpenCV.h"

PollerThread::PollerThread(QObject *parent) :
    QThread(parent)
{
    if(!isRunning()){
        start(LowPriority);
    }
}

PollerThread::~PollerThread(){

}

void PollerThread::run(){
    QThread::sleep(5); //XX fix this shit

    bool isRunning_ = true;

    while(isRunning_){
        auto pVisionManager = isis::VisionManager::instance();
        if(pVisionManager != nullptr){
            auto pCamera = pVisionManager->getCAMInstance(0);
            if(pCamera != nullptr){
                QImage imCur = cvtCvMat2QImage(pCamera->getImage());
                emit pollImage(imCur);

                isRunning_ = isis::Isis::isRunning && isRunning();
            }
        }
        QThread::msleep(66); //2x sampling
    }
}
