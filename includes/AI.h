#ifndef AI_H_
#define AI_H_

#undef IS_USING_PYLEARN2

#include "SyncQueue.h"
#include "trace.h"
#include "SingletonApplication.h"
#include "Application.h"
#include <opencv2/opencv.hpp>

namespace isis{

class AI:public Application {

private:
    static const int DATA_SET_SIZE=1000;

    SyncQueue<cv::Mat>& imgQueue;
    unsigned int        index_;

    cv::Mat             latestImg_;

public:
    AI(unsigned int , SyncQueue<cv::Mat>&);
	~AI(); 

	void Run();

    void PullImgQueue();
    void SetEvents();
};

}//namespace
#endif
