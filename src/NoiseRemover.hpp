#ifndef NOISE_REMOVER_HPP
#define NOISE_REMOVER_HPP

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class NoiseRemover{
    public:
        NoiseRemover();
        cv::Mat RemoveNoise(const cv::Mat &inputFrame);
    };

#endif