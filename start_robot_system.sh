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
$SERVER_BIN -m "$MODEL_PATH" --port 8080 --host 0.0.0.0 --ctx-size 1024 --threads 4 --n-gpu-layers 33 -fa on -np 1 > llama_server.log 2>&1 &
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
echo "[SYSTEM] Launching Robot GUI..."
export QT_QPA_PLATFORM=xcb
$APP_BIN

# 4. Cleanup on exit
echo "[SYSTEM] Shutting down Robot System..."
kill $SERVER_PID
pkill -f llama-server
