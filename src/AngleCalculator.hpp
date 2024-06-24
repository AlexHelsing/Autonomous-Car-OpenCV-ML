#ifndef ANGLE_CALCULATOR_HPP
#define ANGLE_CALCULATOR_HPP

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

class AngleCalculator
{
public:
    AngleCalculator();
    float CalculateSteeringAngle(cv::Mat &yellowInputImage, cv::Mat &blueInputImage, float &steeringWheelAngle, bool isClockwise, float maxSteering, float minSteering, bool VERBOSE);

private:
    cv::Point calculateCentroid(const std::vector<std::vector<cv::Point>> &contours, const cv::Point &imageCenter);
    float adjustSteering(float &newSteering, cv::Point &blueCentroid, cv::Point yellowCentroid, const cv::Point &imageCenter, const cv::Point &imageLeftThird, const cv::Point &imageRightThird, bool isClockwise, bool VERBOSE);
    float smoothSteering(float currentSteering, float alpha);
    static constexpr float steeringSensitivity = 0.1f; // Adjust sensitivity
    static constexpr float steeringThreshold = 0.05f;  // Minimum change required to adjust steering
};

#endif // ANGLE_CALCULATOR_HPP