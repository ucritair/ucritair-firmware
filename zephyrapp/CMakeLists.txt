# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(display)

target_sources(app PRIVATE
	src/batt.c
	src/ble.c
	src/buttons.c
	src/display_pinctrl.c
	src/epaper_driver.c
	src/epaper_rendering.c
	src/flash.c
	src/lcd_driver.c
	src/lcd_rendering.c
	src/main.c
	src/misc.c
	src/power_control.c
	src/rgb_leds.c
	src/sdcard.c
	src/sound.c
	src/wlan.c
)
