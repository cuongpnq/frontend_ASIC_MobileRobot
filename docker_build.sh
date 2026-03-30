#!/bin/bash
set -e

IMAGE_NAME="frontend_asic_build"
CONTAINER_NAME="frontend_asic_builder"

echo "=========================================="
echo "Building Docker image: $IMAGE_NAME"
echo "=========================================="
docker build -t "$IMAGE_NAME" .

echo "=========================================="
echo "Cleaning old build artifacts"
echo "=========================================="
rm -rf build build-output CMakeCache.txt CMakeFiles

echo "=========================================="
echo "Running build inside Docker container"
echo "=========================================="
docker run --rm \
    --name "$CONTAINER_NAME" \
    -v "$(pwd)":/app \
    -w /app \
    "$IMAGE_NAME" \
    bash -c "cmake -S . -B build-output -G Ninja && cmake --build docker-build-output -j\$(nproc)"

echo "=========================================="
echo "Build completed successfully!"
echo "Executables and build artifacts are in: docker-build-output/"
echo "=========================================="