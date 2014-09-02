#ifndef UTILITY_H_
#define UTILITY_H_

#include <boost/date_time.hpp>
#include <boost/chrono.hpp>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

namespace Utility{
	boost::chrono::steady_clock::time_point getSteadyTime();
	double getTimeSinceEpochMS();
    bool runProgram(const std::string&);
	std::string get_cur_date();
	bool create_directory(std::string);
    void removeAll(const std::string& dir);
    cv::Mat imgResize(const cv::Mat & , CvSize, bool, bool);
	cv::Mat doMultiPCA(cv::Mat*,CvSize );
    std::vector<cv::Mat> imgSegment(const cv::Mat&,int SplitFactor=8); //default split factor is 8
    void convertTo32Bit(cv::Mat *);
    std::string getImageType(int);
    std::vector<std::string> split(const std::string& str, const char* separator);
    cv::Mat sigmoid(const cv::Mat& ,float beta,float alpha);
    cv::Mat imgRestore(const std::vector<cv::Mat>& ,CvSize );
}

#endif /* UTILITY_H_ */
