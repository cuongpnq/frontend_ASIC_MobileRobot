# --- Robot AI Frontend Makefile ---
SHELL := /bin/bash
BUILD_DIR ?= build-output

.PHONY: help setup build run run-only clean distclean test-native test-real

# Default: Show help
help:
	@echo "===================================================="
	@echo "      ASIC MOBILE ROBOT AI SYSTEM CONTROL"
	@echo "===================================================="
	@echo "  make setup       - Install system deps & AI core"
	@echo "  make build       - Compile the entire system (GUI + AI)"
	@echo "  make run         - Execute system orchestration (uses flags)"
	@echo "  make test-native - Run headless native automation tests"
	@echo "  make test-real   - Run interactive real-device tests"
	@echo ""
	@echo "  Flags for 'make run':"
	@echo "    BUILD=ON      - Forces a rebuild before running"
	@echo "    RUN_APP=ON     - Launches only the QML GUI"
	@echo ""
	@echo "  Example: make run BUILD=ON"
	@echo "  make clean       - Remove build folders and logs"
	@echo "  make distclean   - Reset project (Deletes AI source code)"
	@echo "===================================================="

# Flags (OFF by default)
BUILD ?= OFF
RUN_APP ?= OFF

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
		libreoffice-impress \
		wget git cmake build-essential curl zstd
	@echo "[SETUP] Cloning AI inference core..."
	chmod +x ./setup_ollama.sh && ./setup_ollama.sh

# 2. Build GUI & AI Server
build:
	@echo "[BUILD] Compiling AI System..."
	source /opt/ros/foxy/setup.bash && cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Release
	source /opt/ros/foxy/setup.bash && cmake --build $(BUILD_DIR) -j$$(nproc)

# 3. Launch System Orchestrator
run:
ifeq ($(BUILD), ON)
	@$(MAKE) build
endif
	@chmod +x ./start_robot_system.sh
ifeq ($(RUN_APP), ON)
	@. /opt/ros/foxy/setup.bash && ./build-output/frontend/frontend_app
else
	@./start_robot_system.sh
endif

shortcut:
	@echo "Creating desktop shortcut..."
	@sed "s|{{PROJECT_ROOT}}|$(CURDIR)|g" RobotControl.desktop > ~/Desktop/RobotControl.desktop
	@chmod +x ~/Desktop/RobotControl.desktop
	@gio set ~/Desktop/RobotControl.desktop metadata::trusted true || true
	@echo "Shortcut created and enabled on Desktop."

# 5. Testing and Validation
test-native:
	@echo "[TEST] Running Headless Native Automation Tests..."
	@chmod +x ./run_native_automation_tests.py
	@./run_native_automation_tests.py

test-real:
	@echo "[TEST] Running Interactive Real Device Verification..."
	@chmod +x ./run_real_device_interactive_tests.py
	@./run_real_device_interactive_tests.py

# 4. Cleanup
clean:
	@echo "[CLEAN] Removing build files and logs..."
	rm -rf $(BUILD_DIR)
	rm -f llama_server.log

distclean: clean
	@echo "[DISTCLEAN] Removing AI source code..."
	rm -rf frontend/3rdparty/llama.cpp