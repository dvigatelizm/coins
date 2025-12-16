# tools\detect_cli\

add_executable(coin_detect_cli
    main.cpp
)

target_link_libraries(coin_detect_cli PRIVATE
    core
    opencv::opencv
)

