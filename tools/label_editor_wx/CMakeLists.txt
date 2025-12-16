# tools\label_editor_wx\

cmake_minimum_required(VERSION 3.21)

project(label_editor_wx LANGUAGES CXX)

add_executable(label_editor_wx
    main.cpp
    MainFrame.cpp
    Canvas.cpp
    LabelIO.cpp
    ../../src/evaluator.cpp
)

target_include_directories(label_editor_wx PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(NanoSVG CONFIG REQUIRED)
find_package(wxWidgets CONFIG REQUIRED)

find_package(OpenCV CONFIG REQUIRED)

target_link_libraries(label_editor_wx PRIVATE
    wx::core
    wx::base
    wx::aui
    wx::html
    opencv_core
    opencv_imgcodecs
    opencv_highgui
)

set_target_properties(label_editor_wx PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    WIN32_EXECUTABLE TRUE
)

install(TARGETS label_editor_wx
    RUNTIME DESTINATION bin
)

target_link_libraries(label_editor_wx PRIVATE core)