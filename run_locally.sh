#!/bin/bash

# Define the directory for the build
BUILD_DIR=build

# Remove the existing build directory if it exists
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi

# Create a new build directory
mkdir "$BUILD_DIR"

# Change to the build directory
cd "$BUILD_DIR"

# Run CMake to configure the project and generate a Makefile
cmake ..

# Build the project
make

# Optionally, change to the directory containing the executable if it's different
#cd path/to/executable

# Run the executable
./main --net=host --ipc=host -e --cid=253 --name=img --width=640 --height=480 --verbose
