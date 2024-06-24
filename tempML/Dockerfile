# Use an official Ubuntu 18.04 as a parent image
FROM ubuntu:18.04

# Set environment variables to non-interactive mode
ENV DEBIAN_FRONTEND=noninteractive

# Install required dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    software-properties-common \
    build-essential \
    python3 \
    python3-pip \
    python3-dev \
    python3-numpy \
    python3-opencv \
    python3-protobuf \
    protobuf-compiler \
    git \
    make \
    wget && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Add the libcluon PPA and install libcluon
RUN add-apt-repository ppa:chrberger/libcluon && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    libcluon && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


# Set the working directory
WORKDIR /app

# copy current directory into the Docker container
COPY . /app

# Install Python dependencies
RUN pip3 install --no-cache-dir joblib pandas scikit-learn

# Run make to generate Python files from the message specifications
RUN make

# Copy your .rec file into the Docker container (replace example.rec with your actual .rec file)


# Run the Python script
CMD ["python3", "receiveEnvelopes.py"]
