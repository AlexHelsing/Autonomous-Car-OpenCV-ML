#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "NoiseRemover.hpp"

NoiseRemover::NoiseRemover(){};

cv::Mat NoiseRemover::RemoveNoise(const cv::Mat &inputFrame) {
    if(inputFrame.empty()){
        return cv::Mat();  // Handle empty input
    }

    cv::Mat outputFrame;
    cv::GaussianBlur(inputFrame, outputFrame, cv::Size(5, 5), 0);  // Increased blur

    cv::Mat eroded;
    cv::erode(outputFrame, eroded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));

    cv::Mat dilated;
    cv::dilate(eroded, dilated, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));
    
    return dilated;  // Return the processed frame
}
