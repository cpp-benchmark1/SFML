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
    libcurl4-openssl-dev \
    gdb \
    valgrind \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /sfml

# Copy the source code
COPY . .

# Create build directory and configure with debug flags
RUN mkdir -p build && \
    cd build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=Debug \
        -DSFML_BUILD_EXAMPLES=ON \
        -DSFML_BUILD_DOC=OFF \
        -DSFML_BUILD_NETWORK=ON \
        -DSFML_BUILD_AUDIO=ON \
        -DSFML_BUILD_GRAPHICS=ON \
        -DSFML_BUILD_WINDOW=ON \
        -DSFML_BUILD_SYSTEM=ON

# Build the project
RUN cd build && make -j$(nproc)

# Set environment variables for testing
ENV LD_LIBRARY_PATH=/sfml/build/lib

# Set the entry point
ENTRYPOINT ["/bin/bash"] 

