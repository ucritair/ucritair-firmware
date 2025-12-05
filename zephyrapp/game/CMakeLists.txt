cmake_minimum_required(VERSION 3.20.0)

project(app)

if(NOT TARGET app)
	add_executable(app)

	message(WARNING "Building for desktop")
	target_compile_definitions(app PRIVATE CAT_DESKTOP=1)
	if(APPLE)
		target_compile_definitions(app PRIVATE CAT_MACOS=1)
	endif()

	find_package(GLEW REQUIRED)
	find_package(glfw3 3.3 REQUIRED)
	find_package(PNG REQUIRED)
	find_package(OpenGL REQUIRED)
	target_link_libraries(app m glfw PNG::PNG GLEW::GLEW)
	if(NOT APPLE)
		target_link_libraries(app OpenGL)
	endif()
	
	set(CMAKE_BUILD_TYPE Debug)
	target_compile_options(app PRIVATE -Wall -g)

	file(
		GLOB exclude_src
		src/core/cat_embedded.c
	)
	target_include_directories(app PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../../src/)
	target_include_directories(app PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../../src/protobufs/)
else()
	message(WARNING "Building for embedded")
	target_compile_definitions(app PRIVATE CAT_EMBEDDED=1)

	option(AQ_FIRST "AQ_FIRST" OFF)
	option(WIFI "WIFI" OFF)
	option(RADIO "RADIO" OFF)

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
		GLOB exclude_src
		src/core/cat_desktop.c
		src/core/cat_persist_archive.c
	)
endif()

function(ASSET_GEN TYPE_NAME GEN_SCRIPT SRC_DATA)
	target_sources(app PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../assets/${TYPE_NAME}_assets.c)
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../assets/${TYPE_NAME}_assets.h ${CMAKE_CURRENT_BINARY_DIR}/../assets/${TYPE_NAME}_assets.c
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../
		COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../${GEN_SCRIPT}
		DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../${GEN_SCRIPT}
		DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../${SRC_DATA}
	)
endfunction()

ASSET_GEN(mesh utils/asset_gen/meshgen.py meshes/meshes.json)
ASSET_GEN(mesh2d utils/asset_gen/mesh2dgen.py meshes/mesh2ds.json)
ASSET_GEN(sprite utils/asset_gen/spritegen.py sprites/sprites.json)
ASSET_GEN(tinysprite utils/asset_gen/tinyspritegen.py sprites/tinysprites.json)
ASSET_GEN(item utils/asset_gen/itemgen.py data/items.json)
ASSET_GEN(theme utils/asset_gen/themegen.py data/themes.json)
ASSET_GEN(fish utils/asset_gen/fishgen.py data/fish.json)
ASSET_GEN(dialogue utils/asset_gen/dialoguegen.py data/dialogue.json)
ASSET_GEN(dialogue_profile utils/asset_gen/dialogueprofilegen.py data/dialogue_profiles.json)
ASSET_GEN(prop utils/asset_gen/propgen.py data/props.json)
ASSET_GEN(scene utils/asset_gen/scenegen.py data/scenes.json)

file(
	GLOB game_src
	src/*.c
	src/**/*.c
)
list(REMOVE_ITEM game_src ${exclude_src})
target_sources(app PRIVATE ${game_src} ${asset_src})

file(
	GLOB game_include
	src
	src/**
)
message("${game_include}")
target_include_directories(app PRIVATE ${game_include})
target_include_directories(app PRIVATE assets)
