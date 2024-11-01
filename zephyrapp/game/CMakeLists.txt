cmake_minimum_required(VERSION 3.20.0)

project(app)

if(NOT TARGET app)
	message(WARNING "Building for desktop!")
	add_executable(app)
	find_package(GLEW REQUIRED)
	find_package(glfw3 3.3 REQUIRED)
	find_package(OpenAL REQUIRED)
	find_package(PNG REQUIRED)
	target_link_libraries(app m glfw PNG::PNG GLEW::GLEW OpenAL::OpenAL OpenGL)
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
