BUILD_DIR ?= build-output
CLEAN_BUILD ?= OFF

.PHONY: qtcreator debug_native run_app build_run

qtcreator:
	qtcreator CMakeLists.txt &

debug_native:
	@if [ "$(CLEAN_BUILD)" = "ON" ]; then \
		echo "Removing previous builds..."; \
		rm -rf $(BUILD_DIR); \
	fi
	cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BUILD_DIR) -j$$(nproc)

run_app:
	./$(BUILD_DIR)/frontend/frontend_app

build_run:
	make debug_native && ./$(BUILD_DIR)/frontend/frontend_app