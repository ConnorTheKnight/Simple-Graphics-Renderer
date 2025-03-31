FROM ubuntu:20.04

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    freeglut3-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libglew-dev \
    libxmu-dev \
    libxi-dev \
    x11-apps \
    mesa-utils \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Copy source code
COPY main.cpp /app/

# Compile the application with thread support
RUN g++ -o shape_renderer main.cpp -lGL -lGLU -lglut -lm -pthread -std=c++11

# Command to run when container starts
CMD ["./shape_renderer"]