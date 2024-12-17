#!/usr/bin/env python3

import csv;
import configparser;
import os;
import OpenGL;
OpenGL.FULL_LOGGING = True;
from OpenGL.GL import *;
import glfw;
from imgui_bundle import imgui;
import json;
from enum import Enum;
import ctypes;
from imgui_bundle.python_backends.glfw_backend import GlfwRenderer;
import copy;

#########################################################
## THE COW TOOLS

def foldl(f, acc, xs):
	if len(xs) == 0:
		return acc;
	else:
		h, t = xs[0], xs[1:];
		return foldl(f, f(acc, h), t);

def foldr(f, acc, xs):
	if len(xs) == 0:
		return acc;
	else:
		h, t = xs[0], xs[1:];
		return f(head, foldr(f, acc, t));

config = configparser.ConfigParser();
config.read("utils/editor.ini");
width = int(config["CONFIG"]["width"]);
height = int(config["CONFIG"]["height"]);

asset_dirs = ["sprites", "sounds", "meshes", "data"];

class JsonFile:
	def __init__(self, path):
		self.path = path;
		self.direc, entry = os.path.split(path);
		self.name, ext = os.path.splitext(entry);
		self.file = open(path, "r+");
		self.data = json.load(self.file);

def build_title(node):
	name = "Node";
	if "name" in node:
		name = node["name"];
	elif "path" in node:
		name = node["path"];
	number = id(node);
	if "id" in node:
		number = node["id"];
	return f"{name} [{number}]";

def build_id(node, key):
	return f"##{id(node)}{id(key)}";

def render_json(node):
	if node is None:
		return;
	elif isinstance(node, list):
		for entry in node:
			render_json(entry);
	elif isinstance(node, dict):
		title = build_title(node);
		if(imgui.tree_node(title)):
			for key in node.keys():
				imgui.text(f"{key} : ");
				if isinstance(node[key], str):
					imgui.same_line();
					node[key] = imgui.input_text(build_id(node, key), node[key])[1];
				elif isinstance(node[key], bool):
					imgui.same_line();
					node[key] = imgui.checkbox(build_id(node, key), node[key]);
				elif isinstance(node[key], int):
					imgui.same_line();
					node[key] = imgui.input_int(build_id(node, key), node[key])[1];
				else:
					render_json(node[key]);
			imgui.tree_pop();
	else:
		imgui.text("UNSUPPORTED TYPE");

#########################################################
## THE EDITOR

def glfw_error_callback(error, description):
 	print(f"[GLFW ERROR] {error}: {description}");
glfw.set_error_callback(glfw_error_callback);
if not glfw.init():
	print("Failed to initialize GLFW. Exiting");
	exit();
glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3);
glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3);
glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE);
glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, GL_TRUE);
handle = glfw.create_window(width, height, "Editor", None, None);
if not handle:
	print("Failed to create window. Exiting");
	glfw.terminate();
	exit();
glfw.make_context_current(handle);
print("Renderer:", glGetString(GL_RENDERER).decode("utf-8"));
print("GL Version:", glGetString(GL_VERSION).decode("utf-8"));
print("SL Version:", glGetString(GL_SHADING_LANGUAGE_VERSION).decode("utf-8"));

imgui.create_context();
imgui_io = imgui.get_io();
imgui.style_colors_dark()

impl = GlfwRenderer(handle);
imgui_flag_list = [
	imgui.WindowFlags_.no_saved_settings,
	imgui.WindowFlags_.no_move,
	imgui.WindowFlags_.no_resize,
	imgui.WindowFlags_.no_nav_inputs,
	imgui.WindowFlags_.no_nav_focus,
	imgui.WindowFlags_.no_collapse,
	imgui.WindowFlags_.no_background,
	imgui.WindowFlags_.no_bring_to_front_on_focus,
];
imgui_flags = foldl(lambda a, b : a | b, 0, imgui_flag_list);

working_file = None;

while not glfw.window_should_close(handle):
	glfw.poll_events();
	impl.process_inputs();

	glEnable(GL_PROGRAM_POINT_SIZE);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	imgui.new_frame();

	imgui.set_next_window_pos((0, 0));
	imgui.set_next_window_size((width, height));
	imgui.begin("Editor", flags=imgui_flags);

	if imgui.begin_main_menu_bar():
		if imgui.begin_menu("File"):
			if imgui.begin_menu("Open"):
				for folder in asset_dirs:
					for entry in os.listdir(folder):
						name, ext = os.path.splitext(entry);
						path = os.path.join(folder, entry);
						if ext == ".json":
							if imgui.menu_item(name, "", working_file != None and name == working_file.name)[0]:
								working_file = JsonFile(path);
				imgui.end_menu();
			if imgui.menu_item("Close", "", False)[0]:
				working_file = None;
			imgui.end_menu();
		imgui.end_main_menu_bar();
	
	if working_file != None:
		render_json(working_file.data);
		if imgui.button("Add New"):
			last = working_file.data[-1];
			new = copy.deepcopy(last);
			new["id"] = last["id"] + 1;
			working_file.data.append(new);

	imgui.end();
	imgui.render();
	impl.render(imgui.get_draw_data());
	imgui.end_frame();

	glfw.swap_buffers(handle);

impl.shutdown();
imgui.destroy_context();

glfw.destroy_window(handle);
glfw.terminate();

