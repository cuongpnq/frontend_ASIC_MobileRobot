#!/bin/bash
# Robot AI System Launcher with Ollama Integration

cleanup() {
    echo "[SYSTEM] Shutting down Robot System..."
    exit
}
trap cleanup SIGINT SIGTERM EXIT

PROJECT_ROOT=$(pwd)
APP_BIN="$PROJECT_ROOT/build-output/frontend/frontend_app"

# 1. Verify that the QML app binary exists
if [ ! -f "$APP_BIN" ]; then
    # Fallback to build output root search
    APP_BIN=$(find build-output -type f -name "frontend_app" | head -n 1)
fi

if [ ! -x "$APP_BIN" ]; then
    echo "[ERROR] Robot GUI App binary not found or not executable: $APP_BIN"
    echo "Please build the application first: make build"
    exit 1
fi

# 2. Orchestrate Ollama Server
echo "[SYSTEM] Checking if Ollama Server is running..."

# Pre-check: Verify Ollama is installed
if ! command -v ollama &>/dev/null; then
    echo -e "\033[0;31m[ERROR] Ollama is not installed on this system.\033[0m"
    echo -e "\033[1;33mPlease install and configure Ollama first by running:\033[0m"
    echo -e "  \033[0;32mmake setup\033[0m"
    exit 1
fi

if ! curl -s --connect-timeout 2 http://localhost:11434/ >/dev/null; then
    echo "[SYSTEM] Ollama is not active. Attempting to start Ollama system service..."
    sudo systemctl start ollama || true
fi

# Wait for Ollama to accept connections
while ! curl -s --connect-timeout 2 http://localhost:11434/ >/dev/null; do
    echo "[SYSTEM] Waiting for Ollama Server to be ready on port 11434..."
    sleep 1
done
echo "[SYSTEM] Ollama Server is READY."

# Verify that Qwen 2.5 model is downloaded, otherwise pull it
echo "[SYSTEM] Verifying Qwen 2.5 model availability..."
if ! ollama list | grep -q "qwen2.5"; then
    echo "[SYSTEM] qwen2.5 model not found locally. Pulling from Ollama registry..."
    ollama pull qwen2.5
else
    echo "[SYSTEM] qwen2.5 model is ready."
fi

# 3. Launch Frontend App
echo "[SYSTEM] Sourcing ROS environment..."
# Source the real robot workspace (adjust path if necessary)
if [ -f "$PROJECT_ROOT/../mobile_robot_asiclab/install/setup.bash" ]; then
    source "$PROJECT_ROOT/../mobile_robot_asiclab/install/setup.bash"
else
    echo "[WARN] Real robot workspace logic setup.bash not found. Sourcing standard ROS 2 Foxy..."
    source /opt/ros/foxy/setup.bash || echo "[ERROR] Could not source ROS 2."
fi

echo "[SYSTEM] Launching Robot GUI..."
export QT_QPA_PLATFORM=xcb

# Run Frontend
$APP_BIN

# 4. Cleanup on exit
echo "[SYSTEM] Shutting down Robot System..."
