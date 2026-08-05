#!/usr/bin/env bash

set -euo pipefail

if ! command -v sudo >/dev/null 2>&1; then
    echo "sudo is required to install dependencies."
    exit 1
fi

sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    qt6-base-dev \
    qt6-multimedia-dev \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-libav