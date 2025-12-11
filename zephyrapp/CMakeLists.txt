# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(display)

add_subdirectory(vendor)
add_subdirectory(game)

target_sources(app PRIVATE
	src/batt.c
	src/ble.c
	src/buttons.c
	src/debugmenu.c
	src/display_pinctrl.c
	src/epaper_driver.c
	src/flash.c
	src/imu.c
	src/lcd_driver.c
	src/lcd_rendering.c
	src/main.c
	src/misc.c
	src/msht.c
	src/power_control.c
	src/rgb_leds.c
	src/rp2350_ipc.c
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

	#src/protobufs/nanopb.pb.c
	src/protobufs/pb_common.c
	src/protobufs/pb_decode.c
	src/protobufs/pb_encode.c

	src/protobufs/meshtastic/
	src/protobufs/meshtastic/admin.pb.c
	src/protobufs/meshtastic/apponly.pb.c
	src/protobufs/meshtastic/atak.pb.c
	src/protobufs/meshtastic/cannedmessages.pb.c
	src/protobufs/meshtastic/channel.pb.c
	src/protobufs/meshtastic/clientonly.pb.c
	src/protobufs/meshtastic/config.pb.c
	src/protobufs/meshtastic/connection_status.pb.c
	#src/protobufs/meshtastic/deviceonly.pb.c
	src/protobufs/meshtastic/device_ui.pb.c
	src/protobufs/meshtastic/interdevice.pb.c
	src/protobufs/meshtastic/localonly.pb.c
	src/protobufs/meshtastic/mesh.pb.c
	src/protobufs/meshtastic/module_config.pb.c
	src/protobufs/meshtastic/mqtt.pb.c
	src/protobufs/meshtastic/paxcount.pb.c
	src/protobufs/meshtastic/portnums.pb.c
	src/protobufs/meshtastic/powermon.pb.c
	src/protobufs/meshtastic/remote_hardware.pb.c
	src/protobufs/meshtastic/rtttl.pb.c
	src/protobufs/meshtastic/storeforward.pb.c
	src/protobufs/meshtastic/telemetry.pb.c
	src/protobufs/meshtastic/xmodem.pb.c

	src/mt_test.c


	src/menu/menu_time.c
	src/menu/menu_system.c
)

target_compile_options(app PRIVATE -fsingle-precision-constant)

target_include_directories(app PRIVATE 
	src/ 
	src/menu/
	src/protobufs/
)

if(MINIMIZE_GAME_FOOTPRINT)
	message(WARNING "Not building in the actual game")
	target_compile_definitions(app PRIVATE MINIMIZE_GAME_FOOTPRINT=1)
endif()

zephyr_linker_sources(SECTIONS persistant.ld)
