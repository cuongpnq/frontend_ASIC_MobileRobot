#!/bin/bash
set -e

IMAGE_NAME="frontend_asic_build"

xhost +local:docker

docker run --rm -it \
    --device /dev/dri \
    -e DISPLAY=$DISPLAY \
    -e QT_QPA_PLATFORM=xcb \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v "$(pwd)":/app \
    -w /app \
    "$IMAGE_NAME" \
    bash -c "./build-output/frontend/frontend_app"