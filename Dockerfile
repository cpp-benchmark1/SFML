# Use Ubuntu 24.04 as base image
FROM ubuntu:24.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install essential build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libx11-dev \
    libxrandr-dev \
    libudev-dev \
    libgl1-mesa-dev \
    libflac-dev \
    libogg-dev \
    libvorbis-dev \
    libopenal-dev \
    libfreetype6-dev \
    libxi-dev \
    libxcursor-dev \
    libxinerama-dev \
    libxrandr-dev \
    libxrender-dev \
    libxfixes-dev \
    libxext-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /sfml

# Copy the source code
COPY . .

# Create build directory
RUN mkdir build

# Configure and build the project
RUN cmake -B build && \
    cmake --build build

# Set the entry point
ENTRYPOINT ["/bin/bash"] 