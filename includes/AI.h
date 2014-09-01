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
    const uint64_t DATA_SET_SIZE=1000;
    const uint64_t BUFFER_SIZE=10;
    const std::string PROTO_FILE="isis_solver.proto";
    const std::string TRAINED_NET="isis_train";
    const std::string IMG_DIR = "img";
    const std::string TRAINING_LIST_FILE = "train_images.txt";
    const std::string TESTING_LIST_FILE  = "test_images.txt";
    const std::string PREDICT_LIST_FILE  = "predict_images.txt";

    SyncQueue<cv::Mat>& imgQueue;
    unsigned int        index_;
    uint64_t            currentDataSetSize_;
    uint64_t            predictDataSetSize_;
    cv::Mat             latestImg_;
    bool                isTrainingComplete_;

    void PullImgQueue();
    void SetEvents();
    void normalize(cv::Mat& img);
    void TrainNet(const std::string& filename);
    void WriteToFile(const cv::Mat& img, const std::string &filename, uint64_t classif);

public:
    AI(unsigned int , SyncQueue<cv::Mat>&);
    ~AI(); 

    void Run();
    void Predict(const cv::Mat& img, const std::string&);
};

}//namespace
#endif
