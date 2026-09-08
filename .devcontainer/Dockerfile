FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    linux-tools-generic \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
