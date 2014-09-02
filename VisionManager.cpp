#include "VisionManager.h"
#include "CamInstance.h"
#include "trace.h"
#include "Isis.h"
#include "Utility.h"

using namespace Utility;
using namespace isis;

VisionManager::VisionManager()
    : Application("VisionManager")
{

}

void VisionManager::addCAM(boost::shared_ptr<cv::VideoCapture> cam)
{
    mVisionObjects.push_back(new CamInstance(mVisionObjects.size(),
                                             cam));

    boost::posix_time::microsec interval(50); //just in case?
    boost::this_thread::sleep(interval);

    mAIObjects.push_back(new AI(mAIObjects.size(),
                                mVisionObjects.back()->getQueue()));
}

CamInstance* VisionManager::getCAMInstance(unsigned int index) const{
    if(mVisionObjects[index]!=nullptr){
        return mVisionObjects[index];
    }else{
        return nullptr;
    }
}

AI* VisionManager::getAIInstance(unsigned int index) const{
    if(mAIObjects[index]!=nullptr){
        return mAIObjects[index];
    }else{
        return nullptr;
    }
}

void VisionManager::CreateStorageDirectories()
{
    if(!create_directory("img")) log()->error("Error creating img dir");
    if(!create_directory("video")) log()->error("Error creating video dir");
}

void VisionManager::initialize(){
    CreateStorageDirectories();

    if(mVisionObjects.size() && mAIObjects.size()){
        for(auto v : mVisionObjects){
            mThreads.create_thread(boost::bind(&CamInstance::RunApp, v));
        }
        for(auto a : mAIObjects){
            mThreads.create_thread(boost::bind(&AI::RunApp, a));
        }
        log()->info("Threads created successfully. Proceeding to join");
        mThreads.join_all();
    }else throw std::runtime_error("Error, no valid vision and ai threads defined");
}

void VisionManager::Run(){
    initialize();
    while(Isis::isRunning){
        boost::posix_time::seconds workTime(5);
        boost::this_thread::sleep(workTime);
    }
}

VisionManager::~VisionManager() { // Destructor
    mThreads.interrupt_all();
    mThreads.join_all();
}
