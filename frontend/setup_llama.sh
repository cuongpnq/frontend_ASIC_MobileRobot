#!/bin/bash

# Configuration
FRONTEND_DIR="/home/kien/Development/frontend_ASIC_MobileRobot/frontend"
THIRD_PARTY_DIR="$FRONTEND_DIR/3rdparty"
LLAMA_REPO="https://github.com/ggerganov/llama.cpp"
MODEL_URL="https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf/resolve/main/Phi-3-mini-4k-instruct-q4.gguf"
MODEL_NAME="phi-3-mini.gguf"

echo "Setting up llama.cpp and AI Models for Jetson Xavier..."

# 1. Create 3rdparty directory
mkdir -p "$THIRD_PARTY_DIR"
cd "$THIRD_PARTY_DIR"

# 2. Clone llama.cpp if not exists
if [ ! -d "llama.cpp" ]; then
    echo "Cloning llama.cpp..."
    git clone "$LLAMA_REPO"
else
    echo "llama.cpp already exists, skipping clone."
fi

# 3. Create models directory and download model
echo "Checking for AI model..."
mkdir -p "$FRONTEND_DIR/models"
if [ ! -f "$FRONTEND_DIR/models/$MODEL_NAME" ]; then
    echo "Downloading $MODEL_NAME from Hugging Face..."
    wget -c "$MODEL_URL" -O "$FRONTEND_DIR/models/$MODEL_NAME"
    if [ $? -eq 0 ]; then
        echo "Model downloaded successfully."
    else
        echo "Error: Failed to download model. Please check your internet connection."
        exit 1
    fi
else
    echo "Model $MODEL_NAME already exists, skipping download."
fi

echo "===================================================="
echo "Setup complete!"
echo "AI Core: Ready in $THIRD_PARTY_DIR/llama.cpp"
echo "Model:   Ready in $FRONTEND_DIR/models/$MODEL_NAME"
echo "===================================================="
echo "Next step: Run 'make system' to launch the robot brain."
