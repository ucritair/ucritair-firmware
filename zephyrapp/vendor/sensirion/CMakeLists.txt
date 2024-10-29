target_include_directories(app PRIVATE .)

add_subdirectory(sen5x)

target_sources(app PRIVATE
    sensirion_common.c
    sensirion_i2c_hal.c
    sensirion_i2c.c
    )
