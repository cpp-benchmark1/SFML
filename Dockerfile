# Use Ubuntu 24.04 as base image
FROM ubuntu:24.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install essential build tools and dependencies

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libx11-dev \
    libxrandr-dev \
    libudev-dev \
    libgl1-mesa-dev \
    libflac-dev \
    libogg-dev \
    libvorbis-dev \
    libopenal-dev \
    libfreetype6-dev \
# Use Ubuntu 24.04 as base image
FROM ubuntu:24.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install required packages
RUN apt-get update && apt-get install -y \
    libjpeg-dev \
    libpng-dev \
    libssl-dev \
    libmongoc-dev \
    libbson-dev \
    libmysqlclient-dev \
    libxcursor-dev \
    libxi-dev \
    libxinerama-dev \
    libxrandr-dev \
    libxrender-dev \
    libxfixes-dev \
    libxext-dev \
    cmake \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /sfml

# Copy the source code
COPY . .

# Configure and build the project
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --config Release

# Set the entry point
ENTRYPOINT ["/bin/bash"]
