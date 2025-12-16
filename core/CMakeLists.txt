# core\

add_library(core
    Detector.cpp
    ${CMAKE_SOURCE_DIR}/src/coin_detector.cpp
    ${CMAKE_SOURCE_DIR}/src/evaluator.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(core PUBLIC
    opencv::opencv
)

