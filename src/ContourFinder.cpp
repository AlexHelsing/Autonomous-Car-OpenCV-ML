#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "ContourFinder.hpp"

ContourFinder::ContourFinder(){};

cv::Mat ContourFinder::FindContours(const cv::Mat &imageInput,const cv::Mat &originalInput, int &minContourArea, int &maxContourArea){
    // Find contours and save them in contours.
                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(imageInput, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);


                // Create a black image, size of original img.
                cv::Mat contourOutput = cv::Mat::zeros(originalInput.size(), originalInput.type());
                // Add the processed contour to the black image. We can not just add the contours to the black image and then
                // add it to original image by using addWeighted. We have to manually offset the points to the correct position.
                // This is to ensure that the contours appear where they should in the original picture.
                for (const auto& contour : contours) {
                    double area = cv::contourArea(contour);
                    if (area > minContourArea && area < maxContourArea) { // Only process contours smaller than the maximum area
                        std::vector<cv::Point> shiftedContour;
                        for (const auto& point : contour) {
                            shiftedContour.push_back(cv::Point(point.x, point.y + originalInput.rows / 2)); // Offset vertically
                        }
                    cv::drawContours(contourOutput, std::vector<std::vector<cv::Point>>{shiftedContour}, -1, cv::Scalar(0, 255, 0), 2);
                    }
                }
                return contourOutput;
}

int ContourFinder::isEmptyOfSignificantContours(const cv::Mat &imageInput) {
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(imageInput, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    int minimumArea = 100;

    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area > minimumArea) {
            return 1; // Found a contour that is larger than the minimum area
        }
    }
    return -1; // No contours found that are larger than the minimum area
}



