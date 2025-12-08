cmake_minimum_required(VERSION 3.10)
project(CoinDetector VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} include)

add_executable(coin_detector
    src/main.cpp
    src/coin_detector.cpp
    src/evaluator.cpp
)

target_link_libraries(coin_detector ${OpenCV_LIBS})