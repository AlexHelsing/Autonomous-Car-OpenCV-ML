#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "HsvColorSeparator.hpp"
#include "NoiseRemover.hpp"
#include "ContourFinder.hpp"
#include "DirectionCalculator.hpp"
#include "CommonDefs.hpp"

DirectionCalculator::DirectionCalculator() {}

int DirectionCalculator::CalculateDirection(cv::Mat &inputImage, int &direction, bool VERBOSE)
{

    cv::Mat hsvConvertedImg;

    cv::cvtColor(inputImage, hsvConvertedImg, CV_BGR2HSV);

    int width = inputImage.cols / 2;
    int adjustedHeight = static_cast<int>(inputImage.rows * 0.8);

    // Define the rectangles for the left and right halves
    cv::Rect leftHalf(0, 0, width, adjustedHeight);
    cv::Rect rightHalf(inputImage.cols / 2, 0, width, adjustedHeight);

    // Process left half
    cv::Mat leftImage = hsvConvertedImg(leftHalf);
    cv::Mat leftYellowMask = colorSeparator.detectYellowColor(leftImage, VERBOSE);
    leftYellowMask = noiseRemover.RemoveNoise(leftYellowMask);
    int leftYellow = contourFinder.isEmptyOfSignificantContours(leftYellowMask);

    // Process right half
    cv::Mat rightImage = hsvConvertedImg(rightHalf);
    cv::Mat rightYellowMask = colorSeparator.detectYellowColor(rightImage, VERBOSE);
    rightYellowMask = noiseRemover.RemoveNoise(rightYellowMask);
    int rightYellow = contourFinder.isEmptyOfSignificantContours(rightYellowMask);

    if (leftYellow == 1 && rightYellow == -1)
    {
        direction = 1;
        return direction;
    }
    else if (rightYellow == 1 && leftYellow == -1)
    {
        direction = -1;
        return direction;
    }
    else
    {
        return direction; // No direction or both sides have yellow
    }
}