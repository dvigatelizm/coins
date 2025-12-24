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

# --- NanoSVG (vendored header-only) ---
target_link_libraries(label_editor_wx PRIVATE NanoSVG)

# --- wxWidgets ---
find_package(wxWidgets REQUIRED COMPONENTS core base aui html)
include(${wxWidgets_USE_FILE})

target_include_directories(label_editor_wx PRIVATE
    ${wxWidgets_INCLUDE_DIRS}
)
target_link_libraries(label_editor_wx PRIVATE
    ${wxWidgets_LIBRARIES}
)

# --- OpenCV ---
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs highgui)

target_include_directories(label_editor_wx PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(label_editor_wx PRIVATE
    ${OpenCV_LIBS}
)

# --- Link your internal core library ---
target_link_libraries(label_editor_wx PRIVATE core)

set_target_properties(label_editor_wx PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    WIN32_EXECUTABLE TRUE
)

install(TARGETS label_editor_wx
    RUNTIME DESTINATION bin
)