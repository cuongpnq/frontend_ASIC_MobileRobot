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
    SERVER_BIN=$(find build-output -name "llama-server" | head -n 1)
fi

echo "[SYSTEM] Starting AI Server in background..."
# Run the server on port 8080. Using CPU if no CUDA is found.
$SERVER_BIN -m "$MODEL_PATH" --port 8080 --host 0.0.0.0 --ctx-size 512 --threads 6 > llama_server.log 2>&1 &
SERVER_PID=$!

# Wait for server to be ready
echo "[SYSTEM] Waiting for AI Server to initialize..."
until curl -s http://localhost:8080/health | grep -q "ok"; do
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
