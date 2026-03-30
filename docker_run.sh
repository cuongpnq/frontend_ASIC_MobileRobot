#!/bin/bash
set -e

IMAGE_NAME="frontend_asic_build"
APP_PATH="./build-output/frontend/frontend_app"

if [ ! -f "$APP_PATH" ]; then
    echo "App not found: $APP_PATH"
    exit 1
fi

SESSION_TYPE="${XDG_SESSION_TYPE:-}"

echo "Detected session type: ${SESSION_TYPE:-unknown}"

if [ "$SESSION_TYPE" = "wayland" ] && [ -n "$WAYLAND_DISPLAY" ] && [ -n "$XDG_RUNTIME_DIR" ] && [ -S "$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY" ]; then
    echo "Running with Wayland..."

    docker run --rm -it \
        --device /dev/dri \
        -e QT_QPA_PLATFORM=wayland \
        -e WAYLAND_DISPLAY="$WAYLAND_DISPLAY" \
        -e XDG_RUNTIME_DIR=/tmp/runtime-host \
        -v "$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY:/tmp/runtime-host/$WAYLAND_DISPLAY" \
        -v "$(pwd):/app" \
        -w /app \
        "$IMAGE_NAME" \
        bash -c 'mkdir -p /tmp/runtime-host && chmod 700 /tmp/runtime-host && "'"$APP_PATH"'"'

elif [ -n "$DISPLAY" ] && [ -S /tmp/.X11-unix/X0 -o -d /tmp/.X11-unix ]; then
    echo "Running with X11..."

    xhost +local:docker >/dev/null 2>&1 || true

    docker run --rm -it \
        --device /dev/dri \
        -e DISPLAY="$DISPLAY" \
        -e QT_QPA_PLATFORM=xcb \
        -e QT_QUICK_BACKEND=software \
        -v /tmp/.X11-unix:/tmp/.X11-unix \
        -v "$(pwd):/app" \
        -w /app \
        "$IMAGE_NAME" \
        bash -c "$APP_PATH"

else
    echo "No usable display backend detected."
    echo "XDG_SESSION_TYPE=$SESSION_TYPE"
    echo "DISPLAY=${DISPLAY:-<empty>}"
    echo "WAYLAND_DISPLAY=${WAYLAND_DISPLAY:-<empty>}"
    echo "XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR:-<empty>}"
    exit 1
fi