from imgui_bundle import imgui;
import glfw;

class InputManager:
	_glfw_handle = None;
	_imgui_impl = None;

	_glfw_io = {};
	_imgui_io = None;
	

	def initialize(glfw_handle, imgui_impl):
		InputManager._glfw_handle = glfw_handle;
		InputManager._imgui_impl = imgui_impl;
		InputManager._imgui_io = imgui.get_io();
		InputManager._glfw_io = {};

	def update():
		for key in range(glfw.KEY_SPACE, glfw.KEY_LAST+1):
			if not key in InputManager._glfw_io:
				InputManager._glfw_io[key] = [glfw.get_key(InputManager._glfw_handle, key), False];
			else:
				InputManager._glfw_io[key] = [glfw.get_key(InputManager._glfw_handle, key), InputManager._glfw_io[key][0]];
		for key in range(glfw.MOUSE_BUTTON_1, glfw.MOUSE_BUTTON_LAST+1):
			if not key in InputManager._glfw_io:
				InputManager._glfw_io[key] = [glfw.get_mouse_button(InputManager._glfw_handle, key), False];
			else:
				InputManager._glfw_io[key] = [glfw.get_mouse_button(InputManager._glfw_handle, key), InputManager._glfw_io[key][0]];

	def get_imgui_cursor():
		return InputManager._imgui_io.mouse_pos;

	def is_held(key):
		return InputManager._glfw_io[key][0];

	def is_pressed(key):
		return InputManager._glfw_io[key][0] and not InputManager._glfw_io[key][1];