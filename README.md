![pipeline status](https://git.chalmers.se/courses/dit638/students/2024-group-16/badges/main/pipeline.svg)

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
