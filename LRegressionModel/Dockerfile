# Use an official Python runtime as a parent image
FROM python:3.8-slim

# Set the working directory in the container
WORKDIR /usr/src/app

# Copy the current directory contents into the container at /usr/src/app
COPY . /usr/src/app

# Install any needed packages specified in requirements.txt
RUN pip install --no-cache-dir joblib pycluon pandas scikit-learn

# You may need to download and install other dependencies for pycluon manually if they are not available via pip
# Example: RUN apt-get update && apt-get install -y some-package

# Run your script when the container launches
CMD ["python", "./cluontest.py"]
