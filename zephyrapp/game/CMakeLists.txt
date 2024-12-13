cmake_minimum_required(VERSION 3.20.0)

project(app)

if(NOT TARGET app)
	message(WARNING "Building for desktop!")
	add_executable(app)

	target_compile_definitions(app PRIVATE CAT_DESKTOP=1)
	target_compile_definitions(app PRIVATE CAT_DEBUG=1)

	find_package(GLEW REQUIRED)
	find_package(glfw3 3.3 REQUIRED)
	find_package(PNG REQUIRED)
	find_package(OpenGL REQUIRED)
	target_link_libraries(app m glfw PNG::PNG GLEW::GLEW)
	if(NOT APPLE)
		target_link_libraries(app OpenGL)
	endif()
	
	target_compile_options(app PRIVATE -Wall)
	target_sources(app PRIVATE src/cat_desktop.c)
else()
	message(WARNING "Building for embedded")

	target_compile_definitions(app PRIVATE CAT_EMBEDDED=1)
	target_compile_definitions(app PRIVATE CAT_BAKED_ASSETS=1)
	
	target_sources(app PRIVATE src/cat_embedded.c)
endif()

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_LIST_DIR}/meshes/mesh_assets.h ${CMAKE_CURRENT_LIST_DIR}/meshes/mesh_assets.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	COMMAND ${CMAKE_CURRENT_LIST_DIR}/utils/meshgen.py ${CMAKE_CURRENT_LIST_DIR}/meshes/
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/utils/meshgen.py
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/meshes/meshes.json
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/meshes/*.obj
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_LIST_DIR}/sounds/sound_assets.h ${CMAKE_CURRENT_LIST_DIR}/sounds/sound_assets.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	COMMAND ${CMAKE_CURRENT_LIST_DIR}/utils/soundgen.py ${CMAKE_CURRENT_LIST_DIR}/sounds/
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/utils/soundgen.py
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/sounds/sounds.json
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/sounds/*.wav
)

target_sources(app PRIVATE
	src/cat_aqi.c
	src/cat_gui.c
	src/cat_input.c
	src/cat_item.c
	src/cat_machine.c
	src/cat_sprite.c
	src/cat_math.c
	src/cat_arcade.c
	src/cat_bag.c
	src/cat_deco.c
	src/cat_manual.c
	src/cat_menu.c
	src/cat_room.c
	src/cat_stats.c
	src/cat_vending.c
	src/cat_actions.c
	src/cat_pet.c
	src/cat_item_dialog.c
	src/cat_snake.c
	src/cat_mines.c
	${CMAKE_CURRENT_LIST_DIR}/sounds/sound_assets.c
	${CMAKE_CURRENT_LIST_DIR}/meshes/mesh_assets.c
	${CMAKE_CURRENT_LIST_DIR}/sprites/sprite_assets.c
	src/cat_main.c)

target_include_directories(app PRIVATE src/)
