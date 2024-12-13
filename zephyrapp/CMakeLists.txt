# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(display)

add_subdirectory(vendor)
add_subdirectory(game)

target_sources(app PRIVATE
	src/airquality.c
	src/batt.c
	src/ble.c
	src/buttons.c
	src/debugmenu.c
	src/display_pinctrl.c
	src/epaper_driver.c
	src/epaper_rendering.c
	src/flash.c
	src/imu.c
	src/lcd_driver.c
	src/lcd_rendering.c
	src/main.c
	src/misc.c
	src/power_control.c
	src/rgb_leds.c
	src/rtc.c
	src/sound.c
	src/sbcdec.c
	src/sdcard.c
	src/sensor_hal.c
	src/sensor_lps22hh.c
	src/sensor_sen5x.c
	src/sensor_sunrise.c
	src/sound.c
	src/touch.c
	src/wlan.c

	src/menu/menu_time.c
	src/menu/menu_aqi.c
	src/menu/menu_system.c
	src/menu/menu_graph.c

	${CMAKE_CURRENT_BINARY_DIR}/game/sprites/sprite_assets.c
)

target_include_directories(app PRIVATE src/ src/menu/)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/game/sprites/sprite_assets.c
	COMMAND ${CMAKE_CURRENT_LIST_DIR}/game/utils/pack_sprites.py
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/game/utils/pack_sprites.py
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/game/sprites/sprites.json
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/assets
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/assets/*.png
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/game/sprites
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/game/sprites/*.png
)

if(MINIMIZE_GAME_FOOTPRINT)
	message(WARNING "Not building in the actual game")
	target_compile_definitions(app PRIVATE MINIMIZE_GAME_FOOTPRINT=1)
endif()

zephyr_linker_sources(SECTIONS persistant.ld)