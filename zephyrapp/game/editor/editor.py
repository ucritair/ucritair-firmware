#!/usr/bin/env python3

import csv;
import configparser;
import os;
import OpenGL;
OpenGL.FULL_LOGGING = True;
from OpenGL.GL import *;
import glfw;
from imgui_bundle import imgui, implot;
import json;
from enum import Enum, Flag, auto;
import ctypes;
from imgui_bundle.python_backends.glfw_backend import GlfwRenderer;
import copy;
import glob;
from pathlib import Path;
from PIL import Image;
from playsound3 import playsound;
import subprocess as sp;
from stat import *;
import wave;
import numpy as np;


#########################################################
## CONTEXT

config = configparser.ConfigParser();
config.read("editor/editor.ini");
window_width = int(config["CONFIG"]["width"]);
window_height = int(config["CONFIG"]["height"]);

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
handle = glfw.create_window(window_width, window_height, "Editor", None, None);
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
## COW TOOLS

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
## DOCUMENT MODEL

class AssetSchema:
	def __init__(self, schema):
		self.root = copy.deepcopy(schema);
		self.path = [self.root];

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

	def __default(t):
		if t == "int":
			return 0;
		elif t == "bool":
			return False;
		elif t == "string":
			return "";
		elif t == "float":
			return 0.0;
		elif t == "ivec2":
			return [0, 0];
		elif "*" in t:
			return "";
		elif t in asset_types:
			return "";
		elif isinstance(t, list):
			return t[0];
		else:
			return None;
	
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
				node[key] = AssetSchema.__default(child["type"]);

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
	
	def refresh(self):
		for entry in self.entries:
			self.schema.prototype(entry);
	
	def save(self):
		self.file.seek(0);
		self.file.truncate();
		self.file.write(json.dumps(self.data, indent=4));


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


asset_dirs = [Path("sprites"), Path("sounds"), Path("meshes"), Path("data")];
asset_docs = [];
asset_types = [];

for folder in asset_dirs:
	for entry in folder.iterdir():
		name, ext = os.path.splitext(entry);
		if ext == ".json":
			doc = AssetDocument(entry);
			asset_docs.append(doc);
			asset_type = doc.type;
			if not asset_type in asset_types:
				asset_types.append(asset_type);

document = None;


#########################################################
## RENDERING

def load_texture(path):
	image = Image.open(path);
	buffer = image.tobytes();
	width = image.size[0];
	height = image.size[1];
	texture = glGenTextures(1);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	return texture, width, height;

class DrawFlags(Flag):
	CENTER_X = auto()
	CENTER_Y = auto()
	BOTTOM = auto()

class Canvas:
	def __init__(self, width, height):
		self.width = width;
		self.height = height;
		self.buffer = bytearray(bytes(width * height * 3));
		self.texture = glGenTextures(1);
		glBindTexture(GL_TEXTURE_2D, self.texture);
		glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, self.buffer);

		self.draw_flags = ();

	def clear(self, c):
		for y in range(self.height):
			for x in range(self.width):
				i = (y * self.width + x) * 3;
				self.buffer[i+0] = c[0];
				self.buffer[i+1] = c[1];
				self.buffer[i+2] = c[2];

	def draw_pixel(self, x, y, c):
		x = int(x);
		y = int(y);

		if x < 0 or x >= self.width:
			return;
		if y < 0 or y >= self.height:
			return;
		i = (y * self.width + x) * 3;
		self.buffer[i+0] = c[0];
		self.buffer[i+1] = c[1];
		self.buffer[i+2] = c[2];

	def draw_rect(self, x, y, w, h, c):
		x = int(x);
		y = int(y);
		w = int(w);
		h = int(h);

		if DrawFlags.CENTER_X in self.draw_flags:
			x -= image.size[0] // 2;
		if DrawFlags.CENTER_Y in self.draw_flags:
			y -= frame_h // 2;
		elif DrawFlags.BOTTOM in self.draw_flags:
			y -= frame_h;

		for dy in range(y, y+h):
			self.draw_pixel(x, dy, c);
			self.draw_pixel(x+w-1, dy, c);
		for dx in range(x, x+w):
			self.draw_pixel(dx, y, c);
			self.draw_pixel(dx, y+h-1, c);
	
	def draw_sprite(self, x, y, sprite, frame):
		x = int(x);
		y = int(y);

		path = os.path.join("sprites", sprite["path"]);
		image = Image.open(path);
		pixels = image.load();

		frame_h = image.size[1] // sprite["frames"];
		frame_y = frame_h * frame;

		if DrawFlags.CENTER_X in self.draw_flags:
			x -= image.size[0] // 2;
		if DrawFlags.CENTER_Y in self.draw_flags:
			y -= frame_h // 2;
		elif DrawFlags.BOTTOM in self.draw_flags:
			y -= frame_h;

		for yr in range(frame_y, frame_y + frame_h):
			yw = y+yr;
			if yw < 0 or yw >= self.height:
				continue;
			for xr in range(0, image.size[0]):
				xw = x+xr;
				if xw < 0 or xw >= self.width:
					continue;
				c = pixels[xr, yr];
				if c[3] < 128:
					continue;
				i = (yw * self.width + xw) * 3;
				self.buffer[i+0] = c[0];
				self.buffer[i+1] = c[1];
				self.buffer[i+2] = c[2];
	
	def render(self, scale):
		glBindTexture(GL_TEXTURE_2D, self.texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, self.width, self.height, GL_RGB, GL_UNSIGNED_BYTE, self.buffer);
		imgui.image(self.texture, (self.width * scale, self.height * scale));


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
				if imgui.menu_item_simple(name):
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
## ASSET PREVIEWER

class Preview:
	def __init__(self):
		self.cache = {};

	def __invert_all_black(image):
		pixels = image.load();
		all_black = True;
		inversion = [];
		for y in range(image.size[1]):
			for x in range(image.size[0]):
				p = pixels[x, y];
				if p[3] >= 128:
					all_black &= (p[0] == 0 and p[1] == 0 and p[2] == 0);
					if not all_black:
						return image;
					inversion.append((255, 255, 255, p[3]));
				else:
					inversion.append(p);
		if all_black:
			image.putdata(inversion);
		return image;
		
	def __transpose_frames(image, frames):
		pixels = image.load();
		if frames > 1:
			frame_h = image.size[1] // frames;
			transpose = [];
			for y in range(frame_h):
				for f in range(frames):
					for x in range(image.size[0]):
						transpose.append(pixels[x, y + f * frame_h]);
			image = Image.new(image.mode, (image.size[0] * frames, frame_h));
			image.putdata(transpose);
		return image;
	
	def render_image(self, path, frames):
		texture, width, height = None, None, None;

		invalidate_cache = True;
		if path in self.cache:
			if frames == self.cache[path][3]:
				invalidate_cache = False;
		
		if invalidate_cache:
			image = Image.open(path);
			image = Preview.__invert_all_black(image);
			image = Preview.__transpose_frames(image, frames);

			buffer = image.tobytes();
			width = image.size[0];
			height = image.size[1];
			texture = glGenTextures(1);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
			self.cache[path] = texture, width, height, frames;
		else:
			texture, width, height, frames = self.cache[path];
		
		if texture != None:
			imgui.image(texture, (width * 2, height * 2));
	
	def render_sound(self, path):
		frames, meta = None, None;
		if path in self.cache:
			frames, meta = self.cache[path];
		else:
			clip = wave.open(path, "r");
			meta = clip.getparams();
			n_channels = meta.nchannels;
			n_frames = meta.nframes;
			width = meta.sampwidth;
			buffer = bytearray(clip.readframes(n_frames));
			samples = np.ndarray((n_frames * n_channels,), dtype=np.int32);
			for i in range(len(samples)):
				start = i * width;
				end = start + width;
				samples[i] = int.from_bytes(buffer[start:end], "little", signed=True);
			frames = np.ndarray((n_channels, n_frames), dtype=np.float32);
			for i in range(len(frames)):
				frames[i] = samples[i::n_channels];
			self.cache[path] = frames, meta;
		if not frames is None:
			for i in range(frames.shape[0]):
				half_range = (1 << ((meta.sampwidth*8)-1)) - 1;
				plot_min = -half_range;
				plot_max = half_range;
				plot_width = window_width//2;
				plot_height = window_height//10;
				imgui.plot_lines(f"####{id(frames[i])}", frames[i], scale_min=plot_min, scale_max=plot_max, graph_size=(plot_width, plot_height));
	
	def render(self, doc, node, key):
		if "*" in doc.schema.get_type(key):
			path = node[key];
			path = os.path.join(doc.parent, path);
			if not Path(path).exists():
				return;
			name, ext = os.path.splitext(path);

			if ext == ".png":
				frames = node["frames"] if "frames" in node else 1;
				self.render_image(path, frames);
			elif ext == ".wav":
				self.render_sound(path);
				if imgui.button(f"Play####{id(node)}{id(key)}"):
					playsound(path, block=False);
			else:
				return;
			imgui.same_line();
			if imgui.button(f"Refresh####{path}"):
				if path in self.cache:
					del self.cache[path];

					
		elif doc.schema.get_type(key) in asset_types:
			docs = list(filter(lambda d: d.type == doc.schema.get_type(key), asset_docs));
			if docs == []:
				return;
			d = docs[0];
			nodes = list(filter(lambda n: n["name"] == node[key], d.entries));
			if nodes == []:
				return;
			n = nodes[0];
			if n != node:
				for k in n:
					self.render(d, n, k);

preview = Preview();


#########################################################
## DOCUMENT GUI

popup_statuses = {};

class DocumentRenderer:	
	def __render(doc, node):
		imgui.separator();

		for key in node:
			key_type = doc.schema.get_type(key);
			readable = doc.schema.is_readable(key);
			writable = doc.schema.is_writable(key);
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
			
			elif key_type == "ivec2":
				imgui.text(key);
				imgui.same_line();
				_, node[key] = imgui.input_int2(get_id(node, key), node[key]);
			
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

			elif "*" in key_type:
				preview.render(doc, node, key);
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

			elif key_type in asset_types:
				preview.render(doc, node, key);
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
				if imgui.tree_node(key):
					doc.schema.push(key);
					DocumentRenderer.__render(doc, node[key]);
					doc.schema.pop();
					imgui.tree_pop();
			
			else:
				imgui.text(f"[UNSUPPORTED TYPE \"{key_type}\"]");
	
	def render(doc):
		idx = 0;
		while idx < len(doc.entries):
			node = doc.entries[idx];
			node_open = imgui.tree_node(f"{get_name(node)} {get_number(node)}####{str(id(node))}");
		
			delete_popup_id = f"delete_popup####{id(node)}";
			popup_statuses[delete_popup_id] = False;
			imgui.push_id(delete_popup_id);

			if imgui.begin_popup_context_item():	
				if imgui.menu_item_simple("Delete##init"):
					popup_statuses[delete_popup_id] = True;
					imgui.close_current_popup();
				imgui.end_popup();
			
			if popup_statuses[delete_popup_id]:
				imgui.open_popup(delete_popup_id);
				popup_statuses[delete_popup_id] = False;

			delete_node = False;
			if imgui.begin_popup(delete_popup_id):
				imgui.text(f"Delete {get_name(node)}?");
				if imgui.button("Cancel"):
					imgui.close_current_popup();
				imgui.same_line();
				if imgui.button("Delete##confirm"):
					delete_node = True;
					imgui.close_current_popup();
				imgui.end_popup();

			imgui.pop_id();

			if node_open:
				DocumentRenderer.__render(doc, node);
				imgui.tree_pop();

			if delete_node:
				del doc.entries[idx];
				idx -= 1;
			idx += 1;


#########################################################
## PROP PREVIEWER

class PropViewer:
	_ = None;

	def __init__(self):
		if PropViewer._ != None:
			return None;
		PropViewer._ = self;

		self.canvas = Canvas(240, 128);
		self.size = (640, 480);
		self.scale = 240 / self.canvas.height;
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;

		self.props = [];
		sprite_bank = [];
		for doc in asset_docs:
			if doc.type == "item":
				self.props = list(filter(lambda i: i["type"] == "prop", doc.entries));
			elif doc.type == "sprite":
				sprite_bank = doc.entries;
		self.sprites = {};
		for prop in self.props:
			for sprite in sprite_bank:
				if sprite["name"] == prop["sprite"]:
					self.sprites[prop["name"]] = sprite;

		self.parent_prop = self.props[0];
		self.child_prop = self.parent_prop;

	def __draw_prop(self, x, y, prop):
		sprite = self.sprites[prop["name"]];
		shape = prop["prop_data"]["shape"];

		tlx = x - (shape[0] * 16) // 2;
		tly = y - (shape[1] * 16);

		self.canvas.draw_flags = DrawFlags(0);
		for sy in range(shape[1]):
			for sx in range(shape[0]):
				self.canvas.draw_rect(tlx + sx * 16, tly + sy * 16, 16, 16, (255, 0, 0));

		self.canvas.draw_flags = DrawFlags.CENTER_X | DrawFlags.BOTTOM;
		self.canvas.draw_sprite(x, y, sprite, 0);
		
	def render():
		if PropViewer._ == None:
			return;
		self = PropViewer._;

		if self.open:
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"Prop Viewer", self.open, flags=self.window_flags);

			self.canvas.clear((0, 0, 0));
			draw_x = self.canvas.width/2;
			draw_y = self.canvas.height*0.75;
			self.__draw_prop(draw_x, draw_y, self.parent_prop);
			if self.child_prop["prop_data"]["type"] == "top":
				parent_shape = self.parent_prop["prop_data"]["shape"];
				child_x = (parent_shape[0] // 2) * 16;
				child_y = parent_shape[1] * 16;
				off_y = self.child_prop["prop_data"]["child_dy"];
				self.__draw_prop(draw_x, draw_y - child_y - off_y, self.child_prop);
			self.canvas.render(self.scale);

			if imgui.begin_combo(f"Parent", self.parent_prop["name"]):
				for prop in self.props:
					selected = prop["name"] == self.parent_prop["name"];
					if imgui.selectable(prop["name"], selected)[0]:
						self.parent_prop = prop;
					if selected:
						imgui.set_item_default_focus();
				imgui.end_combo();

			if self.parent_prop["prop_data"]["type"] == "bottom":
				if imgui.begin_combo(f"Child", self.child_prop["name"]):
					for prop in self.props:
						if prop["prop_data"]["type"] == "top":
							selected = prop["name"] == self.child_prop["name"];
							if imgui.selectable(prop["name"], selected)[0]:
								self.child_prop = prop;
							if selected:
								imgui.set_item_default_focus();
					imgui.end_combo();
				if self.child_prop["prop_data"]["type"] == "top":
					_, self.child_prop["prop_data"]["child_dy"] = imgui.input_int("Child dY", self.child_prop["prop_data"]["child_dy"]);

			self.size = imgui.get_window_size();
			imgui.end();
		
		if not self.open:
			PropViewer._ = None;


#########################################################
## EDITOR GUI

splash_tex = load_texture("editor/splash.png");

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

splash_flag_list = [
	imgui.WindowFlags_.no_scrollbar,
	imgui.WindowFlags_.no_scroll_with_mouse
];
splash_flags = foldl(lambda a, b : a | b, 0, splash_flag_list);

while not glfw.window_should_close(handle):
	glfw.poll_events();
	impl.process_inputs();

	glEnable(GL_PROGRAM_POINT_SIZE);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	imgui.new_frame();

	imgui.set_next_window_pos((0, 0));
	imgui.set_next_window_size((window_width, window_height));
	imgui.begin("Editor", flags=window_flags | (splash_flags if document == None else 0));

	if imgui.begin_main_menu_bar():
		if imgui.begin_menu("File"):
			if imgui.begin_menu("Open"):
				for doc in asset_docs:
					if imgui.menu_item_simple(str(doc.path), selected = document != None and doc.name == document.name):
						document = doc;
				imgui.end_menu();
			if imgui.menu_item_simple("Save", enabled=document != None):
				document.save();
			if imgui.menu_item_simple("Save All"):
				for doc in asset_docs:
					doc.save();
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
			imgui.end_menu();
		
		if imgui.begin_menu("Tools"):
			if imgui.menu_item_simple("Prop Viewer"):
				PropViewer();		
			imgui.end_menu();
	
		if imgui.begin_menu("Utils"):
			for util in Path("utils").iterdir():
				if not util.is_file() or not (os.stat(util).st_mode & S_IXUSR) > 0:
					continue;
				if imgui.menu_item_simple(str(util)):
					sp.Popen(str(util), shell=True);				
			imgui.end_menu();

		imgui.end_main_menu_bar();
	
	if document != None:
		document.refresh();
		DocumentRenderer.render(document);
	else:
		imgui.set_scroll_x(0);
		imgui.set_scroll_y(0);
		imgui.image(splash_tex[0], (splash_tex[1], splash_tex[2]));
	if PropViewer._ != None:
		PropViewer.render();

	imgui.end();
	imgui.render();
	impl.render(imgui.get_draw_data());
	imgui.end_frame();

	glfw.swap_buffers(handle);

impl.shutdown();
imgui.destroy_context();

glfw.destroy_window(handle);
glfw.terminate();

