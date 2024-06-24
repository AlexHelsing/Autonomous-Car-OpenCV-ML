#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "HsvColorSeparator.hpp"

int iLowH = 90;
int iHighH = 135;

int iLowS = 44;
int iHighS = 255;

int iLowV = 45;
int iHighV = 255;

int yellowLowH = 15;
int yellowHighH = 30;

int yellowLowS = 42;
int yellowHighS = 255;

int yellowLowV = 46;
int yellowHighV = 255;

HsvColorSeparator::HsvColorSeparator(){};

cv::Mat HsvColorSeparator::detectBlueColor(const cv::Mat &inputFrame, bool VERBOSE)
{
    cv::Mat mask;
    cv::inRange(inputFrame, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), mask);

    // Add trackbars for manually adjusting HSV during runtime. Makes it easier to experiment with filters and finding
    // the correct HSV values.
    if (VERBOSE)
    {
        cv::namedWindow("BlueTrackingControl", cv::WINDOW_AUTOSIZE);
        cv::createTrackbar("LowH", "BlueTrackingControl", &iLowH, 179); // Hue (0 - 179)
        cv::createTrackbar("HighH", "BlueTrackingControl", &iHighH, 179);
        cv::createTrackbar("LowS", "BlueTrackingControl", &iLowS, 255); // Saturation (0 - 255)
        cv::createTrackbar("HighS", "BlueTrackingControl", &iHighS, 255);
        cv::createTrackbar("LowV", "BlueTrackingControl", &iLowV, 255); // Value (0 - 255)
        cv::createTrackbar("HighV", "BlueTrackingControl", &iHighV, 255);
    }
    return mask;
}

cv::Mat HsvColorSeparator::detectYellowColor(const cv::Mat &inputFrame, bool VERBOSE)
{
    cv::Mat mask;
    cv::inRange(inputFrame, cv::Scalar(yellowLowH, yellowLowS, yellowLowV), cv::Scalar(yellowHighH, yellowHighS, yellowHighV), mask);

    // Add trackbars for manually adjusting HSV during runtime. Makes it easier to experiment with filters and finding
    // the correct HSV values.
    if (VERBOSE)
    {

        cv::namedWindow("YellowTrackingControl", cv::WINDOW_AUTOSIZE);
        cv::createTrackbar("LowH", "YellowTrackingControl", &yellowLowH, 179); // Hue (0 - 179)
        cv::createTrackbar("HighH", "YellowTrackingControl", &yellowHighH, 179);

        cv::createTrackbar("LowS", "YellowTrackingControl", &yellowLowS, 255); // Saturation (0 - 255)
        cv::createTrackbar("HighS", "YellowTrackingControl", &yellowHighS, 255);

        cv::createTrackbar("LowV", "YellowTrackingControl", &yellowLowV, 255); // Value (0 - 255)
        cv::createTrackbar("HighV", "YellowTrackingControl", &yellowHighV, 255);
    }

    return mask;
}
