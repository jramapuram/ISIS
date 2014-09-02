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
    : Application  ("AI_"+std::to_string(index))
    , index_       (index)
    , imgQueue     (Q)
    , isLearning_  (false)
{
    Caffe::set_mode(Caffe::GPU);
    google::SetCommandLineOption("minloglevel", "2");//error for caffe
    create_directory(IMG_DIR + "/" + PREDICT_DIR);
    set_events();
}

AI::~AI() {

}

void AI::train(const std::string& proto_filename){
    isLearning_ = true;

    //start our training
    SolverParameter solver_param;
    ReadProtoFromTextFile(proto_filename, &solver_param);
    SGDSolver<float> solver(solver_param);
    solver.Solve();

    //cleanup
    log()->info("Optimization complete...");
    raiseEvent("StackedDAETrainingComplete", true);
    isLearning_ = false;
}

const std::string AI::indexToStr(const std::string& file_list, uint64_t line_number){
    std::ifstream infile(file_list);
    std::string line;
    uint64_t cur_line = 0;
    while (std::getline(infile, line)){
        if(cur_line++ == line_number) break;
    }
    infile.close();
    return line;
}

void AI::predict(const std::string& network_proto){
    auto net = load_network(network_proto
                            , MODEL_DIR + "/" + IMAGENET_MODEL); //XX move loading to imgNet ptr
    float loss; // Run Forward pass
    auto& result =  net->ForwardPrefilled(&loss);

    // Now result will contain the argmax.
    const float* argmaxs = result[1]->cpu_data();
    for (int i = 0; i < result[1]->num(); ++i) {
        log()->info("[%s] predicts image %d as  [%s]"
                    , net->name()
                    , i
                    , indexToStr(MODEL_DIR + "/" + SYNSET_FILE
                                 , static_cast<uint64_t>(argmaxs[i])));
    }

    //clean up all the prediction files from this trial
    removeAll(IMG_DIR + "/" + PREDICT_DIR);
}

Net<float>* AI::load_network(const std::string& proto_model, const std::string& pretrained_model){
    if(networks_.find(proto_model) == networks_.end()){
        log()->debug("Created new network from [%s | %s]", proto_model, pretrained_model);
        networks_[proto_model] = new Net<float>(proto_model);
        networks_[proto_model]->CopyTrainedLayersFrom(pretrained_model); //get trained net
    }
    return networks_[proto_model];
}

void AI::write_to_file(const cv::Mat &img
                       , const std::string& imagename
                       , const std::string& directory
                       , const std::string& filename
                       , uint64_t classif){
    std::ofstream of(directory + "/" + filename, std::ios_base::app | std::ios_base::ate | std::ios_base::out);
    std::string latestfile = directory + "/" + imagename + ".JPEG";
    cv::imwrite(latestfile, img);
    of<<latestfile<<" "<<classif<<std::endl;
    of.close();
}

void AI::normalize(cv::Mat& img){
    cv::subtract(img, cv::mean(img), img);
    //TODO: Perhaps std dev normalize?
}

void AI::pull_img_queue() {
  imgQueue.TryDequeue(latestImg_);
  normalize(latestImg_);

  if(!isLearning_){
      if(predictDataSetSize_++ < BUFFER_SIZE){
          create_directory(IMG_DIR + "/" + PREDICT_DIR);
          write_to_file(latestImg_
                        , std::to_string(predictDataSetSize_)
                        , IMG_DIR + "/" + PREDICT_DIR
                        , PREDICT_LIST_FILE
                        , 0);
      }else{
         predict(MODEL_DIR + "/" + PROTO_FILE);
         predictDataSetSize_ = 0; //reset
      }
  }else{
      if(currentDataSetSize_++ < DATA_SET_SIZE){
        (currentDataSetSize_ > 0.8f*DATA_SET_SIZE)? write_to_file(latestImg_, std::to_string((uint64_t)getTimeSinceEpochMS()), IMG_DIR, TESTING_LIST_FILE, 0)
                                                  : write_to_file(latestImg_, std::to_string((uint64_t)getTimeSinceEpochMS()), IMG_DIR, TRAINING_LIST_FILE, 0);
      }else{
        train(PROTO_FILE_SOLV);
      }
  }
}

void AI::set_events(){
    addSignal("StackedDAETrainingComplete");
}

void AI::Run() {
    while (Isis::isRunning) {
      try {
        pull_img_queue();
      } catch (std::exception& e) {
        log()->error("Exception in Run: %s",e.what());
      }
    }
}
