#ifndef CAMINSTANCE_H
#define CAMINSTANCE_H

//#ifdef __APPLE__
//#define ISMAC true
//#define ISLINUX false
//#elif __linux
//#define ISMAC false
//#define ISLINUX true
//#elif __unix // all unices not caught above
//#define ISMAC false
//#define ISLINUX true
//#endif

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <math.h>
#include <iostream>
#include <memory>
#include <tuple>

#include <boost/chrono.hpp>
#include <boost/date_time.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
//#include <boost/thread/condition_variable.hpp>/

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/photo/photo.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/ocl/ocl.hpp>

#include <opencv2/nonfree/ocl.hpp>
//#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include "Utility.h"
#include "Application.h"
#include "SyncQueue.h"

namespace isis{

class CamInstance:public Application{
private:
    const int                               HEIGHT = 480, WIDTH = 640, CHANNELS = 3;
    const double                            MHI_DURATION_MS = 1000; //1sec
    const double                            MIN_DELTA = 50, MAX_DELTA = 500;
    const int                               MIN_COUNT_MOTION = 10000;
    const unsigned int                      DEFAULT_QUEUE_SIZE = 100;

    bool EveryOtherFlag, motionFlags,isAlreadyRecording, isStackedDAETrained, m_DepthSensorExists;

    SyncQueue<cv::Mat>  imgQueue;

    //CV Specific variables
    cv::Mat imCur_, frame1_, frame2_, framediff_, history_, MHI_, orientation_, mask_, imDepth_, foreground_, segmentedImage_;
    boost::shared_ptr<cv::VideoCapture> input;

    cv::BackgroundSubtractorMOG2 mog;

    CvSize imgSize;
    cv::VideoWriter outputMovie;

    //Methods
    void GenericInit();
    void resetImages();
    void grabImage();
    double DetectMotionDirection(const cv::Mat &);
    void convertToGrayScale(const cv::Mat &);
    void ExitConditionCheck();
    cv::Rect getNonZeroROI(const cv::Mat &);

    void MovementDetection();
    bool IsThereMotion(int,double);

    void AppendRectangle(cv::Mat*, cv::RotatedRect);
    void AppendRectangle(cv::Mat*, cv::Rect);

    cv::Mat GetMotionImage(const cv::Mat & , const cv::Mat & , cv::Rect );

    bool isRectNull(cv::Rect);
    bool isRectNull(cv::RotatedRect);
    bool isMatrixNull(const cv::Mat &);

    int MixtureOfGaussians(const cv::Mat&);
    cv::Mat segmentImage(const cv::Mat &imgToSegment);

    void RegisterEvents();
    void AITrainCompletedSlot();

    std::unique_ptr<boost::thread> imPollerThread;
    boost::mutex mPoll;

    unsigned int index_;

    int MOGMag_;
    double GradAngle_;

public:
    typedef std::tuple<std::vector<cv::DMatch>, std::vector<cv::KeyPoint>, std::vector<cv::KeyPoint> > Matched_KeyPoints;

    CamInstance(unsigned int, boost::shared_ptr<cv::VideoCapture>);
    ~CamInstance(); // Destructor

    int getMOGMag() const { return MOGMag_; }
    double getGradAngle() const { return GradAngle_; }
    bool isThereADepthSensor(){return m_DepthSensorExists;}
    SyncQueue<cv::Mat>& getQueue(){ return imgQueue; }

    Matched_KeyPoints calculateFeatures(const cv::ocl::oclMat& img1, const cv::ocl::oclMat& img2);
    cv::Mat getHomography(const Matched_KeyPoints& keypoint_descriptor);

    const cv::Mat &getImage();
    const cv::Mat &getDepthImage();
    const cv::Mat &getForegroundImage();
    const cv::Mat &getSegmentedImage();

    bool isOpened();
    void setImageSize(double , double );
    int getCurrentFrameIndex();
    void moveToFrame(int );
    int getTotalFrames();
    void Run();
    void resetImageSet();


};

}
#endif // CAMINSTANCE_H
