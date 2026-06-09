cmake_minimum_required(VERSION 3.22.1)
project(minemc CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Gather all source files
set(MINEMC_SOURCES
    # Platform / JNI bridge
    platform/AndroidBridge.cpp

    # Core
    core/GameEngine.cpp
    core/GameLoop.cpp

    # World
    world/Chunk.cpp
    world/World.cpp
    world/WorldGen.cpp
    world/WorldSerializer.cpp

    # Rendering
    render/Renderer.cpp
    render/Mesh.cpp

    # Input
    input/InputHandler.cpp
)

add_library(minemc SHARED ${MINEMC_SOURCES})

target_include_directories(minemc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Android system libraries
find_library(log-lib log)
find_library(android-lib android)
find_library(gles3-lib GLESv3)
find_library(egl-lib EGL)

target_link_libraries(minemc
    ${log-lib}
    ${android-lib}
    ${gles3-lib}
    ${egl-lib}
)

target_compile_options(minemc PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Debug>:-O0 -g>
)
