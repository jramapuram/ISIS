#ifndef VISIONMANAGER_H_
#define VISIONMANAGER_H_

#ifdef __APPLE__
#define ISMAC true
#define ISLINUX false
#elif __linux
#define ISMAC false
#define ISLINUX true
#elif __unix // all unices not caught above
#define ISMAC false
#define ISLINUX true
#endif

#include <vector>
#include <thread>
#include "trace.h"
#include "Application.h"
#include "SingletonApplication.h"
#include "CamInstance.h"
#include "AI.h"

//------------------------------------------------------------------------------
namespace isis{
//------------------------------------------------------------------------------

class VisionManager:public Application, public SingletonApplication<VisionManager>{

public:
    VisionManager();
    ~VisionManager(); // Destructor

    void addCAM(boost::shared_ptr<cv::VideoCapture>);
    void Run();

    CamInstance* getCAMInstance(unsigned int index) const;
    AI*          getAIInstance(unsigned int index) const;

private:
    void CreateStorageDirectories();
    void initialize();

    std::vector<CamInstance*>               mVisionObjects;
    std::vector<AI*>                        mAIObjects;

    boost::thread_group                     mThreads;
};

//------------------------------------------------------------------------------
} //namespace
//------------------------------------------------------------------------------

#endif
