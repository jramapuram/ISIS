#ifndef AI_H_
#define AI_H_

#include "SyncQueue.h"
#include "trace.h"
#include "SingletonApplication.h"
#include "Application.h"
#include "caffe/caffe.hpp"
#include <opencv2/opencv.hpp>

namespace isis{

class AI:public Application {

private:
    typedef caffe::Net<float>*                      pModel;
    typedef std::unordered_map<std::string, pModel> NetworkMap;

    const uint64_t    DATA_SET_SIZE      = 1000;
    const uint64_t    BUFFER_SIZE        = 10;
    const std::string MODEL_DIR          = "models";
    const std::string IMG_DIR            = "img";
    const std::string PROTO_FILE_SOLV    = "isis_solver.proto";
    const std::string PROTO_FILE         = "isis.proto";
    const std::string IMAGENET_MODEL     = "caffe_reference_imagenet_model";
    const std::string TRAINED_NET        = "isis_train";
    const std::string TRAINING_LIST_FILE = "train_images.txt";
    const std::string TESTING_LIST_FILE  = "test_images.txt";
    const std::string PREDICT_LIST_FILE  = "predict_images.txt";

    SyncQueue<cv::Mat>& imgQueue;
    unsigned int        index_;
    uint64_t            currentDataSetSize_;
    uint64_t            predictDataSetSize_;
    cv::Mat             latestImg_;
    bool                isLearning_;
    NetworkMap          networks_;


    void pull_img_queue();
    void set_events();
    void normalize(cv::Mat& img);
    void train(const std::string& proto_filename);
    void write_to_file(const cv::Mat& img, const std::string &filename, uint64_t classif);
    caffe::Net<float>* load_network(const std::string& proto_model, const std::string& model_path);

public:
    AI(unsigned int , SyncQueue<cv::Mat>&);
    ~AI(); 

    void Run();
    void predict(const std::string& network_proto, const std::string& file_list);
};

}//namespace
#endif
