#ifndef AI_H_
#define AI_H_

#include "SyncQueue.h"
#include "trace.h"
#include "SingletonApplication.h"
#include "Application.h"
#include <opencv2/opencv.hpp>

namespace isis{

class AI:public Application {

private:
    const int DATA_SET_SIZE=1000;
    const std::string PROTO_FILE="isis_solver.proto";
    const std::string IMG_DIR = "img";
    const std::string CURRENT_FILE_LIST = "current_images.txt";

    SyncQueue<cv::Mat>& imgQueue;
    unsigned int        index_;
    uint64_t            currentDataSetSize_;
    cv::Mat             latestImg_;
    bool                isTrainingComplete_;

    void PullImgQueue();
    void SetEvents();
    void normalize(cv::Mat& img);
    void TrainNet(const std::string& filename);
    void WriteToFile(const cv::Mat& img, uint64_t classif);

public:
    AI(unsigned int , SyncQueue<cv::Mat>&);
    ~AI(); 

    void Run();
    void Predict(const cv::Mat& img);
};

}//namespace
#endif
