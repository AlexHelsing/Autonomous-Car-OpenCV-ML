#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "CommonDefs.hpp"
#include "AngleCalculator.hpp"
#include <iostream>

AngleCalculator::AngleCalculator() {}

float AngleCalculator::CalculateSteeringAngle(cv::Mat &yellowInputImage, cv::Mat &blueInputImage, float &steeringWheelAngle, bool isClockwise, float maxSteering, float minSteering, bool VERBOSE)
{
    std::vector<std::vector<cv::Point>> blueContours;
    std::vector<std::vector<cv::Point>> yellowContours;

    // Assuming you know the dimensions of the image and the distracting area
    int cropHeight = 100; // Height in pixels to crop from the bottom
    cv::Rect roi(0, 0, yellowInputImage.cols, yellowInputImage.rows - cropHeight);
    cv::Mat croppedBlueImage = blueInputImage(roi);
    cv::Mat croppedYellowImage = yellowInputImage(roi);

    cv::findContours(croppedBlueImage, blueContours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(croppedYellowImage, yellowContours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // Define minimum and maximum contour areas
    double minArea = 130.0;  // Minimum area to consider a contour
    double maxArea = 1000.0; // Maximum area to avoid abnormally large contours

    // Define aspect ratio thresholds (specific to the expected shape of the cones)
    float minAspectRatio = 0.5; // Minimum aspect ratio
    float maxAspectRatio = 2.0; // Maximum aspect ratio

    std::vector<std::vector<cv::Point>> filteredYellowContours;
    for (const auto &contour : yellowContours)
    {
        double area = cv::contourArea(contour);

        // Calculate bounding rectangle to get aspect ratio
        cv::Rect boundingBox = cv::boundingRect(contour);
        float aspectRatio = static_cast<float>(boundingBox.width) / boundingBox.height;

        // Check if contour matches area and aspect ratio criteria
        if (area >= minArea && area <= maxArea && aspectRatio >= minAspectRatio && aspectRatio <= maxAspectRatio)
        {
            filteredYellowContours.push_back(contour);
        }
    }

    // Filter blue contours based on area
    std::vector<std::vector<cv::Point>> filteredBlueContours;
    for (const auto &contour : blueContours)
    {
        double area = cv::contourArea(contour);
        if (area >= minArea && area <= maxArea)
        {
            filteredBlueContours.push_back(contour);
        }
    }

    // Calculate points that divide the screen into three equal vertical sections
    cv::Point imageCenter(blueInputImage.cols / 2, blueInputImage.rows / 2);
    cv::Point imageLeftThird(blueInputImage.cols / 3, blueInputImage.rows / 2);      // One third from the left
    cv::Point imageRightThird(blueInputImage.cols * 2 / 3, blueInputImage.rows / 2); // Two thirds from the left

    cv::Point blueCentroid = calculateCentroid(filteredBlueContours, imageCenter);
    cv::Point yellowCentroid = calculateCentroid(filteredYellowContours, imageCenter);

    // Create a visual output by combining the blue and yellow images
    cv::Mat visualOutput;
    cv::cvtColor(blueInputImage, visualOutput, cv::COLOR_GRAY2BGR); // Convert blue image to color for visualization
    cv::Mat yellowBGR;
    cv::cvtColor(yellowInputImage, yellowBGR, cv::COLOR_GRAY2BGR);         // Convert yellow image to color
    cv::addWeighted(visualOutput, 0.5, yellowBGR, 0.5, 0.0, visualOutput); // Blend both images

    // Draw image center
    cv::circle(visualOutput, imageCenter, 5, cv::Scalar(0, 255, 0), -1); // Green color

    // Draw centroids
    cv::circle(visualOutput, blueCentroid, 5, cv::Scalar(255, 0, 0), -1);     // Blue color for blue centroid
    cv::circle(visualOutput, yellowCentroid, 5, cv::Scalar(0, 255, 255), -1); // Yellow color for yellow centroid

    // Draw lines from image center to centroids
    cv::line(visualOutput, imageCenter, blueCentroid, cv::Scalar(255, 0, 0), 2);     // Blue line to blue centroid
    cv::line(visualOutput, imageCenter, yellowCentroid, cv::Scalar(0, 255, 255), 2); // Yellow line to yellow centroid

    // Drawing the division lines on the image for visual verification
    cv::line(visualOutput, cv::Point(imageLeftThird.x, 0), cv::Point(imageLeftThird.x, visualOutput.rows), cv::Scalar(0, 255, 0), 2);   // Green line for left third
    cv::line(visualOutput, cv::Point(imageRightThird.x, 0), cv::Point(imageRightThird.x, visualOutput.rows), cv::Scalar(0, 255, 0), 2); // Green line for right third

    // Display the visual output if VERBOSE is enabled
    if (VERBOSE)
    {
        cv::imshow("Visual Output", visualOutput);
    }

    float newSteering = steeringWheelAngle;
    newSteering = adjustSteering(newSteering, blueCentroid, yellowCentroid, imageCenter, imageLeftThird, imageRightThird, isClockwise, VERBOSE);

    // Clamp the steering value to be within allowed limits
    // newSteering = std::max(minSteering, std::min(maxSteering, newSteering));

    return newSteering;
}

cv::Point AngleCalculator::calculateCentroid(const std::vector<std::vector<cv::Point>> &contours, const cv::Point &imageCenter)
{
    cv::Moments m;
    int xSum = 0, ySum = 0, count = 0;

    for (const auto &contour : contours)
    {
        m = cv::moments(contour);
        if (m.m00 > std::numeric_limits<double>::epsilon())
        { // Avoid division by zero by checking against epsilon
            xSum += static_cast<int>(m.m10 / m.m00);
            ySum += static_cast<int>(m.m01 / m.m00);
            count++;
        }
    }
    return count > 0 ? cv::Point(xSum / count, ySum / count) : imageCenter; // Fallback to image center if no valid centroids found
}

float AngleCalculator::adjustSteering(float &newSteering, cv::Point &blueCentroid, cv::Point yellowCentroid, const cv::Point &imageCenter, const cv::Point &imageLeftThird, const cv::Point &imageRightThird, bool isClockwise, bool VERBOSE)
{
    // Define how much to adjust steering when necessary
    // float steeringAdjustmentAmount = 0.05f;  // This value might need tuning

    // For clockwise direction: Blue cones on the left, Yellow cones on the right
    if (isClockwise)
    {

        if (VERBOSE)
        {
            std::cout << "Clockwise Direction" << std::endl;
        }
        // Check if blue cones have crossed from the left third into the center
        if (blueCentroid.x < imageLeftThird.x && yellowCentroid.x > imageRightThird.x)
        {

            if (VERBOSE)
            {
                std::cout << "Driving Straight" << std::endl;
            }
            newSteering = 0.0f;
            return newSteering; // Steer right to adjust for blue cone moving towards center
        }
        if (blueCentroid.x > imageLeftThird.x && blueCentroid.x < imageRightThird.x)
        {
            if (VERBOSE)
            {
                std::cout << "Steering Right" << std::endl;
            }
            newSteering = -0.11f;
            return newSteering; // Steer right to adjust for blue cone moving towards center
        }
        if (blueCentroid.x > imageCenter.x && blueCentroid.x < imageRightThird.x)
        {
            if (VERBOSE)
            {
                std::cout << "Steering Right Sharp" << std::endl;
            }
            newSteering = -0.17f;
            return newSteering; // Steer right to adjust for blue cone moving towards center
        }
        if (blueCentroid.x > imageRightThird.x)
        {
            if (VERBOSE)
            {
                std::cout << "Steering Right Sharpest" << std::endl;
            }
            newSteering = -0.225f;
            return newSteering; // Steer right to adjust for blue cone moving towards center
        }

        // Check if yellow cones have crossed from the right third into the center
        if (yellowCentroid.x < imageRightThird.x && yellowCentroid.x > imageCenter.x)
        {
            if (VERBOSE)
            {
                std::cout << "Steering Left" << std::endl;
            }
            newSteering = 0.11f;
            return newSteering; // Steer left to adjust for yellow cone moving towards center
        }
        if (yellowCentroid.x < imageCenter.x && yellowCentroid.x > imageLeftThird.x)
        {
            if (VERBOSE)
            {
                std::cout << "Steering Left Sharp" << std::endl;
            }
            newSteering = 0.17f;
            return newSteering; // Steer left to adjust for yellow cone moving towards center
        }
        if (yellowCentroid.x > imageLeftThird.x)
        {
            if (VERBOSE)
            {
                std::cout << "Steering Left Sharpest" << std::endl;
            }
            newSteering = 0.225f;
            return newSteering; // Steer right to adjust for blue cone moving towards center
        }
    }
    // For counter-clockwise direction: Blue cones on the right, Yellow cones on the left
    else
    {
        if (VERBOSE)
        {
            std::cout << "Counter-Clockwise Direction" << std::endl;
        }
        // Blue cones on the right
        if (blueCentroid.x > imageRightThird.x && yellowCentroid.x < imageLeftThird.x)
        {
            if (VERBOSE)
            {
                std::cout << "Driving Straight" << std::endl;
            }
            newSteering = 0.0f;
            return newSteering; // Steer right to adjust for blue cone moving towards center
        }
        if (blueCentroid.x < imageRightThird.x && blueCentroid.x > imageLeftThird.x)
        {
            if (VERBOSE)
            {
                std::cout << "Steering Right" << std::endl;
            }
            newSteering = 0.11f;
            return newSteering;
        }
        if (blueCentroid.x < imageCenter.x && blueCentroid.x > imageLeftThird.x)
        {
            if (VERBOSE)
            {
                std::cout << "Steering Left Sharp" << std::endl;
            }
            newSteering = 0.17f;
            return newSteering; // Steer right to adjust for blue cone moving towards center
        }
        if (blueCentroid.x > imageLeftThird.x)
        {
            if (VERBOSE)
            {
                std::cout << "Steering Left Sharpest" << std::endl;
            }
            newSteering = 0.225f;
            return newSteering; // Steer right to adjust for blue cone moving towards center
        }
        // Yellow cones on the left
        if (yellowCentroid.x > imageLeftThird.x && yellowCentroid.x < imageCenter.x)
        {
            if (VERBOSE)
            {
                std::cout << "Steering Right" << std::endl;
            }
            newSteering = -0.11f;
            return newSteering; // Steer left to adjust for yellow cone moving towards center
        }
        if (yellowCentroid.x > imageCenter.x && yellowCentroid.x < imageRightThird.x)
        {
            if (VERBOSE)
            {
                std::cout << "Steering Right Sharp" << std::endl;
            }
            newSteering = -0.17f;
            return newSteering; // Steer right to adjust for blue cone moving towards center
        }
        if (blueCentroid.x > imageRightThird.x)
        {
            if (VERBOSE)
            {
                std::cout << "Steering Right Sharpest" << std::endl;
            }
            newSteering = -0.225f;
            return newSteering; // Steer right to adjust for blue cone moving towards center
        }
    }

    // If none of the conditions are met, no steering adjustment is needed
    if (VERBOSE)
    {
        std::cout << "No Steering Adjustment" << std::endl;
    }
    return newSteering;
}

float AngleCalculator::smoothSteering(float currentSteering, float alpha)
{
    // Alpha is a smoothing factor, e.g., 0.1
    return alpha * currentSteering + (1 - alpha) * currentSteering;
}
