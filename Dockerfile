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
    libcurl4-openssl-dev \
    gdb \
    valgrind \
    libxml2 \
    libxml2-dev \
    libmongoc-dev \
    libbson-dev \
    libpq-dev \
    && rm -rf /var/lib/apt/lists/* 

# Set working directory
WORKDIR /sfml

# Copy the source code
COPY . .

# Configure and build the project with debug flags and SFML modules
RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSFML_BUILD_SYSTEM=ON \
    -DSFML_BUILD_WINDOW=ON \
    -DSFML_BUILD_GRAPHICS=ON \
    -DSFML_BUILD_AUDIO=ON \
    -DSFML_BUILD_NETWORK=ON \
    -DSFML_BUILD_EXAMPLES=ON \
    -DSFML_BUILD_DOC=OFF \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_CXX_FLAGS="-fPIC -Wall -Wextra -Werror=return-type -Wno-unused-parameter" \
    -DCMAKE_C_FLAGS="-fPIC -Wall -Wextra"

# Build the project
RUN cmake --build build --config Debug -j$(nproc) || (find build -name "*.log" -type f -exec cat {} \; && exit 1)

# Set environment variables for testing
ENV LD_LIBRARY_PATH=/sfml/build/lib

# Set the entry point
ENTRYPOINT ["/bin/bash"] 

