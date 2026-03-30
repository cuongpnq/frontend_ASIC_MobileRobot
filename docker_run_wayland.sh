#!/bin/bash
set -e

IMAGE_NAME="frontend_asic_build"

docker run --rm -it \
    --device /dev/dri \
    -e WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
    -e XDG_RUNTIME_DIR=/tmp/runtime-host \
    -e QT_QPA_PLATFORM=wayland \
    -v $XDG_RUNTIME_DIR/$WAYLAND_DISPLAY:/tmp/runtime-host/$WAYLAND_DISPLAY \
    -v "$(pwd)":/app \
    -w /app \
    "$IMAGE_NAME" \
    bash -c "./build-output/frontend/frontend_app"