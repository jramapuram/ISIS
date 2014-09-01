#include "AI.h"
#include "Utility.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <caffe/caffe.hpp>
#include <cuda_runtime.h>

using namespace Utility;
using namespace isis;
using namespace caffe;

AI::AI(unsigned int index, SyncQueue<cv::Mat>& Q)
    : Application         ("AI_"+std::to_string(index))
    , index_              (index)
    , imgQueue            (Q)
    , isTrainingComplete_ (false)
{
    CreateDirectory(IMG_DIR);
    SetEvents();
}

AI::~AI() {

}

void AI::TrainNet(const std::string& filename){
//    Caffe::set_mode(Caffe::GPU);
////    Caffe::set_phase(Caffe::TRAIN);
    SolverParameter solver_param;
    ReadProtoFromTextFile(filename, &solver_param);
    SGDSolver<float> solver(solver_param);
    solver.Solve();

    log()->info("Optimization complete...");
    raiseEvent("StackedDAETrainingComplete", true);
    isTrainingComplete_ = true;
}

void AI::Predict(const cv::Mat &img, const std::string& filename){
    if(predictDataSetSize_++ < BUFFER_SIZE){
        WriteToFile(img, PREDICT_LIST_FILE, 0);
    }else{
        Net<float> caffe_test_net(filename); //get the net
        caffe_test_net.CopyTrainedLayersFrom(TRAINED_NET); //get trained net

        float loss; // Run Forward pass
        auto& result =  caffe_test_net.ForwardPrefilled(&loss);

        // Now result will contain the argmax results.
        const float* argmaxs = result[0]->cpu_data();
        for (int i = 0; i < result[0]->num(); ++i) {
          log()->info("Image %d is of class %f", i, argmaxs[i]);
        }

        //clean up all the prediction files
        std::ifstream infile(filename);
        std::string line;
        while (std::getline(infile, line)){
            log()->debug("Trying to remove: %s", line);
            removeAll(line);
        }
    }
}

void AI::WriteToFile(const cv::Mat &img, const std::string& filename, uint64_t classif){
    std::ofstream of(IMG_DIR + "/" + filename, std::ios_base::app | std::ios_base::ate | std::ios_base::out);
    std::string latestfile = IMG_DIR + "/" + std::to_string((uint64_t)getTimeSinceEpochMS()) + ".JPEG";
    cv::imwrite(latestfile, img);
    of<<latestfile<<" "<<classif<<std::endl;
    of.close();
}

void AI::normalize(cv::Mat& img){
    cv::subtract(img, cv::mean(img), img);
    //TODO: Perhaps std dev normalize?
}

void AI::PullImgQueue() {
  imgQueue.TryDequeue(latestImg_);
  normalize(latestImg_);

  if(!isTrainingComplete_ && currentDataSetSize_++ < DATA_SET_SIZE){
    (currentDataSetSize_ > 0.8f*DATA_SET_SIZE)? WriteToFile(latestImg_, TESTING_LIST_FILE, 0)
                                              : WriteToFile(latestImg_, TRAINING_LIST_FILE, 0);
  }else if(!isTrainingComplete_){
    TrainNet(PROTO_FILE);
  }else{
    Predict(latestImg_, PROTO_FILE);
  }
}

void AI::SetEvents(){
    addSignal("StackedDAETrainingComplete");
}

void AI::Run() {
    while (Isis::isRunning) {
      try {
        PullImgQueue();
      } catch (std::exception& e) {
        log()->error("Exception in Run: %s",e.what());
      }
    }
}
