FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-22.04

# Install system-level libraries
RUN apt update && apt install -y \
    libstdc++6 \
    libgcc-s1 \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy your binary and Boost dynamic libs into the image
COPY bin/app ./app
COPY lib/boost_1_88_0/stage/lib/ /usr/local/lib/

# Set runtime linker path to include custom boost location
ENV LD_LIBRARY_PATH=/usr/local/lib

# Run the app by default
CMD ["./app"]

