#корневой

cmake_minimum_required(VERSION 3.21)

project(coins LANGUAGES CXX)

add_subdirectory(tools)

find_package(OpenCV CONFIG REQUIRED)

add_library(core
    core/Detector.cpp
    src/coin_detector.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core PUBLIC
    opencv_core
    opencv_imgproc
)

