#include "CamInstance.h"
#include "Config.h"
#include "VisionManager.h"
#include <opencv2/ocl/ocl.hpp>

using namespace cv;
using namespace Utility;
using namespace isis;
using namespace cv::ocl;

CamInstance::CamInstance(unsigned int index, boost::shared_ptr<cv::VideoCapture> camRef)
    : Application           ("CamInstance_"+std::to_string(index))
    , index_                (index)
    , imgQueue              (DEFAULT_QUEUE_SIZE)
    , isStackedDAETrained   (false)
    , input                 (camRef)
    , isAlreadyRecording    (false) //we are not initially recording
{
    double depthSensorCode = camRef->get( CV_CAP_PROP_OPENNI_GENERATOR_PRESENT );
    m_DepthSensorExists    = 0;//depthSensorCode < 0;
    if(m_DepthSensorExists) log()->info("DepthSensor detected with value: %f",depthSensorCode);

    GenericInit();
}

CamInstance::~CamInstance() { // Destructor
    Isis::isRunning = false;

    mPoll.unlock();
    imPollerThread->interrupt();
    imgQueue.StopQueue();
}

void CamInstance::AITrainCompletedSlot()
{
    isStackedDAETrained=true;
    log()->debug("Base AI training complete. Motion will now be calculated for a new object");
}

void CamInstance::GenericInit()
{
    setImageSize(Isis::config->height, Isis::config->width);
    resetImageSet();
}

void CamInstance::resetImageSet(){
    MHI_         = Mat::zeros(imgSize, CV_32FC1);
    orientation_ = Mat::zeros(imgSize, CV_32FC1);
    history_     = Mat::zeros(imgSize, CV_32FC1);
    mask_        = Mat::zeros(imgSize, CV_8UC1);
}

Rect CamInstance::getNonZeroROI(const Mat & BinaryImg)
{
    /*
     * Max X Value is the number of cols
     * Max Y Value is the number of rows
     */
    Point UpperLeft(BinaryImg.cols,BinaryImg.rows);  //init left to the greatest value
    Point LowerRight(0,0); //init right to the lowest value

    /** i goes through 0->480
     * j goes through 0->640 **/
    for(int i = 0; i < BinaryImg.rows; i++) {
        const uchar* p = BinaryImg.ptr<uchar>(i);
        for(int j=0; j < BinaryImg.cols; j++) {
            if(p[j]==255) {
                //Determine Upper leftmost point
                if(j<UpperLeft.x)UpperLeft.x=j;
                if(i<UpperLeft.y)UpperLeft.y=i;

                //Determine Lower Rightmost Point
                if(j>LowerRight.x) LowerRight.x=j;
                if(i>LowerRight.y) LowerRight.y=i;
            }
        }
    }

//    log()->debug("UpperLeft.x: %d | Upperleft.y: %d",UpperLeft.x,UpperLeft.y);
//    log()->debug("LowerRight.x: %d | LowerRight.y: %d",LowerRight.x,LowerRight.y);
    int X_Translation=LowerRight.x-UpperLeft.x;
    int Y_Translation=LowerRight.y-UpperLeft.y;
    return (Rect(UpperLeft.x,UpperLeft.y,X_Translation,Y_Translation));
}

void CamInstance::convertToGrayScale(const Mat& OutImg)
{
    if(frame1_.empty() && frame2_.empty()){
        cvtColor(OutImg, frame1_, CV_BGR2GRAY);
        cvtColor(OutImg, frame2_, CV_BGR2GRAY);
    }else{
        if(EveryOtherFlag){
            cvtColor(OutImg, frame1_, CV_BGR2GRAY);
        }else{
            cvtColor(OutImg, frame2_, CV_BGR2GRAY);
        }
    }
    EveryOtherFlag?EveryOtherFlag=false:EveryOtherFlag=true;
}

double CamInstance::DetectMotionDirection(const Mat &Img)
{
    try{
        convertToGrayScale(Img); //cycle alternating images
        absdiff(frame1_,frame2_,framediff_);
        adaptiveThreshold(framediff_,history_,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,75,15);
        double timestamp = getTimeSinceEpochMS();
        updateMotionHistory(history_,MHI_,timestamp, MHI_DURATION_MS);
        calcMotionGradient(MHI_,mask_,orientation_,MAX_DELTA,MIN_DELTA, 3);
        return(calcGlobalOrientation(orientation_,mask_,MHI_,timestamp,MHI_DURATION_MS));
    }catch(const cv::Exception &ex){
        log()->fatal("ISIS: OpenCV Error During MotionMag Calc: %s",ex.what());
        Isis::isRunning=false;
    }
    return 0;
}

void CamInstance::ExitConditionCheck(){
    if ( (cvWaitKey(33) & 255) == 27 ) { //time delay is necessary for namedWindow
            Isis::isRunning=false;
    }
}

void CamInstance::setImageSize(double NewHeight,double NewWidth){
    try{
        log()->debug("m_DepthSensorExists: %d",m_DepthSensorExists);
        if(!m_DepthSensorExists){
            log()->debug("Height: %d, Width %d",NewHeight,NewWidth);
            input->set(CV_CAP_PROP_FRAME_WIDTH,NewWidth);
            input->set(CV_CAP_PROP_FRAME_HEIGHT,NewHeight);
            imgSize.height=NewHeight;
            imgSize.width=NewWidth;
        }else {
            imgSize.height=Isis::config->height;
            imgSize.width=Isis::config->width;
        }
    }catch(const cv::Exception &ex){
        log()->fatal("Exception during setImageSize: %s",ex.what());
    }
}

const cv::Mat& CamInstance::getImage(){
    {//scoped lock
        boost::mutex::scoped_lock lock(mPoll);
        return imCur_;
    }
}

const cv::Mat& CamInstance::getForegroundImage(){
    {//scoped lock
        //boost::mutex::scoped_lock lock(mPoll);
        return foreground_;
    }
}

const cv::Mat& CamInstance::getSegmentedImage(){
    {//scoped lock
        //boost::mutex::scoped_lock lock(mPoll);
        return segmentedImage_;
    }
}

const cv::Mat& CamInstance::getDepthImage(){
    {//scoped lock
        boost::mutex::scoped_lock lock(mPoll);
        return imDepth_;
    }
}

int CamInstance::getCurrentFrameIndex(){
    try{
        return (int)input->get(CV_CAP_PROP_POS_FRAMES);
    }catch(const cv::Exception &ex){
        log()->error("Exception during getCurrentFrameIndex: %s",ex.what());
    }
    return 0;
}

int CamInstance::getTotalFrames(){
    try{
        return (int)input->get(CV_CAP_PROP_FRAME_COUNT);
    }catch(const cv::Exception &ex){
        log()->error("Exception during getTotalFrames: %s",ex.what());
    }
    return 0;
}

void CamInstance::moveToFrame(int frame){
    log()->debug("moveToFrame>>>");
    try{
        input->set(CV_CAP_PROP_POS_FRAMES, frame);
    }catch(const cv::Exception &ex){
        log()->error("Exception during moveToFrame: %s",ex.what());
    }
}

bool CamInstance::isOpened(){
    return input->isOpened();
}

void CamInstance::grabImage(){
    while(Isis::isRunning){
//        boost::mutex::scoped_lock lk(mPoll);
        if(m_DepthSensorExists){
            input->grab();
            input->retrieve(imCur_,CV_CAP_OPENNI_BGR_IMAGE);
            input->retrieve(imDepth_,CV_CAP_OPENNI_DEPTH_MAP);
        }else{
            //log()->debug("prepoll");
//            boost::mutex::scoped_lock lk(mPoll);
            input->operator >>(imCur_);
        }
        ExitConditionCheck();
    }
}

//CamInstance::Matched_KeyPoints
//CamInstance::calculateFeatures(const oclMat& img1, const oclMat& img2){
//    SURF_OCL surf;

//    // detecting keypoints & computing descriptors
//    oclMat keypoints1GPU, keypoints2GPU;
//    oclMat descriptors1GPU, descriptors2GPU;

//    // downloading resultsa
//     vector<KeyPoints> keypoints1, keypoints2;
//     vector<DMatch> matches;

//     surf(img1, oclMat(), keypoints1GPU, descriptors1GPU);
//     surf(img2, oclMat(), keypoints2GPU, descriptors2GPU);

//     surf.downloadKeypoints(keypoints1GPU, keypoints1);
//     surf.downloadKeypoints(keypoints2GPU, keypoints2);

//     BruteForceMatcher_OCL_base matcher(BruteForceMatcher_OCL_base::L2Dist);
//     matcher.match(descriptors1GPU, descriptors2GPU, matches);

//     double max_dist = 0; double min_dist = 100;
//     //-- Quick calculation of max and min distances between keypoints
//     for( size_t i = 0; i < keypoints1.size(); i++ ){
//         double dist = matches[i].distance;
//         if( dist < min_dist ) min_dist = dist;
//         if( dist > max_dist ) max_dist = dist;
//     }

//     //-- Draw only "good" matches (i.e. whose distance is less than 2.5*min_dist )
//     std::vector< DMatch > good_matches;
//     for( size_t i = 0; i < keypoints1.size(); i++ ){
//         if( matches[i].distance < 3*min_dist ){
//             good_matches.push_back( matches[i]);
//         }
//     }
//     return std::make_tuple(good_matches, keypoints1, keypoints2);
//}

//Mat
//CamInstance::getHomography(const Matched_KeyPoints& keypoint_descriptor){
//    //-- Localize the object
//    std::vector<Point2f> obj;
//    std::vector<Point2f> scene;

//    for( size_t i = 0; i < std::get<0>(keypoint_descriptor).size(); i++ ){
//        obj.push_back( std::get<1>(keypoint_descriptor)[ std::get<0>(keypoint_descriptor)[i].queryIdx ].pt );
//        scene.push_back( std::get<2>(keypoint_descriptor)[ std::get<0>(keypoint_descriptor)[i].trainIdx ].pt );
//    }
//    return(findHomography( obj, scene, CV_RANSAC ));

////    //-- Get the corners from the image_1 ( the object to be "detected" )
////        std::vector<Point2f> obj_corners(4);
////        obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( cpu_img1.cols, 0 );
////        obj_corners[2] = cvPoint( cpu_img1.cols, cpu_img1.rows ); obj_corners[3] = cvPoint( 0, cpu_img1.rows );
////        std::vector<Point2f> scene_corners(4);

////        perspectiveTransform( obj_corners, scene_corners, H);

////        //-- Draw lines between the corners (the mapped object in the scene - image_2 )
////        line( img_matches, scene_corners[0] + Point2f( (float)cpu_img1.cols, 0), scene_corners[1] + Point2f( (float)cpu_img1.cols, 0), Scalar( 0, 255, 0), 4 );
////        line( img_matches, scene_corners[1] + Point2f( (float)cpu_img1.cols, 0), scene_corners[2] + Point2f( (float)cpu_img1.cols, 0), Scalar( 0, 255, 0), 4 );
////        line( img_matches, scene_corners[2] + Point2f( (float)cpu_img1.cols, 0), scene_corners[3] + Point2f( (float)cpu_img1.cols, 0), Scalar( 0, 255, 0), 4 );
////        line( img_matches, scene_corners[3] + Point2f( (float)cpu_img1.cols, 0), scene_corners[0] + Point2f( (float)cpu_img1.cols, 0), Scalar( 0, 255, 0), 4 );

//}

void CamInstance::AppendRectangle(Mat* ImgSrc,RotatedRect RectToAppend)
{
    try{
        if(!isRectNull(RectToAppend)){
            rectangle((*ImgSrc),RectToAppend.boundingRect(),255,15,8,0);
        }
    }catch(const cv::Exception &ex){
        //do nothing
    }
}

void CamInstance::AppendRectangle(Mat* ImgSrc,Rect RectToAppend)
{
    try{
        if(!isRectNull(RectToAppend)){
            rectangle((*ImgSrc),RectToAppend,255,15,8,0);
        }
    }catch(const cv::Exception &ex){
        //do nothing
    }
}

Mat CamInstance::GetMotionImage(const Mat& img, const Mat& foregroundImg, Rect ROI)
{
    Mat retval;
    try{
        Mat tmpROIImg=(img)(ROI); //dont double use
        bitwise_and(tmpROIImg, tmpROIImg, retval, (foregroundImg)(ROI));
    }catch(const cv::Exception &ex){
       log()->error("Error in Using bitwise_and for segmentation: %s",ex.what());
    }
    dilate(retval,retval,Mat());
    return retval;
}

//Helper to return true is there is motion. Shouldwebeginfilewrite adds the extra is already recording parameter.
bool CamInstance::IsThereMotion(int MotionMagMOG, double gradAngle){
    return(gradAngle>10 && gradAngle<200 && MotionMagMOG>=MIN_COUNT_MOTION);
}

bool CamInstance::isMatrixNull(const Mat &imgToCheck){
    return (imgToCheck.empty() || imgToCheck.cols==0 || imgToCheck.rows==0);
}

bool CamInstance::isRectNull(Rect rectToCheck){
    return( (rectToCheck.height==0 || rectToCheck.width==0 || rectToCheck.area()<1000 ) );
}

bool CamInstance::isRectNull(RotatedRect rectToCheck){
    Rect rectToCheck_Reg = rectToCheck.boundingRect();
    return( (rectToCheck_Reg.height==0 || rectToCheck_Reg.width==0 ));
}

int CamInstance::MixtureOfGaussians(const Mat& img)
{
    mog(img,foreground_,0.1);
    adaptiveThreshold(foreground_,foreground_,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY_INV,75,12);
    return countNonZero(foreground_);
}

Mat CamInstance::segmentImage(const cv::Mat& imgToSegment)
{
    Mat retval;
    if(!isMatrixNull(foreground_)){
        Rect camshiftResult = getNonZeroROI(foreground_);
        if(!isRectNull(camshiftResult)) {
            //uncomment to get the xor image
            //retval = GetMotionImage(imgToSegment, foreground_, camshiftResult);
            retval = (imCur_)(camshiftResult);
        }
    }
    return retval;
}

void CamInstance::MovementDetection()
{
    Mat latestimg = getImage();
    if(!latestimg.empty()){
        MOGMag_  = MixtureOfGaussians(latestimg); //Mixed Model
        GradAngle_ = DetectMotionDirection(latestimg); // Gradient Magnitude
        if(IsThereMotion(MOGMag_, GradAngle_)){
            log()->debug("Motion detected [Gradmad: %e | MOGMAG: %e]",GradAngle_, MOGMag_);
            segmentedImage_ = segmentImage(latestimg);
            if(!segmentedImage_.empty()){
                //cvtColor(segmentedImage_, segmentedImage_, CV_BGR2GRAY);
                imwrite("test.png",segmentedImage_);
                imgQueue.Enqueue(segmentedImage_);
            }
        }
    }
}

void CamInstance::RegisterEvents(){
    registerEvent(*VisionManager::instance()->getAIInstance(index_),"StackedDAETrainingComplete",[&] () {
        log()->info("Base AI training complete. Motion will now be calculated for a new object");
        isStackedDAETrained = true;
    });// Note: this is how you pass to a method boost::bind(&CamInstance::AITrainCompletedSlot,this));
}

void CamInstance::Run() {
    RegisterEvents();
    imPollerThread.reset(new boost::thread(&CamInstance::grabImage,this));

    boost::posix_time::millisec workTime(120);
    while(Isis::isRunning){
        boost::this_thread::sleep(workTime);
        MovementDetection();
    }
}
