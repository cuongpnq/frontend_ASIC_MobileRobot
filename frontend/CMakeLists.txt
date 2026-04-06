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
    Qt5::DBus
)

if (Qt5VirtualKeyboard_FOUND)
    target_link_libraries(frontend_app PRIVATE Qt5::VirtualKeyboard)
endif()
