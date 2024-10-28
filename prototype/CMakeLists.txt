cmake_minimum_required(VERSION 3.20.0)

project(cat_prototype)

add_executable(cat_prototype
	cat_core.c
	cat_gui.c
	cat_input.c
	cat_item.c
	cat_main.c
	cat_math.c
	cat_sprite.c)

target_link_libraries(cat_prototype m glfw png GLEW OpenGL)
