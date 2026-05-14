#!/bin/bash
# Robot AI System Launcher
cleanup() {
    echo "[SYSTEM] Shutting down Robot System..."
    [ ! -z "$SERVER_PID" ] && kill $SERVER_PID
    pkill -f llama-server
    exit
}
trap cleanup SIGINT SIGTERM EXIT

# Pre-Cleanup: Ensure no existing servers are on port 8080
pkill -f llama-server

# 2. Build or verify paths
PROJECT_ROOT=$(pwd)
SERVER_BIN="$PROJECT_ROOT/build-output/frontend/build-output/llama-server/llama-server"
MODEL_PATH="$PROJECT_ROOT/frontend/models/phi-3-mini.gguf"
APP_BIN="$PROJECT_ROOT/build-output/frontend/frontend_app"

if [ ! -f "$SERVER_BIN" ]; then
    # Fallback to build output root search
    SERVER_BIN=$(find build-output -type f -name "llama-server" | head -n 1)
fi

if [ ! -x "$SERVER_BIN" ]; then
    echo "[ERROR] AI Server binary not found or not executable: $SERVER_BIN"
    exit 1
fi

if [ ! -f "$MODEL_PATH" ]; then
    echo "[ERROR] Model file not found: $MODEL_PATH"
    exit 1
fi

echo "[SYSTEM] Starting AI Server in background..."
# Run the server on port 8080. Using GPU layers if possible.
# Optimized for Jetson Xavier:
#   --ctx-size 512:    Prompts are ~300 tokens max. Saves ~576 MiB KV cache vs 2048.
#   --threads 6:       Xavier NX has 6 ARM cores. Use all of them.
#   --batch-size 256:  Smaller batch = faster prompt processing on limited RAM.
#   --ubatch-size 128: Micro-batch optimization for ARM architecture.
#   --mlock:           Pin model in RAM, prevent OS from swapping to disk.
#   --no-warmup:       Skip warmup run, saves 1-2s on startup.
$SERVER_BIN -m "$MODEL_PATH" --port 8080 --host 0.0.0.0 \
    --ctx-size 512 --threads 6 --n-gpu-layers 33 \
    --batch-size 256 --ubatch-size 128 \
    -fa on -np 1 --mlock --no-warmup > llama_server.log 2>&1 &
SERVER_PID=$!

# Wait for server to be ready
echo "[SYSTEM] Waiting for AI Server to initialize..."
while ! curl -s --connect-timeout 2 --max-time 5 http://localhost:8080/health | grep -q "ok"; do
    if ! kill -0 $SERVER_PID 2>/dev/null; then
        echo "[ERROR] AI Server process died unexpectedly."
        echo "Last 10 lines of llama_server.log:"
        tail -n 10 llama_server.log
        exit 1
    fi
    sleep 1
done
echo "[SYSTEM] AI Server is READY."

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

# Disable Shared Memory and increase discovery resilience (fixes "send_goal failed")
# export RMW_FASTRTPS_USE_SHM=0
# export FASTRTPS_DEFAULT_PROFILES_FILE=""

# Run Frontend
$APP_BIN

# 4. Cleanup on exit
echo "[SYSTEM] Shutting down Robot System..."
kill $SERVER_PID
pkill -f llama-server
