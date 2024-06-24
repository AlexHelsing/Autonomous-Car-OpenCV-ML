/*
 * Copyright (C) 2020  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Include the single-file, header-only middleware libcluon to create high-performance microservices
#include "cluon-complete.hpp"
// Include the OpenDLV Standard Message Set that contains messages that are usually exchanged for automotive or robotic applications

// Include the OpenDLV Standard Message Set that contains messages that are usually exchanged for automotive or robotic applications
#include "opendlv-standard-message-set.hpp"

// Include the GUI and image processing header files from OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include "HsvColorSeparator.hpp"
#include "NoiseRemover.hpp"
#include "ContourFinder.hpp"
#include "DirectionCalculator.hpp"
#include "AngleCalculator.hpp"
#include "CommonDefs.hpp"

int32_t main(int32_t argc, char **argv)
{
    int32_t retCode{1};

    float steeringWheelAngle = 0.0f;
    float MLSteeringAngle = 0.0f;
    int direction = 0; // -1 for clockwise, 1 for counter-clockwise

    // For TESTING STUFF
    int totalEntries = 0;
    int totalWithinRange = 0;

    // Open a file for writing our steering angles
    std::ofstream outputFile("../steeringAngles.csv", std::ios::out | std::ios::app);

    // Write headers if the file is new or empty
    if (outputFile.tellp() == 0)
    {
        outputFile << "Timestamp, SteeringAngle, OriginalSteering\n";
    }
    else
    {
        // If the file is not empty, we need to clear it
        outputFile.close();
        outputFile.open("../steeringAngles.csv", std::ios::out | std::ios::trunc);
        outputFile << "Timestamp, SteeringAngle, OriginalSteering\n";
    }

    // Parse the command line parameters as we require the user to specify some mandatory information on startup.
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if ((0 == commandlineArguments.count("cid")) ||
        (0 == commandlineArguments.count("name")) ||
        (0 == commandlineArguments.count("width")) ||
        (0 == commandlineArguments.count("height")))
    {
        std::cerr << argv[0] << " attaches to a shared memory area containing an ARGB image." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --cid=<OD4 session> --name=<name of shared memory area> [--verbose]" << std::endl;
        std::cerr << "         --cid:    CID of the OD4Session to send and receive messages" << std::endl;
        std::cerr << "         --name:   name of the shared memory area to attach" << std::endl;
        std::cerr << "         --width:  width of the frame" << std::endl;
        std::cerr << "         --height: height of the frame" << std::endl;
        std::cerr << "Example: " << argv[0] << " --cid=253 --name=img --width=640 --height=480 --verbose" << std::endl;
    }
    else
    {
        // Extract the values from the command line parameters
        const std::string NAME{commandlineArguments["name"]};
        const uint32_t WIDTH{static_cast<uint32_t>(std::stoi(commandlineArguments["width"]))};
        const uint32_t HEIGHT{static_cast<uint32_t>(std::stoi(commandlineArguments["height"]))};
        const bool VERBOSE{commandlineArguments.count("verbose") != 0};

        // Attach to the shared memory.
        std::unique_ptr<cluon::SharedMemory> sharedMemory{new cluon::SharedMemory{NAME}};
        if (sharedMemory && sharedMemory->valid())
        {
            std::clog << argv[0] << ": Attached to shared memory '" << sharedMemory->name() << " (" << sharedMemory->size() << " bytes)." << std::endl;

            // Interface to a running OpenDaVINCI session where network messages are exchanged.
            // The instance od4 alblueLowS you to send and receive messages.
            cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};

            opendlv::proxy::GroundSteeringRequest gsr;
            std::mutex gsrMutex;
            auto onGroundSteeringRequest = [&gsr, &gsrMutex](cluon::data::Envelope &&env)
            {
                // The envelope data structure provide further details, such as sampleTimePoint as shown in this test case:
                // https://github.com/chrberger/libcluon/blob/master/libcluon/testsuites/TestEnvelopeConverter.cpp#L31-L40
                std::lock_guard<std::mutex> lck(gsrMutex);
                gsr = cluon::extractMessage<opendlv::proxy::GroundSteeringRequest>(std::move(env));
                // std::cout << "lambda: groundSteering = " << gsr.groundSteering() << std::endl;
            };

            od4.dataTrigger(opendlv::proxy::GroundSteeringRequest::ID(), onGroundSteeringRequest);

            // Revieve incoming steering commands from the ML model, and update the steering angle
            SteeringCommand sc;
            auto onPythonMessage = [&sc, &MLSteeringAngle](cluon::data::Envelope &&env)
            {
                sc = cluon::extractMessage<SteeringCommand>(std::move(env));
                MLSteeringAngle = sc.steeringAngle();
            };

            od4.dataTrigger(SteeringCommand::ID(), onPythonMessage);

            // cv::namedWindow("Combined Color tracking", cv::WINDOW_AUTOSIZE);
            // cv::createTrackbar("maxContourArea", "Combined Color tracking", &maxContourArea, 2500);
            // cv::createTrackbar("minContourArea", "Combined Color tracking", &minContourArea, 2500);

            DirectionCalculator directionCalculator;
            AngleCalculator angleCalculator;

            // Car position on the X axis
            // const int carPositionX = 320;

            const float maxSteering = 0.3f;
            const float minSteering = -0.3f;
            int frameCount = 0;

            // Endless loop; end the program by pressing Ctrl-C.
            while (od4.isRunning())
            {

                frameCount++; // Count the number of frames processed.

                // OpenCV data structure to hold an image.

                cv::Mat img;
                cv::Mat hsvImg; // HSV Image
                cv::Mat finalThresh;

                // Wait for a notification of a new frame.
                sharedMemory->wait();

                // Lock the shared memory.
                sharedMemory->lock();
                {
                    // Copy the pixels from the shared memory into our own data structure.
                    cv::Mat wrapped(HEIGHT, WIDTH, CV_8UC4, sharedMemory->data());
                    img = wrapped.clone();
                }

                // We start off by detecting if the track is moving in a clockwise or counter-clockwise direction.
                if (frameCount % 15 == 0 || frameCount < 10)
                {
                    direction = directionCalculator.CalculateDirection(img, direction, VERBOSE);
                    if (direction == -1)
                    {
                        if (VERBOSE)
                        {
                            std::cout << "Direction: Clockwise" << std::endl;
                        }
                    }
                    else if (direction == 1)
                    {
                        if (VERBOSE)
                        {
                            std::cout << "Direction: Counter-Clockwise" << std::endl;
                        }
                    }
                    else
                    {
                        if (VERBOSE)
                        {
                            std::cout << "Direction: No direction" << std::endl;
                        }
                    }
                }
                // Crop bottom half. Only bottom 50% part will be used for processing and contour tracking.
                cv::Rect roi(0, img.rows / 2, img.cols, img.rows / 2);
                cv::Mat croppedImg = img(roi);

                // inRange filters out blue colors. Use gaussian blur to smooth out image, and morphological operations
                // Erode makes objects smaller but fills in the holes. Dilate does the opposite, so if you combine them
                // it will make a nice end result
                cv::cvtColor(croppedImg, hsvImg, CV_BGR2HSV);

                cv::Mat blueThreshImg = colorSeparator.detectBlueColor(hsvImg, VERBOSE);
                cv::Mat yellowThreshImg = colorSeparator.detectYellowColor(hsvImg, VERBOSE);

                yellowThreshImg = noiseRemover.RemoveNoise(yellowThreshImg);
                blueThreshImg = noiseRemover.RemoveNoise(blueThreshImg);

                // cv::Mat yellowContourOutput = contourFinder.FindContours(yellowThreshImg, img, minContourArea, maxContourArea);
                // cv::Mat blueContourOutput = contourFinder.FindContours(blueThreshImg, img, minContourArea, maxContourArea);

                // If clockwise map, blue cones on left side, yellow cones on right side.
                // If counter-clockwise map, blue cones on right side, yellow cones on left side.
                if (direction == 1)
                {
                    // use ml steering angle
                    steeringWheelAngle = MLSteeringAngle;
                }
                else

                {
                    bool isClockwise = (direction == -1);
                    steeringWheelAngle = angleCalculator.CalculateSteeringAngle(yellowThreshImg, blueThreshImg, steeringWheelAngle, isClockwise, maxSteering, minSteering, VERBOSE);
                }

                // cv::bitwise_or(blueContourOutput, yellowContourOutput, finalThresh);

                // Now we can combine the contours with the original picture. The reason for doing this is to eliminate the noise
                // in the top part of the picture, and the contour processing is only done on 50% of the original image, which should
                // increase performance.
                // cv::Mat finalOutput;
                // cv::addWeighted(img, 1.0, finalThresh, 1.0, 0.0, finalOutput);

                // TODO: Here, you can add some code to check the sampleTimePoint when the current frame was captured.

                std::pair<bool, cluon::data::TimeStamp> ts = sharedMemory->getTimeStamp();

                int64_t sampleTimePoint = cluon::time::toMicroseconds(ts.second);
                std::string ts_string = std::to_string(sampleTimePoint);

                sharedMemory->unlock();

                // If you want to access the latest received ground steering, don't forget to lock the mutex:
                {
                    std::lock_guard<std::mutex> lck(gsrMutex);
                    float actualSteering = gsr.groundSteering();
                    // group_XY;sampleTimeStamp in microseconds;steeringWheelAngle

                    float lowerBound = std::min(actualSteering * 0.75f, actualSteering * 1.25f);
                    float upperBound = std::max(actualSteering * 0.75f, actualSteering * 1.25f);

                    bool isWithinRange = (steeringWheelAngle >= lowerBound) && (steeringWheelAngle <= upperBound);
                    std::cout << "group_16;" << ts_string << ";" << steeringWheelAngle << std::endl;
                    if (actualSteering != 0.0)
                    {

                        if (isWithinRange)
                        {
                            totalWithinRange++;
                            // print within range frames
                            if (VERBOSE)
                            {
                                std::cout << "total frames within range: " << totalWithinRange << " frames:" << totalEntries << "\n";
                            }
                        }
                        totalEntries++;
                    }

                    // write to file
                    outputFile << ts_string << "," << steeringWheelAngle << "," << gsr.groundSteering() << "\n";

                    // check if the steering angle is within +-25% of the actual steering
                }

                // Display image on your screen.
                if (VERBOSE)
                {
                    // cv::imshow(sharedMemory->name().c_str(), img);
                    // cv::imshow("ResultImg", img);
                    // cv::imshow("Blue", blueContourOutput);
                    // cv::imshow("Yellow", yellowContourOutput);
                    // cv::imshow("Combined Color tracking", finalOutput);

                    cv::waitKey(1);
                }
            }
        }
        retCode = 0;

        std::cout << "Total entries: " << totalEntries << std::endl;
        std::cout << "Total within range: " << totalWithinRange << std::endl;
        // print percentage of frames within range
        float percentageWithinRange = (static_cast<float>(totalWithinRange) / totalEntries) * 100.0f;
        std::cout << "Percentage of frames within range: " << percentageWithinRange << "%" << std::endl;
    }

    // Close the file
    outputFile.close();

    return retCode;
}
