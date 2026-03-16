cmake_minimum_required(VERSION 3.16)

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
)

set(FRONTEND_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(FRONTEND_INCLUDE_DIR ${FRONTEND_ROOT}/include)
set(FRONTEND_SRC_DIR ${FRONTEND_ROOT}/src)
set(FRONTEND_RESOURCE_DIR ${FRONTEND_ROOT}/resource)

# Removed redundant definitions

# Fetch all Headers
file(GLOB_RECURSE FRONTEND_HEADERS
    ${FRONTEND_INCLUDE_DIR}/*.hpp
    ${FRONTEND_INCLUDE_DIR}/*.h
)

# Fetch all Sources
file(GLOB_RECURSE FRONTEND_SOURCES
    ${FRONTEND_SRC_DIR}/*.cpp
)

# Fetch Resources
set(FRONTEND_RESOURCES
    ${FRONTEND_RESOURCE_DIR}/qml.qrc
)

add_executable(frontend_app
    ${FRONTEND_HEADERS}
    ${FRONTEND_SOURCES}
    ${FRONTEND_RESOURCES}
)

target_include_directories(frontend_app PRIVATE
    ${FRONTEND_INCLUDE_DIR}
)

target_link_libraries(frontend_app PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
)

# Organize in IDE
source_group(TREE ${FRONTEND_INCLUDE_DIR} PREFIX "include" FILES ${FRONTEND_HEADERS})
source_group(TREE ${FRONTEND_SRC_DIR} PREFIX "src" FILES ${FRONTEND_SOURCES})
source_group(TREE ${FRONTEND_RESOURCE_DIR} PREFIX "resource" FILES ${FRONTEND_RESOURCES})
