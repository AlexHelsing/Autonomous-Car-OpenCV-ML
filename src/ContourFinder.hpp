#ifndef CONTOUR_FINDER_HPP
#define CONTOUR_FINDER_HPP

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>



class ContourFinder{
    public:
    ContourFinder();
    cv::Mat FindContours(const cv::Mat &imageInput, const cv::Mat &originalImage, int &minContourArea, int &maxContourArea);
    int isEmptyOfSignificantContours(const cv::Mat &imageInput);
};

#endif
