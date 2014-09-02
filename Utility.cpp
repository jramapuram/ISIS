/**
 * Utility functions that might be needed from different portions of the app
 */
#include "Utility.h"

namespace Utility {
	/**
	 * OS Invariant Steady time sample :)
	 */
	boost::chrono::steady_clock::time_point getSteadyTime()
	{
		return( boost::chrono::steady_clock::now());
	}

	/**
	 * Returns time since epoch as a double using boost
	 */
	double getTimeSinceEpochMS(){
		return((double) boost::chrono::duration_cast<boost::chrono::milliseconds>(boost::chrono::steady_clock::now().time_since_epoch()).count());
	}

    /**
     * @brief runProgram :runs the appropriate program
     * @param cmd
     * @return
     */
    bool runProgram(const std::string& cmd){
        bool retval=false;
        if(system(NULL)){ //if we have a command processor available
            if(system(cmd.c_str())==0){
                retval=true;
            }
        }
        return retval;
    }

	/**
	 * Helper that returns the current date
	 */
	std::string get_cur_date()
	{
		time_t now; struct tm *timeinfo;std::string retStr;
		char the_date[128];
		time(&now); timeinfo=localtime(&now);
		if(now!=-1)    strftime(the_date, 256, "%m_%d_%y_%H_%M_%S", timeinfo);
		retStr.append(the_date); //retStr.append(".avi");
		return retStr;
	}

	/**
	 * Helper to create directories
	 */
	bool create_directory(std::string path)
	{
		boost::filesystem::path dirToCreate(path);
		if(!boost::filesystem::exists(dirToCreate)){
			if (!boost::filesystem::create_directory(dirToCreate))
				return false;
		}
		return true;
	}

    void removeAll(const std::string& dir){
        boost::filesystem::path directory(dir);
        if(boost::filesystem::exists(directory)){
            boost::filesystem::remove_all(directory);
        }
    }

    /**
     * @brief convertTo32Bit: helper to convert to FP matrix
     * @param InputMat
     * @return
     */
    void convertTo32Bit(cv::Mat* InputOutputMat)
    {
        //cv::Mat retval;//=InputMat.clone();
        InputOutputMat->convertTo(*InputOutputMat,CV_32FC1);
    }

    std::string getImageType(int number)
    {
        // find type
        int imgTypeInt = number%8;
        std::string imgTypeString;

        switch (imgTypeInt)
        {
            case 0:
                imgTypeString = "8U";
                break;
            case 1:
                imgTypeString = "8S";
                break;
            case 2:
                imgTypeString = "16U";
                break;
            case 3:
                imgTypeString = "16S";
                break;
            case 4:
                imgTypeString = "32S";
                break;
            case 5:
                imgTypeString = "32F";
                break;
            case 6:
                imgTypeString = "64F";
                break;
            default:
                break;
        }

        // find channel
        int channel = (number/8) + 1;

        std::stringstream type;
        type<<"CV_"<<imgTypeString<<"C"<<channel;

        return type.str();
    }

	/**
	 * Resizes image to newImgSize
	 * convertToSingleRow: converts image into a single row
	 * convertToCV32FC1: converts format to CV32FC_1 (sometimes needed for compatibility)
	 */
    cv::Mat imgResize(const cv::Mat& img, CvSize newImgSize, bool convertToSingleRow, bool convertToCV32FC1){
		cv::Mat retval;
        if(img.rows != newImgSize.height && img.cols!=newImgSize.width){
            resize(img,retval,newImgSize,0.0,0.0,CV_INTER_AREA);
        }else{
            retval=img.clone();
        }
        if(convertToSingleRow) retval=retval.reshape(retval.channels(),1);
        if(convertToCV32FC1) retval.convertTo(retval,CV_32FC1);
		return retval;
	}
    /**
     * @brief sigmoid:  f(x)=\beta*(1-e^{-\alpha x})/(1+e^{-\alpha x} )
     * @param InputMat
     * @return
     */
    cv::Mat sigmoid(const cv::Mat& InputMat,float beta,float alpha){
        cv::Mat retval;
        cv::exp((-1*alpha*InputMat),retval);
        return(beta*(1-retval)/(1+retval));
    }

	/**
	 * Return a segmented image of squares
	 */
    std::vector<cv::Mat> imgSegment(const cv::Mat& InputImg,int SplitFactor){
		std::vector<cv::Mat> retval;
        int MaxRows=((InputImg.rows));
        int MaxCols=((InputImg.cols));
		//std::cout<<"[MaxRows,MaxCols]: ["<<MaxRows<<","<<MaxCols<<"]"<<std::endl;
		for(int r=0;r<MaxCols;r+=SplitFactor){
			for(int c=0;c<MaxRows;c+=SplitFactor){
				cv::Rect currentRect(cv::Point(r,c),cv::Point(r+SplitFactor,c+SplitFactor));
                //std::cout<<"[r,c] = ["<<r<<","<<c<<"]"<<std::endl;
                //std::cout<<"currentRect:["<<currentRect.x<<","<<currentRect.y<<"]["<<currentRect.area()<<"]"<<std::endl;
                //cv::Mat tmpRegion((*InputImg)(currentRect));
                if(currentRect.width<=0
                        || currentRect.height<=0
                        || currentRect.x+currentRect.width<=InputImg.cols
                        || currentRect.y+currentRect.height<=InputImg.rows)
                {
                    retval.clear();
                    return retval;
                }
                cv::Mat tmpRegion((InputImg)(currentRect));
				retval.push_back(tmpRegion);
			}
		}
		return retval;
	}

    /**
     * @brief imgRestore: restores an image from it's segments
     * @param ImgSet: vector containing all image chunks
     * @param size
     * @return
     */
    cv::Mat imgRestore(const std::vector<cv::Mat>& ImgSet,CvSize size){
        cv::Mat retval,CurCol;
        for(cv::Mat m : ImgSet){
            if(CurCol.cols<size.width){
                std::cout<<"adding to col\n";
                cv::hconcat(CurCol,m,CurCol);
            }else{
                if(!CurCol.empty()){
                    std::cout<<"adding to row\n";
                    retval.push_back(CurCol);
                    CurCol.release();
                }
            }
        }
        return retval;
    }

	/**
	 * Helper to do PCA on x & y on an image.
	 */
	cv::Mat doMultiPCA(cv::Mat *img,CvSize newImgSize) {
		cv::PCA pcaR(*img, cv::Mat(), CV_PCA_DATA_AS_ROW, newImgSize.width);
		cv::Mat RowProj = pcaR.project(*img);
		cv::PCA pcaC(RowProj, cv::Mat(), CV_PCA_DATA_AS_COL, newImgSize.height);
		cv::Mat ColProj = pcaC.project(RowProj);
		ColProj = ColProj.reshape(ColProj.channels(), 1);
		return ColProj;
	}

}
