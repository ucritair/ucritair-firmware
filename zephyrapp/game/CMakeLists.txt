cmake_minimum_required(VERSION 3.20.0)

project(app)

if(NOT TARGET app)
	message(WARNING "Building for desktop!")
	add_executable(app)

	target_compile_definitions(app PRIVATE CAT_DESKTOP=1)
	target_compile_definitions(app PRIVATE CAT_DEBUG=1)
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
	target_sources(app PRIVATE src/core/cat_desktop.c)
else()
	message(WARNING "Building for embedded")

	target_compile_definitions(app PRIVATE CAT_EMBEDDED=1)
	target_compile_definitions(app PRIVATE CAT_DEBUG=1)
	
	target_compile_options(app PRIVATE -fsingle-precision-constant)
	target_sources(app PRIVATE src/core/cat_embedded.c)
endif()

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../meshes/mesh_assets.h ${CMAKE_CURRENT_BINARY_DIR}/../meshes/mesh_assets.c
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../meshes/mesh2d_assets.h ${CMAKE_CURRENT_BINARY_DIR}/../meshes/mesh2d_assets.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/meshgen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/meshgen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../meshes/meshes.json
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../sounds/sound_assets.h ${CMAKE_CURRENT_BINARY_DIR}/../sounds/sound_assets.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/soundgen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/soundgen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../sounds/sounds.json
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../sprites/sprite_assets.h ${CMAKE_CURRENT_BINARY_DIR}/../sprites/sprite_assets.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/spritegen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/spritegen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../sprites/sprites.json
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../data/item_assets.h ${CMAKE_CURRENT_BINARY_DIR}/../data/item_assets.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/itemgen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/itemgen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../data/items.json
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../data/theme_assets.h ${CMAKE_CURRENT_BINARY_DIR}/../data/theme_assets.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/themegen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/themegen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../data/themes.json
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../data/fish_assets.h ${CMAKE_CURRENT_BINARY_DIR}/../data/fish_assets.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/fishgen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/fishgen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../data/fish.json
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../data/dialogue_assets.h ${CMAKE_CURRENT_BINARY_DIR}/../data/dialogue_assets.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/dialoguegen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/dialoguegen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../data/dialogue.json
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../data/prop_assets.h ${CMAKE_CURRENT_BINARY_DIR}/../data/prop_assets.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/propgen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/propgen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../data/props.json
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../data/scene_assets.h ${CMAKE_CURRENT_BINARY_DIR}/../data/scene_assets.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/scenegen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/scenegen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../data/scenes.json
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../data/props.json
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../data/dialogue_assets.h ${CMAKE_CURRENT_BINARY_DIR}/../data/dialogue_assets.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/dialoguegen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/dialoguegen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../data/dialogue.json
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../data/dialogue_profile_assets.h ${CMAKE_CURRENT_BINARY_DIR}/../data/dialogue_profile_assets.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/dialogueprofilegen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../utils/asset_gen/dialogueprofilegen.py
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../data/dialogue_profiles.json
)

target_sources(app PRIVATE
	src/core/cat_common.c

	src/render/cat_berrier.c
	src/render/cat_spriter.c
	src/render/cat_draw_queue.c
	src/render/cat_animation.c
	src/render/cat_poly.c
	src/render/cat_effects.c
	src/render/cat_gizmos.c

	src/menu/cat_menu.c
	src/menu/cat_menu_insights.c
	src/menu/cat_menu_manual.c
	src/menu/cat_menu_magic.c
	src/menu/cat_menu_hedron.c
	src/menu/cat_menu_debug.c
	src/menu/cat_menu_colour_picker.c
	src/menu/cat_menu_palette_picker.c

	src/cowtools/cat_structures.c
	src/cowtools/cat_math.c

	src/caring/cat_play.c
	src/caring/cat_study.c
	src/feeding/cat_feed.c

	src/arcade/cat_arcade.c
	src/arcade/cat_arcade_snake.c
	src/arcade/cat_arcade_mines.c
	src/arcade/cat_arcade_foursquares.c

	src/monitors/cat_monitors.c
	src/monitors/cat_monitor_summary.c
	src/monitors/cat_monitor_details.c
	src/monitors/cat_monitor_sparklines.c
	src/monitors/cat_monitor_calendar.c
	src/monitors/cat_monitor_gameplay.c
	src/monitors/cat_monitor_clock.c
	src/monitors/cat_monitor_graph.c
	src/monitors/cat_monitor_ach.c
	src/monitors/cat_monitor_utils.c

	src/inventory/cat_item.c
	src/inventory/cat_shop.c

	src/gui/cat_gui.c
	src/gui/cat_text.c
	src/gui/cat_graph.c

	src/air/cat_aqi.c
	src/air/cat_crisis.c
	src/air/cat_crisis_report.c

	src/systems/cat_input.c
	src/systems/cat_machine.c
	src/systems/cat_time.c

	src/room/cat_deco.c
	src/room/cat_room.c
	src/room/cat_pet.c
	src/room/cat_death_report.c
	src/room/cat_notices.c

	src/world/cat_world.c
	src/world/cat_dialogue.c
	src/world/cat_scene.c

	src/procs/cat_procs.c

	src/crafting/cat_crafting.c

	sounds/sound_assets.c
	meshes/mesh_assets.c
	meshes/mesh2d_assets.c
	sprites/sprite_assets.c
	data/item_assets.c
	data/theme_assets.c
	data/fish_assets.c
	data/dialogue_assets.c
	data/prop_assets.c
	data/scene_assets.c
	data/dialogue_profile_assets.c

	src/cat_save.c
	src/cat_main.c)

target_include_directories(app PRIVATE src/)
target_include_directories(app PRIVATE src/core)
target_include_directories(app PRIVATE src/render)
target_include_directories(app PRIVATE src/menu)
target_include_directories(app PRIVATE src/arcade)
target_include_directories(app PRIVATE src/monitors)
target_include_directories(app PRIVATE src/gui)
target_include_directories(app PRIVATE src/cowtools)
target_include_directories(app PRIVATE src/inventory)
target_include_directories(app PRIVATE src/caring)
target_include_directories(app PRIVATE src/air)
target_include_directories(app PRIVATE src/systems)
target_include_directories(app PRIVATE src/room)
target_include_directories(app PRIVATE src/world)
target_include_directories(app PRIVATE src/procs)
target_include_directories(app PRIVATE src/crafting)
target_include_directories(app PRIVATE src/feeding)

target_include_directories(app PRIVATE sounds/)
target_include_directories(app PRIVATE meshes/)
target_include_directories(app PRIVATE sprites/)
target_include_directories(app PRIVATE data/)
