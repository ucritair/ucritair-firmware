cmake_minimum_required(VERSION 3.20.0)

project(app)

if(NOT TARGET app)
	message(WARNING "Building for desktop!")
	add_executable(app)
	target_link_libraries(app m glfw png GLEW OpenGL openal)
	target_compile_definitions(app PRIVATE CAT_DESKTOP=1)
	target_sources(app PRIVATE src/cat_desktop.c)
else()
	message(WARNING "Building for embedded")
	target_compile_definitions(app PRIVATE CAT_EMBEDDED=1)
	target_sources(app PRIVATE src/cat_embedded.c)
endif()

target_sources(app PRIVATE
	src/cat_gui.c
	src/cat_input.c
	src/cat_item.c
	src/cat_machine.c
	src/cat_main.c
	src/cat_math.c
	src/cat_sprite.c)

target_include_directories(app PRIVATE
	src/)