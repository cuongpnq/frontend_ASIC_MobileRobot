cmake_minimum_required(VERSION 3.16)

enable_language(C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    Concurrent
    DBus
)

find_package(Qt5VirtualKeyboard QUIET)
if (Qt5VirtualKeyboard_FOUND)
    add_compile_definitions(HAS_VIRTUAL_KEYBOARD)
endif()

set(FRONTEND_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(FRONTEND_INCLUDE_DIR ${FRONTEND_ROOT}/include)
set(FRONTEND_SRC_DIR ${FRONTEND_ROOT}/src)
set(FRONTEND_RESOURCE_DIR ${FRONTEND_ROOT}/resource)

# Yakindu State Machine Directories
set(YAKINDU_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../state_machine/src)
set(YAKINDU_GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../state_machine/src-gen)

# Fetch all Headers
file(GLOB_RECURSE FRONTEND_HEADERS
    ${FRONTEND_INCLUDE_DIR}/*.hpp
    ${YAKINDU_SRC_DIR}/*.h
    ${YAKINDU_GEN_DIR}/*.h
)

# Fetch all C++ Sources
file(GLOB_RECURSE FRONTEND_SOURCES
    ${FRONTEND_SRC_DIR}/*.cpp
)

# Fetch Yakindu generated C sources separately
file(GLOB_RECURSE YAKINDU_C_SOURCES
    ${YAKINDU_GEN_DIR}/*.c
)

# Fetch Resources
set(FRONTEND_RESOURCES
    ${FRONTEND_RESOURCE_DIR}/qml.qrc
)

add_executable(frontend_app
    ${FRONTEND_HEADERS}
    ${FRONTEND_SOURCES}
    ${YAKINDU_C_SOURCES}
    ${FRONTEND_RESOURCES}
)

target_include_directories(frontend_app PRIVATE
    ${FRONTEND_INCLUDE_DIR}
    ${YAKINDU_SRC_DIR}
    ${YAKINDU_GEN_DIR}
)

target_link_libraries(frontend_app PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Concurrent
    Qt5::DBus
)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/llama.cpp")
    # --- llama.cpp Integration ---
    # Support for Jetson Xavier / NVIDIA GPU
    list(APPEND CMAKE_PREFIX_PATH "/usr/local/cuda")
    set(CUDAToolkit_ROOT "/usr/local/cuda")
    find_package(CUDAToolkit QUIET)

    if (CUDAToolkit_FOUND)
        message(STATUS "CUDA found: ${CUDAToolkit_BIN_DIR}")
        set(GGML_CUDA ON CACHE BOOL "Enable CUDA for GPU optimization" FORCE)
        set(LLAMA_CUDA ON CACHE BOOL "Enable CUDA for GPU optimization" FORCE)
        # Optimize for Xavier if on ARM (AArch64)
        if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm")
            set(GGML_CUDA_FORCE_MMV ON CACHE BOOL "Force MMV kernels for Jetson" FORCE)
            set(CMAKE_CUDA_ARCHITECTURES 72 CACHE STRING "Target Xavier architecture" FORCE)
        endif()
    else()
        message(WARNING "CUDA Toolkit not found. Building for CPU only.")
        set(GGML_CUDA OFF CACHE BOOL "Enable CUDA for GPU optimization" FORCE)
        set(LLAMA_CUDA OFF CACHE BOOL "Enable CUDA for GPU optimization" FORCE)
    endif()
    
    set(LLAMA_BUILD_COMMON ON CACHE BOOL "Build llama.cpp common" FORCE)
    set(LLAMA_BUILD_TOOLS OFF CACHE BOOL "Disable llama.cpp tools to save time" FORCE)
    set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "Disable llama.cpp examples" FORCE)
    set(LLAMA_BUILD_TESTS OFF CACHE BOOL "Disable llama.cpp tests" FORCE)
    set(LLAMA_BUILD_SERVER ON CACHE BOOL "Enable llama.cpp server" FORCE)
    set(LLAMA_BUILD_WEBUI OFF CACHE BOOL "Disable server web UI" FORCE)
    
    # Define variable needed by mtmd and other tools when added manually
    set(LLAMA_INSTALL_VERSION "0.0.0")
    
    # Save current flags and mute warnings for 3rdparty
    set(OLD_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    set(OLD_CUDA_FLAGS ${CMAKE_CUDA_FLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -w")
    
    add_subdirectory(3rdparty/llama.cpp)
    
    # Explicitly add necessary subdirectories to match expected binary paths
    # Using manual binary directories fixes the path for start_robot_system.sh
    add_subdirectory(3rdparty/llama.cpp/tools/mtmd build-output/llama-mtmd)
    add_subdirectory(3rdparty/llama.cpp/tools/server build-output/llama-server)
    
    # Restore flags for the project
    set(CMAKE_CXX_FLAGS ${OLD_CXX_FLAGS})
    set(CMAKE_CUDA_FLAGS ${OLD_CUDA_FLAGS})
    
    # Ensure llama-server is built as it's needed by the start script
    if (TARGET llama-server)
        add_dependencies(frontend_app llama-server)
    endif()
    
    target_link_libraries(frontend_app PRIVATE llama ggml)
    
    if (GGML_CUDA)
        target_compile_options(frontend_app PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-w>)
    endif()
    message(STATUS "llama.cpp integrated (GGML_CUDA: ${GGML_CUDA}, SERVER: ON)")
else()
    message(WARNING "llama.cpp not found in 3rdparty directory. Please run setup_llama.sh")
endif()
# -----------------------------

if (Qt5VirtualKeyboard_FOUND)
    target_link_libraries(frontend_app PRIVATE Qt5::VirtualKeyboard)
endif()
