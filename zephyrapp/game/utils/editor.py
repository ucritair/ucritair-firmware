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
import glob;
from pathlib import Path;
from PIL import Image;


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


#########################################################
## CONTEXT

config = configparser.ConfigParser();
config.read("utils/editor.ini");
width = int(config["CONFIG"]["width"]);
height = int(config["CONFIG"]["height"]);

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


#########################################################
## JSON DOCUMENT

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

class JsonDocument:
	def __init__(self, path):
		self.path = path;
		self.parent, entry = os.path.split(path);
		self.name, _ = os.path.splitext(entry);
		self.file = open(path, "r+");
		self.data = json.load(self.file);

		self.preview_cache = {};
	
	def schema_type(self, key):
		return self.data["schema"][key]["type"];

	def readable(self, key):
		return "read" in self.data["schema"][key]["permissions"] or "write" in self.data["schema"][key]["permissions"];

	def writable(self, key):
		return "write" in self.data["schema"][key]["permissions"];
	
	def preview(self, path):
		name, ext = os.path.splitext(path);
		if ext == ".png":
			texture, width, height = None, None, None;
			if path not in self.preview_cache:
				image = Image.open(path);
				buffer = image.tobytes();
				width = image.size[0];
				height = image.size[1];
				texture = glGenTextures(1);
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
				self.preview_cache[path] = texture, width, height;
			else:
				texture, width, height = self.preview_cache[path];
			imgui.image(texture, (width, height));
		else:
			imgui.text("[NO PREVIEW AVAILABLE]");
	
	def render_helper(self, node):
		if node is None:
			return;
		if isinstance(node, dict):
			title = build_title(node);
			if(imgui.tree_node(title)):
				for key in node:
					if self.readable(key):
						schema_type = self.schema_type(key);
						if schema_type == "int":
							imgui.text(key);
							imgui.same_line();
							if self.writable(key):
								_, node[key] = imgui.input_int(build_id(node, key), node[key]);
							else:
								imgui.text(str(node[key]));
						elif schema_type == "string":
							imgui.text(key);
							imgui.same_line();
							if self.writable(key):
								_, node[key] = imgui.input_text(build_id(node, key), node[key]);
							else:
								imgui.text(node(key));
						elif schema_type == "bool":
							imgui.text(key);
							imgui.same_line();
							if self.writable(key):
								node[key] = imgui.checkbox(build_id(node, key), node[key]);
							else:
								imgui.text(str(node(key)));
						elif "*" in schema_type:
							self.preview(node[key]);
							imgui.text(key);
							imgui.same_line();
							if self.writable(key):
								_, node[key] = imgui.input_text(build_id(node, key), node[key]);
								imgui.same_line();
								ident = build_id(node, key);
								if imgui.button(f"...{ident}"):
									FileExplorer(ident, document.parent, schema_type);
								if FileExplorer.is_active(ident):
									FileExplorer.render();
									ready, result = FileExplorer.harvest();
									node[key] = str(result) if ready else node[key];
							else:
								imgui.text(node(key));
						elif isinstance(schema_type, list):
							imgui.text(key);
							imgui.same_line();
							if imgui.begin_combo(build_id(node, key), str(node[key])):
								for value in schema_type:
									selected = value == node[key];
									if imgui.selectable(value, selected)[0]:
										node[key] = value;
									if selected:
										imgui.set_item_default_focus();	
								imgui.end_combo();
						else:
							imgui.text(f"UNSUPPORTED TYPE \"{schema_type}\"");
				imgui.tree_pop();
	
	def render(self):
		for node in self.data["entries"]:
			self.render_helper(node);

document = None;


#########################################################
## FILE EXPLORER

class FileExplorer:
	_ = None;
	result = None;

	def __init__(self, target, anchor, glob):
		if FileExplorer._ != None:
			return None;
		FileExplorer._ = self;
		FileExplorer.result = None;

		self.target = target;

		self.anchor = Path(anchor);
		self.current = Path(anchor);
		self.glob = glob;

		self.size = (640, 480);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;
	
	def is_active(target):
		return FileExplorer._ != None and FileExplorer._.target == target;

	def render():
		if FileExplorer._ == None:
			return;

		self = FileExplorer._;
		if self.open:
			listings = [self.current.parent.absolute()];
			for entry in self.current.iterdir():
				hidden = foldl(lambda a, b : a or b, False, [str(p).startswith(".") for p in entry.parts]);
				if entry.absolute().is_dir() and not hidden:
					listings.append(entry);
			for entry in self.current.glob(self.glob):
				if not entry in listings:
					listings.append(entry);
			
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"File Explorer ({self.current.name})", self.open, flags=self.window_flags);
			for item in listings:
				name = ".." if item == self.current.parent.absolute() else item.name;
				if imgui.button(name):
					if item.is_dir():
						self.current = item;
					else:
						FileExplorer.result = self.anchor/item.absolute().relative_to(self.anchor.absolute());
						FileExplorer._ = None;
			self.size = imgui.get_window_size();
			imgui.end();
		if not self.open:
			FileExplorer._ = None;
		
	def harvest():
		ready, result = FileExplorer.result != None, FileExplorer.result;
		FileExplorer.result = None;
		return ready, result;


#########################################################
## EDITOR GUI

asset_dirs = ["sprites", "sounds", "meshes", "data"];

window_flag_list = [
	imgui.WindowFlags_.no_saved_settings,
	imgui.WindowFlags_.no_move,
	imgui.WindowFlags_.no_resize,
	imgui.WindowFlags_.no_nav_inputs,
	imgui.WindowFlags_.no_nav_focus,
	imgui.WindowFlags_.no_collapse,
	imgui.WindowFlags_.no_background,
	imgui.WindowFlags_.no_bring_to_front_on_focus,
];
window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);

while not glfw.window_should_close(handle):
	glfw.poll_events();
	impl.process_inputs();

	glEnable(GL_PROGRAM_POINT_SIZE);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	imgui.new_frame();

	imgui.set_next_window_pos((0, 0));
	imgui.set_next_window_size((width, height));
	imgui.begin("Editor", flags=window_flags);

	if imgui.begin_main_menu_bar():
		if imgui.begin_menu("File"):
			if imgui.begin_menu("Open"):
				for folder in asset_dirs:
					for entry in os.listdir(folder):
						name, ext = os.path.splitext(entry);
						path = os.path.join(folder, entry);
						if ext == ".json":
							if imgui.menu_item(name, "", document != None and name == document.name)[0]:
								document = JsonDocument(path);
				imgui.end_menu();
			if imgui.menu_item("Close", "", False)[0]:
				document = None;
			imgui.end_menu();
		imgui.end_main_menu_bar();
	
	if document != None:
		document.render();

	imgui.end();
	imgui.render();
	impl.render(imgui.get_draw_data());
	imgui.end_frame();

	glfw.swap_buffers(handle);

impl.shutdown();
imgui.destroy_context();

glfw.destroy_window(handle);
glfw.terminate();

