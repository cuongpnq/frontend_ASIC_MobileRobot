BUILD_DIR ?= build-output
CLEAN_BUILD ?= OFF

.PHONY: qtcreator debug_native

qtcreator:
	qtcreator CMakeLists.txt &

debug_native:
	@if [ "$(CLEAN_BUILD)" = "ON" ]; then \
		echo "Removing previous builds..."; \
		rm -rf $(BUILD_DIR); \
	fi
	cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BUILD_DIR) -j$$(nproc)
