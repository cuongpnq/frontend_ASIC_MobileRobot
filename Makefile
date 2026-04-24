# --- Robot AI Frontend Makefile ---
BUILD_DIR ?= build-output

.PHONY: help setup build run run-only clean distclean

# Default: Show help
help:
	@echo "===================================================="
	@echo "      ASIC MOBILE ROBOT AI SYSTEM CONTROL"
	@echo "===================================================="
	@echo "  make setup     - Install system deps & AI core"
	@echo "  make build     - Compile the entire system (GUI + AI)"
	@echo "  make run       - Build and Launch (AI Server + QML GUI)"
	@echo "  make run-only  - Launch without building (AI Server + QML GUI)"
	@echo "  make clean     - Remove build folders and logs"
	@echo "  make distclean - Reset project (Deletes AI source code)"
	@echo "===================================================="

# 1. System Setup
setup:
	@echo "[SETUP] Installing Jetson dependencies..."
	sudo apt update && sudo apt install -y \
		libqt5virtualkeyboard5-dev \
		qml-module-qtquick-virtualkeyboard \
		qtvirtualkeyboard-plugin \
		qml-module-qt-labs-folderlistmodel \
		qml-module-qt-labs-settings \
		qml-module-qtquick2 \
		qml-module-qtquick-window2 \
		qml-module-qtquick-controls2 \
		qml-module-qtquick-layouts \
		qml-module-qtgraphicaleffects \
		wget git cmake build-essential curl
	@echo "[SETUP] Cloning AI inference core..."
	chmod +x ./frontend/setup_llama.sh && ./frontend/setup_llama.sh

# 2. Build GUI & AI Server
build:
	@echo "[BUILD] Compiling AI System..."
	cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR) -j$$(nproc)

# 3. Launch System Orchestrator
run: build
	@chmod +x ./start_robot_system.sh
	@./start_robot_system.sh

run-only:
	@chmod +x ./start_robot_system.sh
	@./start_robot_system.sh

# 4. Cleanup
clean:
	@echo "[CLEAN] Removing build files and logs..."
	rm -rf $(BUILD_DIR)
	rm -f llama_server.log

distclean: clean
	@echo "[DISTCLEAN] Removing AI source code..."
	rm -rf frontend/3rdparty/llama.cpp