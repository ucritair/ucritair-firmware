cmake_minimum_required(VERSION 3.20.0)

project(app)

set(ZEPHYRAPP_DIR ${CMAKE_CURRENT_BINARY_DIR}/../..)
set(GAME_DIR ${ZEPHYRAPP_DIR}/game)

if(NOT TARGET app)
	add_executable(app)

	message(WARNING "Building for desktop")
	target_compile_definitions(app PRIVATE CAT_DESKTOP=1)
	if(APPLE)
		target_compile_definitions(app PRIVATE CAT_MACOS=1)
	endif()

	set(RESEARCH_NAME "" CACHE STRING "")

	find_package(GLEW REQUIRED)
	find_package(glfw3 3.3 REQUIRED)
	find_package(PNG REQUIRED)
	find_package(OpenGL REQUIRED)
	target_link_libraries(app m glfw PNG::PNG GLEW::GLEW)
	if(NOT APPLE)
		target_link_libraries(app OpenGL)
	endif()
	
	set(CMAKE_BUILD_TYPE Debug)
	target_compile_options(app PRIVATE -Wall -g -DRESEARCH_NAME="${RESEARCH_NAME}")

	file(
		GLOB EXCLUDE_SRC
		src/core/cat_embedded.c
	)
	target_include_directories(app PRIVATE ${ZEPHYRAPP_DIR}/src/)
	target_include_directories(app PRIVATE ${ZEPHYRAPP_DIR}/src/protobufs/)
else()
	message(WARNING "Building for embedded")
	target_compile_definitions(app PRIVATE CAT_EMBEDDED=1)

	option(AQ_FIRST "AQ_FIRST" OFF)
	option(WIFI "WIFI" OFF)
	option(RADIO "RADIO" OFF)
	option(RADIO "RADIO" OFF)
	set(RESEARCH_NAME "" CACHE STRING "P")

	if(AQ_FIRST)
		target_compile_definitions(app PRIVATE CAT_PRIORITIZE_AQ=1)
		message(WARNING "Prioritizing AQ")
	endif()
	if(WIFI)
		target_compile_definitions(app PRIVATE CAT_WIFI_ENABLED=1)
		message(WARNING "Enabling WiFi")
	endif()
	if(RADIO)
		target_compile_definitions(app PRIVATE CAT_RADIO_ENABLED=1)
		message(WARNING "Enabling radio")
	endif()
	
	file(
		GLOB EXCLUDE_SRC
		src/core/cat_desktop.c
		src/core/cat_persist_archive.c
	)
endif()

function(asset_gen TYPE_NAME GEN_SCRIPT SRC_DATA)
	target_sources(app PRIVATE ${GAME_DIR}/assets/${TYPE_NAME}_assets.c)
	add_custom_command(
		OUTPUT ${GAME_DIR}/assets/${TYPE_NAME}_assets.h ${GAME_DIR}/assets/${TYPE_NAME}_assets.c
		WORKING_DIRECTORY ${GAME_DIR}/
		COMMAND ${GAME_DIR}/${GEN_SCRIPT}
		DEPENDS ${GAME_DIR}/${GEN_SCRIPT}
		DEPENDS ${GAME_DIR}/${SRC_DATA}
	)
endfunction()

asset_gen(mesh generators/meshgen.py meshes/meshes.json)
asset_gen(mesh2d generators/mesh2dgen.py meshes/mesh2ds.json)
asset_gen(sprite generators/spritegen.py sprites/sprites.json)
asset_gen(tinysprite generators/tinyspritegen.py sprites/tinysprites.json)
asset_gen(item generators/itemgen.py data/items.json)
asset_gen(theme generators/themegen.py data/themes.json)
asset_gen(fish generators/fishgen.py data/fish.json)
asset_gen(dialogue generators/dialoguegen.py data/dialogue.json)
asset_gen(dialogue_profile generators/dialogueprofilegen.py data/dialogue_profiles.json)
asset_gen(prop generators/propgen.py data/props.json)
asset_gen(scene generators/scenegen.py data/scenes.json)

file(
	GLOB GAME_SRC
	src/*.c
	src/**/*.c
)
list(REMOVE_ITEM GAME_SRC ${EXCLUDE_SRC})
target_sources(app PRIVATE ${GAME_SRC})

file(
	GLOB GAME_INCLUDE
	src
	src/**
)
target_include_directories(app PRIVATE ${GAME_INCLUDE})
target_include_directories(app PRIVATE assets)
