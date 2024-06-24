## Introduction

This project showcases the development of software features enabling a miniature car to drive autonomously using two distinct approaches: computer vision and machine learning. Our goal was to design, implement, and test these algorithms in a controlled environment, leveraging data from an RC vehicle equipped with various sensors such as a camera, infrared sensors, magnetic field sensors.

## Project Overview
In this project, we developed two main approaches to autonomous driving for a miniature car, focusing on different driving directions and technologies.

### Clockwise Approach: Computer Vision
The counter-clockwise driving algorithm utilizes computer vision techniques, primarily using the OpenCV library. The core of this approach involves tracking cones on the track using HSV filters and object detection methods. Our computer vision approach starts by determining the track direction—clockwise or counter-clockwise—based on the placement of blue and yellow cones in the initial frames. We split the image into left and right parts and use HSV color filtering to isolate the cones, which helps us identify their positions accurately regardless of lighting conditions.

To enhance cone visibility and reduce noise, we apply Gaussian blur, followed by erosion and dilation. This preprocessing ensures that the cones stand out clearly from the background. After cleaning the image, we identify the contours of the cones and calculate their geometric centers (centroids).

We then divide the image into left, center, and right areas to determine the car's steering direction. Knowing the track direction, we assess the position of the cones: if a cone appears in an unexpected area, the car adjusts its steering accordingly. For example, in a clockwise track, blue cones should be on the left. If a blue cone is detected in the right area of the left section, the car steers right. Conversely, yellow cones influence left turns. The position of the cones along the X-axis determines the sharpness of the steering angle, allowing the car to navigate the track smoothly and autonomously.

### Counter-Clockwise Approach: Machine Learning
In our machine learning approach, we used data from the angular velocity and ground steering sensors to predict steering angles. We synchronized the data by rounding timestamps and merging records to align the sensor data accurately. This preprocessed data was fed into a Random Forest Regression model, trained on historical data to predict steering angles.

After training, we evaluated the model's accuracy using reserved test data and continuously fine-tuned it. The model was then deployed to a steering prediction service that processed real-time sensor data and predicted steering angles, which were sent back to the core service for execution.

We used scikit-learn for its flexibility in testing different algorithms, ultimately finding that Random Forest provided the best accuracy (25-45%). This approach allowed us to effectively navigate the track by leveraging sensor data correlations.

### Key Features
- Algorithm Development: Designed and implemented algorithms for autonomous driving using computer vision and machine learning techniques.
- Data Utilization: Utilized data from scaled vehicles to develop, test, and evaluate the driving algorithms.
- Computer Vision Techniques: Employed HSV filters and object detection techniques to track cones and guide the car's movements.
- Machine Learning Model: Trained a machine learning model on previous track data to predict steering actions for autonomous navigation.
- Testing and Evaluation: Rigorously tested and evaluated the algorithms to ensure they meet performance requirements.
- Documentation: Documented the entire development process, including conceptual ideas, algorithmic fundamentals, system architecture, implementation details, test methods, and project retrospectives.
- This project demonstrates the integration of theoretical knowledge and practical skills in developing autonomous driving technologies. Through this hands-on experience, we gained valuable insights into the complexities and intricacies of self-driving vehicle development.

![Autonomous Car](https://i.ibb.co/vJ5dKxf/ezgif-3-b4229a2a26.webp)

![Autonomous Car OpenCV](https://i.ibb.co/PMYcf27/ezgif-7-e2260e26a0.webp)

![System Design](https://i.imgur.com/BNyVE5Y.png)


## Prerequisites

### For Running with Docker

To build and run this project using Docker, you will need:

- Docker installed on your machine. Visit the [official Docker documentation](https://docs.docker.com/get-docker/) to get started.

## Setup and Build Instructions

Clone this repository to your local machine:

```bash
git clone git@git.chalmers.se:courses/dit638/students/2024-group-16.git
cd 2024-group-16
```

### Using Docker

Build the Docker image:

```bash
docker build -f Dockerfile -t 2024-group-16 .
```

Run the Docker container:

```bash
docker run --rm -ti --net=host --ipc=host -e DISPLAY=$DISPLAY -v /tmp:/tmp 2024-group-16:latest --cid=253 --name=img --width=640 --height=480 --verbose
```

append verbose for debbuging mode

### For Local Development

To build and run this project on your local machine without Docker, you will need:

- CMake: For generating makefiles and managing the build process.
- A C++ compiler (e.g., GCC or Clang) and `build-essential` package on Linux for compiling the project.

On Ubuntu, you can install these with the following commands:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake
```

Create a build directory and navigate to it:

```bash
mkdir build && cd build
```

Generate makefiles using CMake:

```bash
cmake ..
```

Build the project:

```bash
make
```

Run the project:

```bash
./main --net=host --ipc=host -e --cid=253 --name=img --width=640 --height=480 --verbose
```

Optionally, just run the bash script:

```bash
./run_locally.sh
```

## Adding New Features

1. **Feature Branches:** New features are developed in separate branches (feature branches) created from the main development branch. This isolates the work on the new feature from the main codebase and ongoing development.

   ```bash
   git checkout -b feature/<feature-name>
   ```

2. **Continuous Integration (CI):** TODO

3. **Code Review:** Once the feature is implemented, a code review is conducted to ensure that the code meets the project's standards

4. **Merge and Deploy:** After the code review is approved, the feature branch is merged into the main development branch.

### Fixing Unexpected Behavior in Existing Features

1. **Issue Tracking:** Any unexpected behavior or bugs in the application are reported as issues in our [Trello kanban](https://trello.com/b/todo)
2. **Bugfix Branches:** Similar to feature branches, we create separate branches for fixing bugs. This allows us to isolate the bug fix from the main codebase and ongoing development, prefixing the branch name with `bugfix/`.

   ```bash
   git checkout -b bugfix/<bugfix-name>
   ```

3. **Code Review:** After the bug is fixed, a code review is conducted to ensure that the fix is correct and does not introduce new issues.

4. **Merge and Verification:** Once approved, the bugfix branch is merged into the main development.

### Commit Message Guidelines

We will follow the guides for commit messages that can be found here: https://cbea.ms/git-commit/

1. Separate subject from body with a blank line
2. Limit the subject line to 50 characters
3. Capitalize the subject line
4. Do not end the subject line with a period
5. Use the imperative mood in the subject line
6. Wrap the body at 72 characters
7. Use the body to explain what and why vs. how
