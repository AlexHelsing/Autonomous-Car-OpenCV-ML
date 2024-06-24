#ifndef DIRECTION_CALCULATOR_HPP
#define DIRECTION_CALCULATOR_HPP

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

class DirectionCalculator
{
public:
    DirectionCalculator();
    int CalculateDirection(cv::Mat &inputImage, int &direction, bool VERBOSE);
};

#endif // DIRECTION_CALCULATOR_HPP