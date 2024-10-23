cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(cat)

add_subdirectory(vendor)

target_sources(app PRIVATE src/main.c
    src/sen5x.c
    src/sunrise.c
    src/lps22hh.c)
