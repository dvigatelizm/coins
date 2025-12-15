cmake_minimum_required(VERSION 3.16)
project(label_editor_wx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

add_executable(label_editor
    main.cpp
    MainFrame.cpp
    Canvas.cpp
    LabelIO.cpp
)

target_include_directories(label_editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(label_editor PRIVATE ${wxWidgets_LIBRARIES})