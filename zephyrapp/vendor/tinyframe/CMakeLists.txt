# TinyFrame library CMakeLists.txt

target_sources(app PRIVATE
    TinyFrame.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
