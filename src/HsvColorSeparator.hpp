#ifndef HSV_COLOR_SEPARATOR_HPP
#define HSV_COLOR_SEPARATOR_HPP

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class HsvColorSeparator
{
public:
    HsvColorSeparator();
    cv::Mat detectBlueColor(const cv::Mat &inputFrame, bool VERBOSE);
    cv::Mat detectYellowColor(const cv::Mat &inputFrame, bool VERBOSE);
};

#endif