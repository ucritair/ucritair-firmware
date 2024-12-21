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
from playsound3 import playsound;
import subprocess as sp;
from stat import *;


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
						FileExplorer.result = item.absolute().relative_to(self.anchor.absolute());
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
## JSON DOCUMENT

asset_dirs = [Path("sprites"), Path("sounds"), Path("meshes"), Path("data")];
asset_docs = [];
asset_types = [];
preview_cache = {};

def get_name(node):
	if "name" in node:
		return node["name"];
	elif "path" in node:
		return node["path"];
	elif "id" in node:
		return node["id"];
	return "Node";

def get_number(node):
	if "id" in node:
		return node["id"];
	return id(node);

def get_id(node, key):
	return f"##{id(node)}{id(key)}";

def get_default(t):
	if t == "int":
		return 0;
	elif t == "bool":
		return False;
	elif t == "string":
		return "";
	elif t == "float":
		return 0.0;
	elif "*" in t:
		return "";
	elif t in asset_types:
		return "";
	elif isinstance(t, list):
		return t[0];
	else:
		return None;

class AssetSchema:
	def compute_constraints(self, node):
		for key_a in node:
			if "is_constraint" in node[key_a]:
				continue;
			for key_b in node:
				if "constraint" in node[key_b] and node[key_b]["constraint"]["key"] == key_a:
					node[key_a]["is_constraint"] = True;
			if isinstance(node[key_a]["type"], dict):
				self.compute_constraints(node[key_a]["type"]);

	def __init__(self, schema):
		self.root = copy.deepcopy(schema);
		self.path = [self.root];
		self.compute_constraints(self.root);

	def reset(self):
		self.path = [self.root];

	def push(self, key):
		self.path.append(self.path[-1][key]["type"]);
	
	def pop(self):
		self.path.pop();

	def get_type(self, key):
		return self.path[-1][key]["type"];
	
	def is_readable(self, key):
		perms = self.path[-1][key]["permissions"];
		return "read" in perms or "write" in perms;

	def is_writable(self, key):
		perms = self.path[-1][key]["permissions"];
		return "write" in perms;
	
	def is_constraint(self, key):
		node = self.path[-1][key];
		return "is_constraint" in node and node["is_constraint"];
	
	def prototype(self, node):
		parent = self.path[-1];
		for key in parent:
			child = parent[key];
			if "constraint" in child:
				ckey = child["constraint"]["key"];
				cval = child["constraint"]["value"];
				if node[ckey] != cval:
					if key in node:
						del node[key];
						continue;
					else:
						continue;
			if key in node:
				continue;
			if isinstance(child["type"], dict):
				self.push(key);
				node[key] = {};
				self.prototype(node[key]);
				self.pop();
			else:
				node[key] = get_default(child["type"]);

class AssetDocument:
	def __init__(self, path):
		self.path = path;
		self.parent, entry = os.path.split(path);
		self.name, _ = os.path.splitext(entry);
		self.file = open(path, "r+");

		self.data = json.load(self.file);
		self.type = self.data["type"];
		self.schema = AssetSchema(self.data["schema"]);
		self.entries = self.data["entries"];
	
	def preview(self, node):
		try:
			path = next(node[k] for k in node if not self.schema.get_type(k) is None and "*" in self.schema.get_type(k));
			path = os.path.join(self.parent, path);
			if not Path(path).exists():
				imgui.text("[NO PREVIEW AVAILABLE]");
				return;

			name, ext = os.path.splitext(path);
			if ext == ".png":
				texture, width, height = None, None, None;
				frames = node["frames"] if "frames" in node else 1;
				if path not in preview_cache or (path in preview_cache and frames != preview_cache[path][3]):
					image = Image.open(path);
					pixels = image.load();

					all_black = True;
					inversion = [];
					for y in range(image.size[1]):
						for x in range(image.size[0]):
							p = pixels[x, y];
							if p[3] >= 128:
								all_black &= (p[0] == 0 and p[1] == 0 and p[2] == 0);
								inversion.append((255, 255, 255, p[3]));
							else:
								inversion.append(p);
					if all_black:
						image.putdata(inversion);
					
					if frames > 1:
						frames = node["frames"];
						frame_h = image.size[1] // frames;
						transpose = [];
						for y in range(frame_h):
							for f in range(frames):
								for x in range(image.size[0]):
									transpose.append(pixels[x, y + f * frame_h]);
						image = Image.new(image.mode, (image.size[0] * frames, frame_h));
						image.putdata(transpose);

					buffer = image.tobytes();
					width = image.size[0];
					height = image.size[1];
					texture = glGenTextures(1);
					glBindTexture(GL_TEXTURE_2D, texture);
					glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

					preview_cache[path] = texture, width, height, frames;
				else:
					texture, width, height, frames = preview_cache[path];
				imgui.image(texture, (width * 2, height * 2));
			elif ext == ".wav":
				if(imgui.button("PLAY")):
					playsound(path, block=False);
			else:
				imgui.text("[NO PREVIEW AVAILABLE]");

		except StopIteration:
			try:
				k = next(k for k in node if self.schema.get_type(k) in asset_types);
				d = next(d for d in asset_docs if d.type == self.schema.get_type(k));
				a = next(a for a in d.entries if a["name"] == node[k]);
				if node != a:
					d.preview(a);
				else:
					imgui.text("[NO PREVIEW AVAILABLE]");
			except StopIteration:
				imgui.text("[NO PREVIEW AVAILABLE]");
	
	def render_helper(self, node):
		if(imgui.tree_node(f"{get_name(node)} {get_number(node)} ####{str(id(node))}")):
			imgui.separator();
			self.preview(node);

			for key in node:
				key_type = self.schema.get_type(key);
				readable = self.schema.is_readable(key);
				writable = self.schema.is_writable(key);
				if not readable:
					continue;

				if key_type is None:
					imgui.text(key);
				elif key_type == "int":
					imgui.text(key);
					imgui.same_line();
					if writable:
						_, node[key] = imgui.input_int(get_id(node, key), node[key]);
					else:
						imgui.text(str(node[key]));

				elif key_type == "string":
					imgui.text(key);
					imgui.same_line();
					if writable:
						_, node[key] = imgui.input_text(get_id(node, key), node[key]);
					else:
						imgui.text(node[key]);

				elif key_type == "bool":
					imgui.text(key);
					imgui.same_line();
					if writable:
						node[key] = imgui.checkbox(get_id(node, key), node[key])[1];
					else:
						imgui.text(str(node(key)));

				elif "*" in key_type:
					imgui.text(key);
					imgui.same_line();
					if writable:
						_, node[key] = imgui.input_text(get_id(node, key), node[key]);
						imgui.same_line();
						ident = get_id(node, key);
						if imgui.button(f"...{ident}"):
							FileExplorer(ident, document.parent, key_type);
						if FileExplorer.is_active(ident):
							FileExplorer.render();
							ready, result = FileExplorer.harvest();
							node[key] = str(result) if ready else node[key];
					else:
						imgui.text(node[key]);

				elif isinstance(key_type, list):
					imgui.text(key);
					imgui.same_line();
					if imgui.begin_combo(get_id(node, key), str(node[key])):
						for value in key_type:
							selected = value == node[key];
							if imgui.selectable(value, selected)[0]:
								node[key] = value;
							if selected:
								imgui.set_item_default_focus();	
						imgui.end_combo();

				elif key_type in asset_types:
					imgui.text(key);
					imgui.same_line();
					if imgui.begin_combo(get_id(node, key), str(node[key])):
						for doc in asset_docs:
							if doc.type == key_type:
								for asset in doc.entries:
									selected = asset["name"] == node[key];
									if imgui.selectable(asset["name"], selected)[0]:
										node[key] = asset["name"];
									if selected:
										imgui.set_item_default_focus();	
						imgui.end_combo();
					
				elif isinstance(key_type, dict):
					self.schema.push(key);
					self.render_helper(node[key], key);
					self.schema.pop();

				else:
					imgui.text(f"[UNSUPPORTED TYPE \"{key_type}\"]");
			self.schema.prototype(node);
			imgui.tree_pop();
	
	def render(self):
		for entry in self.entries:
			self.render_helper(entry);
	
	def save(self):
		self.file.seek(0);
		self.file.truncate();
		self.file.write(json.dumps(self.data));

document = None;


#########################################################
## EDITOR GUI

for folder in asset_dirs:
	for entry in folder.iterdir():
		name, ext = os.path.splitext(entry);
		if ext == ".json":
			doc = AssetDocument(entry);
			asset_docs.append(doc);
			asset_type = doc.type;
			if not asset_type in asset_types:
				asset_types.append(asset_type);

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
				for doc in asset_docs:
					if imgui.menu_item_simple(str(doc.path), selected = document != None and doc.name == document.name):
						document = doc;
				imgui.end_menu();
			if imgui.menu_item_simple("Save", enabled=document != None):
					document.save();
			if imgui.menu_item_simple("Close", enabled=document != None):
				document = None;
			imgui.end_menu();

		if imgui.begin_menu("Assets", enabled=document != None):
			if imgui.begin_menu("Sort", enabled=document != None):
				if imgui.menu_item_simple("Name"):
					document.entries.sort(key = lambda n: get_name(n));
				if imgui.menu_item_simple("Number"):
					document.entries.sort(key = lambda n: get_number(n));
				imgui.end_menu();
			if imgui.menu_item_simple("New"):
				new_asset = {};
				document.schema.prototype(new_asset);
				new_asset["name"] = f"new_{document.type}";
				if "id" in new_asset:
					new_asset["id"] = len(document.entries);
				document.entries.append(new_asset);
	
		if imgui.begin_menu("Utils"):
			for util in Path("utils").iterdir():
				if not util.is_file() or not (os.stat(util).st_mode & S_IXUSR) > 0:
					continue;
				if imgui.menu_item_simple(str(util)):
					sp.Popen(str(util), shell=True);				
				
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

