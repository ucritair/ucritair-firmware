import OpenGL;
OpenGL.FULL_LOGGING = True;
from OpenGL.GL import *;

import glfw;

from imgui_bundle import imgui;
from imgui_bundle.python_backends.glfw_backend import GlfwRenderer;

glfw_handle = None;
imgui_impl = None;
imgui_io = None;
time = None;
delta_time = None;

def glfw_error_callback(error, description):
 		print(f"[GLFW ERROR] {error}: {description}");

def get_clipboard_text(_ctx: imgui.internal.Context) -> str:
	s = glfw.get_clipboard_string(glfw_handle);
	return s.decode();
def set_clipboard_text(_ctx: imgui.internal.Context, text: str) -> str:
	glfw.set_clipboard_string(glfw_handle, text);

def create_context(width, height):
	global glfw_handle;
	global imgui_impl;
	global imgui_io;
	global time;
	global delta_time;

	glfw.set_error_callback(glfw_error_callback);
	if not glfw.init():
		print("Failed to initialize GLFW. Exiting");
		exit();
	glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3);
	glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3);
	glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE);
	glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfw_handle = glfw.create_window(width, height, "Entropic Editor", None, None);
	if not glfw_handle:
		print("Failed to create window. Exiting");
		glfw.terminate();
		exit();
	glfw.make_context_current(glfw_handle);

	print("Renderer:", glGetString(GL_RENDERER).decode("utf-8"));
	print("GL Version:", glGetString(GL_VERSION).decode("utf-8"));
	print("SL Version:", glGetString(GL_SHADING_LANGUAGE_VERSION).decode("utf-8"));

	imgui.create_context();
	imgui_io = imgui.get_io();
	imgui_io.config_windows_move_from_title_bar_only = True;
	imgui.style_colors_dark()
	imgui_impl = GlfwRenderer(glfw_handle);

	platform_io = imgui.get_platform_io();
	platform_io.platform_get_clipboard_text_fn = get_clipboard_text;
	platform_io.platform_set_clipboard_text_fn = set_clipboard_text;

	time = glfw.get_time();
	delta_time = 0;

def destroy_context():
	global glfw_handle;
	global imgui_impl;

	imgui_impl.shutdown();
	imgui.destroy_context();

	glfw.destroy_window(glfw_handle);
	glfw.terminate();

def alive():
	return not glfw.window_should_close(glfw_handle);

def begin_frame():
	global glfw_handle;
	global imgui_impl;
	global time;
	global delta_time;

	time_last = time;
	time = glfw.get_time();
	delta_time = time - time_last;

	glfw.poll_events();
	imgui_impl.process_inputs();

	glEnable(GL_PROGRAM_POINT_SIZE);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	imgui.new_frame();

def end_frame():
	global glfw_handle;
	global imgui_impl;

	imgui.render();
	imgui_impl.render(imgui.get_draw_data());
	imgui.end_frame();
	glfw.swap_buffers(glfw_handle);
		
		


