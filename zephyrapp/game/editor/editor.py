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
from collections import OrderedDict;
import math;


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
imgui_io.config_windows_move_from_title_bar_only = True;
imgui.style_colors_dark()
impl = GlfwRenderer(handle);

def get_clipboard_text(_ctx: imgui.internal.Context) -> str:
	s = glfw.get_clipboard_string(handle);
	return s.decode();

def set_clipboard_text(_ctx: imgui.internal.Context, text: str) -> str:
	glfw.set_clipboard_string(handle, text);

platform_io = imgui.get_platform_io();
platform_io.platform_get_clipboard_text_fn = get_clipboard_text;
platform_io.platform_set_clipboard_text_fn = set_clipboard_text;

time = glfw.get_time();
delta_time = 0;


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

	def keys(self):
		return [k for k in self.path[-1].keys()];
	
	def is_valid(self, key):
		return key in self.path[-1];

	def get_type(self, key):
		return self.path[-1][key]["type"];
	
	def is_readable(self, key):
		perms = self.path[-1][key]["permissions"];
		return "read" in perms or "write" in perms;

	def is_writable(self, key):
		perms = self.path[-1][key]["permissions"];
		return "write" in perms;

	def is_ranked(self, key):
		perms = self.path[-1][key]["permissions"];
		return "rank" in perms;

	def __default(t):
		if t == None:
			return None;
		elif t == "int":
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
		elif t[0] == "[" and t[-1] == "]":
			return [];
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

			if isinstance(child["type"], dict):
				self.push(key);
				if not key in node:
					node[key] = {};
				self.prototype(node[key]);
				self.pop();
				
			if not key in node:
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
	
		self.id_set = set();
		if "id" in self.schema.keys():
			for entry in self.entries:
				self.id_set.add(entry["id"]);
	
	def take_free_id(self):
		M = max(self.id_set);
		i = 0;
		while i <= M:
			if not i in self.id_set:
				self.id_set.add(i);
				return i;
			i += 1;
		self.id_set.add(M+1);
		return M+1;
	
	def spawn_new_entry(self):
		new = {};
		self.schema.prototype(new);
		new["name"] = f"new_{document.type}";
		if "id" in new:
			new["id"] = self.take_free_id();
		self.entries.append(new);
	
	def duplicate_entry(self, idx):
		new = copy.deepcopy(self.entries[idx]);
		if "id" in new:
			new["id"] = self.take_free_id();
		self.entries.append(new);
	
	def delete_entry(self, idx):
		entry = self.entries[idx];
		if "id" in entry:
			self.id_set.remove(entry["id"]);
		del self.entries[idx];
	
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
asset_types = [];
asset_docs = {};

for folder in asset_dirs:
	for entry in folder.iterdir():
		name, ext = os.path.splitext(entry);
		if ext == ".json":
			try:
				doc = AssetDocument(entry);
				asset_type = doc.type;
				if not asset_type in asset_types:
					asset_types.append(asset_type);
				asset_docs[asset_type] = doc;
			except:
				print(f"Failed to load {entry} as asset document!");

document = None;

def find_asset(asset_type, name):
	for asset in asset_docs[asset_type].entries:
		if asset["name"] == name:
			return asset;
	return None;


#########################################################
## RENDERING

def make_texture(buffer, width, height):
	texture = glGenTextures(1);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	return texture;

class DrawFlags(Flag):
	CENTER_X = auto()
	CENTER_Y = auto()
	BOTTOM = auto()

class Canvas:
	def __init__(self, width, height):
		self.width = width;
		self.height = height;
		self.buffer = bytearray(bytes(width * height * 4));
		self.texture = make_texture(self.buffer, width, height);
		self.draw_flags = ();

	def clear(self, c):
		for y in range(self.height):
			for x in range(self.width):
				i = (y * self.width + x) * 4;
				self.buffer[i+0] = c[0];
				self.buffer[i+1] = c[1];
				self.buffer[i+2] = c[2];
				self.buffer[i+3] = 255;

	def draw_pixel(self, x, y, c):
		x = int(x);
		y = int(y);

		if x < 0 or x >= self.width:
			return;
		if y < 0 or y >= self.height:
			return;

		i = (y * self.width + x) * 4;
		self.buffer[i+0] = c[0];
		self.buffer[i+1] = c[1];
		self.buffer[i+2] = c[2];
	
	def draw_hline(self, y, c):
		for x in range(self.width):
			self.draw_pixel(x, y, c);
	
	def draw_vline(self, x, c):
		for y in range(self.height):
			self.draw_pixel(x, y, c);

	def draw_rect(self, x, y, w, h, c):
		x = int(x);
		y = int(y);
		w = int(w);
		h = int(h);

		if DrawFlags.CENTER_X in self.draw_flags:
			x -= w // 2;
		if DrawFlags.CENTER_Y in self.draw_flags:
			y -= h // 2;
		elif DrawFlags.BOTTOM in self.draw_flags:
			y -= h;

		for dy in range(y, y+h):
			self.draw_pixel(x, dy, c);
			self.draw_pixel(x+w-1, dy, c);
		for dx in range(x, x+w):
			self.draw_pixel(dx, y, c);
			self.draw_pixel(dx, y+h-1, c);

	def draw_line(self, xi, yi, xf, yf, c):
		xi = int(xi);
		yi = int(yi);
		xf = int(xf);
		yf = int(yf);
	
		steep = abs(yf-yi) > abs(xf-xi);
		if steep:
			temp = xi;
			xi = yi;
			yi = temp;

			temp = xf;
			xf = yf;
			yf = temp;

		# if the line heads left, swap its start and end points
		leftward = xi > xf;
		if leftward:
			temp = xi;
			xi = xf;
			xf = temp;

			temp = yi;
			yi = yf;
			yf = temp;
		
		dx = xf - xi;
		dy = yf - yi;

		# account for line heading up or down
		y_step = 1 if (yf > yi) else -1;
		y = yi;
		
		# approximate d_err as abs(dy) / (dx ~= 0.5)
		d_err = abs(dy) * 2;
		err = 0;

		# if line is steep, we swap x,y in the draw call to undo our earlier transposition
		# we employ a branch between two for loops to avoid branching within one loop
		if steep:
			for x in range(xi, xf):
				xw = x;
				if y >= 0 and y < self.width and xw >= 0 and xw < self.height:
					self.draw_pixel(y, xw, c);

				err += d_err;
				if err > dx:
					y += y_step;
					err -= 2*dx;
		else:
			for x in range(xi, xf):
				yw = y;
				if x >= 0 and x < self.width and yw >= 0 and yw < self.height:
					self.draw_pixel(x, yw, c);

				err += d_err;
				if err > dx:
					y += y_step;
					err -= dx*2;

	def draw_circle(self, x, y, r, c):
		x = int(x);
		y = int(y);
		r = int(r);
	
		f = 1 - r;
		ddfx = 0;
		ddfy = -2 * r;
		dx = 0;
		dy = r;

		self.draw_pixel(x, y + r, c);
		self.draw_pixel(x, y - r, c);
		self.draw_pixel(x + r, y, c);
		self.draw_pixel(x - r, y, c);

		while dx < dy:
			if f >= 0:
				dy -= 1;
				ddfy += 2;
				f += ddfy;

			dx += 1;
			ddfx += 2;
			f += ddfx + 1;

			self.draw_pixel(x + dx, y + dy, c);
			self.draw_pixel(x - dx, y + dy, c);
			self.draw_pixel(x + dx, y - dy, c);
			self.draw_pixel(x - dx, y - dy, c);
			self.draw_pixel(x + dy, y + dx, c);
			self.draw_pixel(x - dy, y + dx, c);
			self.draw_pixel(x + dy, y - dx, c);
			self.draw_pixel(x - dy, y - dx, c);
	
	def draw_image(self, x, y, image):
		x = int(x);
		y = int(y);

		if DrawFlags.CENTER_X in self.draw_flags:
			x -= image.width // 2;
		if DrawFlags.CENTER_Y in self.draw_flags:
			y -= image.height // 2;
		elif DrawFlags.BOTTOM in self.draw_flags:
			y -= image.height;

		pixels = image.load();
		for yr in range(0, image.height):
			yw = y+yr;
			if yw < 0 or yw >= self.height:
				continue;
			for xr in range(0, image.width):
				xw = x+xr;
				if xw < 0 or xw >= self.width:
					continue;
				c = pixels[xr, yr];
				if c[3] < 128:
					continue;

				i = (yw * self.width + xw) * 4;
				self.buffer[i+0] = c[0];
				self.buffer[i+1] = c[1];
				self.buffer[i+2] = c[2];
	
	def render(self, scale):
		glBindTexture(GL_TEXTURE_2D, self.texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, self.width, self.height, GL_RGBA, GL_UNSIGNED_BYTE, self.buffer);
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
			listings.sort();
			
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"File Explorer ({self.current.name})", self.open, flags=self.window_flags);
			for item in listings:
				name = ".." if item == self.current.parent.absolute() else item.name;
				name = f"{name}/" if item.is_dir() else name;
				if imgui.menu_item_simple(name):
					if item.is_dir():
						self.current = item;
					else:
						FileExplorer.result = Path(os.path.relpath(item.absolute(), self.anchor.absolute()));
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
## SPRITE EXPLORER

class SpriteExplorer:
	_ = None;
	result = None;

	def __init__(self, target):
		if SpriteExplorer._ != None:
			return None;
		SpriteExplorer._ = self;
		SpriteExplorer.result = None;

		self.target = target;

		self.size = (640, 480);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;
	
	def is_active(target):
		return SpriteExplorer._ != None and SpriteExplorer._.target == target;

	def render():
		if SpriteExplorer._ == None:
			return;

		self = SpriteExplorer._;
		if self.open:
			listings = [s["name"] for s in asset_docs["sprite"].entries];
			listings.sort();
			
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"Sprite Explorer", self.open, flags=self.window_flags);
			for item in listings:
				Preview.thumbnail("sprite", item);
				if imgui.menu_item_simple(item):
					SpriteExplorer.result = item;
					SpriteExplorer._ = None;
			self.size = imgui.get_window_size();
			imgui.end();
		if not self.open:
			SpriteExplorer._ = None;
		
	def harvest():
		ready, result = SpriteExplorer.result != None, SpriteExplorer.result;
		SpriteExplorer.result = None;
		return ready, result;

#########################################################
## ASSET PREVIEWER

class PreviewSprite:
	def __invert_black(self, image):
		pixels = image.load();
		result = Image.new(self.image.mode, self.image.size);

		all_black = True;
		inversion = [];
		for y in range(self.image.height):
			for x in range(self.image.width):
				p = pixels[x, y];
				if p[3] >= 128:
					all_black &= (p[0] == 0 and p[1] == 0 and p[2] == 0);
					if not all_black:
						return image;
					inversion.append((255, 255, 255, p[3]));
				else:
					inversion.append(p);

		result.putdata(inversion);
		return result;
		
	def __transpose_frames(self, image):
		pixels = image.load();
		if self.frame_count > 1:
			frame_h = image.height // self.frame_count;
			transpose = [];
			for y in range(frame_h):
				for f in range(self.frame_count):
					for x in range(image.width):
						transpose.append(pixels[x, y + f * frame_h]);
			result = Image.new(image.mode, (image.width * self.frame_count, frame_h));
			result.putdata(transpose);
			return result;
		else:
			return image;
	
	def __make_frame(self, frame):
		pixels = self.image.load();

		frame_h = self.height // self.frame_count;
		frame_y = frame_h * frame;
		frame = [];
		for dy in range(0, frame_h):
			for dx in range(0, self.width):
				frame.append(pixels[dx, dy+frame_y]);

		result = Image.new(self.image.mode, (self.width, frame_h));
		result.putdata(frame);
		return result;

	def __init__(self, sprite):
		path = Path(os.path.join("sprites", sprite["path"]));
		self.image = Image.open(path);
		self.width = self.image.width;
		self.height = self.image.height;
		self.frame_count = max(sprite["frames"], 1);
		self.frame_width = self.width;
		self.frame_height = self.height // self.frame_count;
		self.frame_images = [self.__make_frame(i) for i in range(self.frame_count)];
		self.frame_textures = [make_texture(f.tobytes(), f.width, f.height) for f in self.frame_images];
		self.preview_image = self.image;
		self.preview_image = self.__invert_black(self.preview_image);
		self.preview_image = self.__transpose_frames(self.preview_image);
		self.preview_texture = make_texture(self.preview_image.tobytes(), self.preview_image.width, self.preview_image.height);

class PreviewSound:
	def __init__(self, sound):
		path = os.path.join("sounds", sound["path"]);
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
		
		self.frames = frames;
		self.meta = meta;
		self.path = path;

class PreviewBank:
	def __init__(self):
		self.entries = {};
		for asset_type in asset_types:
			self.entries[asset_type] = {};
	
	def init(self, asset_type, name):
		try:
			asset = find_asset(asset_type, name);
			match asset_type:
				case "sprite":
					self.entries[asset_type][name] = PreviewSprite(asset);
				case "sound":
					self.entries[asset_type][name] = PreviewSound(asset);
		except:
			self.entries[asset_type][name] = None;
		return self.entries[asset_type][name];
	
	def get(self, asset_type, name):
		if not name in self.entries[asset_type]:
			self.init(asset_type, name);
		return self.entries[asset_type][name];
		
preview_bank = PreviewBank();

class Preview:	
	def render_sprite(name):
		sprite = preview_bank.get("sprite", name);
		if sprite != None:
			if imgui.button(f"!##{name}"):
				preview_bank.init("sprite", name);
			imgui.same_line();
			imgui.image(sprite.preview_texture, (sprite.preview_image.width * 2, sprite.preview_image.height * 2));
		else:
			preview_bank.init("sprite", name);

	def thumbnail_sprite(name):
		sprite = preview_bank.get("sprite", name);
		if sprite != None:
			imgui.image(sprite.frame_textures[0], (48, 48));
			imgui.same_line();
	
	def render_sound(name):
		sound = preview_bank.get("sound", name);
		if sound != None:
			for i in range(sound.frames.shape[0]):
				half_range = (1 << ((sound.meta.sampwidth*8)-1)) - 1;
				plot_min = -half_range;
				plot_max = half_range;
				plot_width = window_width//2;
				plot_height = window_height//10;
				imgui.plot_lines(f"####{id(sound.frames[i])}", sound.frames[i], scale_min=plot_min, scale_max=plot_max, graph_size=(plot_width, plot_height));
			if imgui.button(f"Play##{name}"):
				playsound(sound.path, block=False);
			imgui.same_line();
		if imgui.button(f"Refresh##{name}"):
			preview_bank.init("sound", name);
	
	def render(asset_type, name):
		match asset_type:
			case "sprite":
				Preview.render_sprite(name);
			case "sound":
				Preview.render_sound(name);	

	def thumbnail(asset_type, name):
		match asset_type:
			case "sprite":
				Preview.thumbnail_sprite(name);


#########################################################
## DOCUMENT GUI

popup_statuses = {};
search_term = "";

class DocumentRenderer:	
	def __render(doc, node):
		imgui.separator();
		
		if "name" in node:
			Preview.render(doc.type, node["name"]);

		for key in node:
			if not doc.schema.is_valid(key):
				continue;
			
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

			elif key_type == "float":
				imgui.text(key);
				imgui.same_line();
				if writable:
					_, node[key] = imgui.input_float(get_id(node, key), node[key]);
				else:
					imgui.text(str(node[key]));

			elif key_type == "string":
				imgui.text(key);
				imgui.same_line();
				if writable:
					_, output = imgui.input_text(get_id(node, key), node[key]);
					node[key] = output;
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
				Preview.render(key_type, node[key]);
				imgui.text(key);
				imgui.same_line();

				if key_type == "sprite":
					if writable:
						_, node[key] = imgui.input_text(get_id(node, key), node[key]);
						imgui.same_line();
						ident = get_id(node, key);
						if imgui.button(f"...{ident}"):
							SpriteExplorer(ident);
						if SpriteExplorer.is_active(ident):
							SpriteExplorer.render();
							ready, result = SpriteExplorer.harvest();
							node[key] = result if ready else node[key];
					else:
						imgui.text(node[key]);
				else:
					if imgui.begin_combo(get_id(node, key), str(node[key])):
						assets = asset_docs[key_type].entries;
						for asset in assets:
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
		global search_term;
		_, search_term = imgui.input_text("Search", search_term);

		idx = 0;
		while idx < len(doc.entries):
			node = doc.entries[idx];
			if len(search_term) > 0 and search_term not in node['name']:
				idx += 1;
				continue;
			node_open = imgui.tree_node(f"{get_name(node)} {get_number(node)}####{str(id(node))}");
		
			delete_popup_id = f"delete_popup####{id(node)}";
			popup_statuses[delete_popup_id] = False;
			imgui.push_id(delete_popup_id);

			if imgui.begin_popup_context_item():	
				if imgui.menu_item_simple("Delete##init"):
					popup_statuses[delete_popup_id] = True;
					imgui.close_current_popup();
				if imgui.menu_item_simple("Duplicate"):
					doc.duplicate_entry(idx);
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
				doc.delete_entry(idx);
				idx -= 1;
			idx += 1;


#########################################################
## PROP VIEWER

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

		self.props = list(filter(lambda i: i["type"] == "prop", asset_docs["item"].entries));
		self.parent_prop = self.props[0];
		self.child_prop = self.parent_prop;

	def __draw_prop(self, x, y, prop):
		shape = prop["prop_data"]["shape"];
		tlx = x - (shape[0] * 16) // 2;
		tly = y - (shape[1] * 16);
		self.canvas.draw_flags = DrawFlags(0);
		for sy in range(shape[1]):
			for sx in range(shape[0]):
				self.canvas.draw_rect(tlx + sx * 16, tly + sy * 16, 16, 16, (255, 0, 0));

		self.canvas.draw_flags = DrawFlags.CENTER_X | DrawFlags.BOTTOM;
		sprite = preview_bank.get("sprite", prop["sprite"]);
		self.canvas.draw_image(x, y, sprite.frame_images[0]);
		
	def render():
		if PropViewer._ == None:
			return;
		self = PropViewer._;

		if self.open:
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"Prop Viewer", self.open, flags=self.window_flags);

			self.canvas.clear((128, 128, 128));
			draw_x = self.canvas.width/2;
			draw_y = self.canvas.height*0.75;
			self.__draw_prop(draw_x, draw_y, self.parent_prop);
			if self.parent_prop["prop_data"]["type"] == "bottom" and self.child_prop["prop_data"]["type"] == "top":
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
## ANIMATION VIEWER

class AnimationViewer:
	_ = None;

	def __init__(self):
		if AnimationViewer._ != None:
			return None;
		AnimationViewer._ = self;

		self.canvas = Canvas(240, 128);
		self.size = (640, 480);
		self.scale = 240 / self.canvas.height;
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;

		self.sprites = asset_docs["sprite"].entries;
		self.sprite_idx = 0;
		self.frame = 0;
		
		self.animate = True;
		self.show_AABB = False;
		self.timer = 0;

	def render():
		if AnimationViewer._ == None:
			return;
		self = AnimationViewer._;

		if self.open:
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"Animation Viewer", self.open, flags=self.window_flags);

			sprite = self.sprites[self.sprite_idx];
			preview = preview_bank.get("sprite", sprite["name"]);

			self.canvas.clear((128, 128, 128));
			draw_x = self.canvas.width/2;
			draw_y = self.canvas.height/2;
			self.canvas.draw_flags = DrawFlags.CENTER_X | DrawFlags.CENTER_Y;
			self.canvas.draw_image(draw_x, draw_y, preview.frame_images[self.frame]);
			if self.show_AABB:
				self.canvas.draw_rect(draw_x, draw_y, preview.width, preview.height/preview.frame_count, (255, 0, 0));
			self.canvas.render(self.scale);

			if preview.frame_count > 1:	
				animate_changed, self.animate = imgui.checkbox("Animate", self.animate);
				if animate_changed:
					self.frame = 0;
				if self.animate:
					self.timer += delta_time;
					if self.timer >= 0.2:
						self.timer = 0;
						self.frame += 1;
						if self.frame >= preview.frame_count:
							self.frame = 0;
				else:
					_, self.frame = imgui.slider_int("Frame", self.frame, 0, preview.frame_count-1);
			
			show_AABB_changed, self.show_AABB = imgui.checkbox("Show AABB", self.show_AABB);

			if imgui.begin_combo(f"Sprite", sprite["name"]):
				for (idx, entry) in enumerate(self.sprites):
					selected = idx == self.sprite_idx;
					if imgui.selectable(entry["name"], selected)[0]:
						self.sprite_idx = idx;
						self.frame = 0;
					if selected:
						imgui.set_item_default_focus();
				imgui.end_combo();

			self.size = imgui.get_window_size();
			imgui.end();
		
		if not self.open:
			AnimationViewer._ = None;


#########################################################
## THEME EDITOR

class ThemeEditor:
	_ = None;

	def __init__(self):
		if ThemeEditor._ != None:
			return None;
		ThemeEditor._ = self;

		self.size = (720, 480);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;

		self.grid = False;

		self.themes = asset_docs["theme"].entries;
		self.theme = self.themes[0];

		self.wall_canvas = self.wall_canvas = Canvas(240, 96);
		self.floor_canvas = Canvas(240, 224);

		self.cursor = (0, 0);
		self.wall_brush = 0;
		self.floor_brush = 0;

	def render():
		if ThemeEditor._ == None:
			return;
		self = ThemeEditor._;

		if self.open:
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"Theme Editor", self.open, flags=self.window_flags);

			if imgui.begin_combo(f"Theme", self.theme["name"]):
				for theme in self.themes:
						selected = theme == self.theme;
						if imgui.selectable(theme["name"], selected)[0]:
							self.theme = theme;
						if selected:
							imgui.set_item_default_focus();
				imgui.end_combo();
			
			if not self.theme['tile_wall']:
				sprite = preview_bank.get("sprite", self.theme["wall_tiles"]);
				self.wall_canvas = Canvas(sprite.width, sprite.height);
			elif self.wall_canvas.height != 96:
				self.wall_canvas = Canvas(240, 96);
			if not self.theme['tile_floor']:
				sprite = preview_bank.get("sprite", self.theme["floor_tiles"]);
				self.floor_canvas = Canvas(sprite.width, sprite.height);
			elif self.floor_canvas.height != 224:
				self.floor_canvas = Canvas(240, 224);
			
			if self.theme['tile_wall']:
				wall_len = len(self.theme["wall_map"]);
				deviation = (15 * 6) - wall_len;
				if deviation < 0:
					self.theme["wall_map"] = self.theme["wall_map"][:deviation];
				else:
					self.theme["wall_map"] += [0 for i in range(deviation)];

				canvas_pos = imgui.get_cursor_screen_pos();
				self.wall_canvas.clear((128, 128, 128));
				for y in range(6):
					for x in range(15):
						tile = preview_bank.get("sprite", self.theme["wall_tiles"]);
						tile_frame = self.theme["wall_map"][y * 15 + x];
						if tile_frame >= tile.frame_count:
							tile_frame = tile.frame_count-1;
						self.wall_canvas.draw_image(x * 16, y * 16, tile.frame_images[tile_frame]);
				if(self.grid):
					for y in range(1, 6):
						self.wall_canvas.draw_hline(y * 16, (255, 255, 255));
					for x in range(1, 15):
						self.wall_canvas.draw_vline(x * 16, (255, 255, 255));

				mouse_pos = imgui_io.mouse_pos;
				brush_pos = mouse_pos - canvas_pos;
				if brush_pos.x >= 0 and brush_pos.x < (15 * 16) and brush_pos.y >= 0 and brush_pos.y < (6 * 16):
					self.wall_cursor = (int(brush_pos.x // 16), int(brush_pos.y // 16));
					if imgui.is_mouse_down(0):
						idx = self.wall_cursor[1] * 15 + self.wall_cursor[0];
						self.theme["wall_map"][idx] = self.wall_brush;
					self.wall_canvas.draw_rect(self.wall_cursor[0] * 16, self.wall_cursor[1] * 16, 16, 16, (255, 0, 0));
				
				window_rect = self.theme['window_rect']
				if len(window_rect) == 4:
					self.wall_canvas.draw_rect(window_rect[0], window_rect[1], window_rect[2], window_rect[3], (0, 0, 255));

				self.wall_canvas.render(1);
				imgui.same_line();
				_, self.grid = imgui.checkbox("Show grid", self.grid);	

				wall_tiles = preview_bank.get("sprite", self.theme["wall_tiles"]);
				per_line = 0;
				for (idx, frame) in enumerate(wall_tiles.frame_textures):
					tint = (0.5, 0.5, 0.5, 1) if idx == self.wall_brush else (1, 1, 1, 1);
					if imgui.image_button(f"##{id(frame)}", frame, (32, 32), tint_col=tint):
						self.wall_brush = idx;
					imgui.same_line();
					per_line += 1;
					if per_line >= 5:
						imgui.new_line();
						per_line = 0;
				imgui.new_line();
			else:
				self.wall_canvas.clear((128, 128, 128));

				sprite = preview_bank.get("sprite", self.theme["wall_tiles"]);
				self.wall_canvas.draw_image(0, 0, sprite.frame_images[0]);

				window_rect = self.theme['window_rect']
				if len(window_rect) == 4:
					self.wall_canvas.draw_rect(window_rect[0], window_rect[1], window_rect[2], window_rect[3], (0, 0, 255));
				
				self.wall_canvas.render(1);
			
			if self.theme['tile_floor']:
				floor_len = len(self.theme["floor_map"]);
				deviation = (15 * 14) - floor_len;
				if deviation < 0:
					self.theme["floor_map"] = self.theme["floor_map"][:deviation];
				else:
					self.theme["floor_map"] += [0 for i in range(deviation)];	
			
				canvas_pos = imgui.get_cursor_screen_pos();
				self.floor_canvas.clear((128, 128, 128));
				for y in range(14):
					for x in range(15):
						tile = preview_bank.get("sprite", self.theme["floor_tiles"]);
						tile_frame = self.theme["floor_map"][y * 15 + x];
						if tile_frame >= tile.frame_count:
							tile_frame = tile.frame_count-1;
						self.floor_canvas.draw_image(x * 16, y * 16, tile.frame_images[tile_frame]);
				if(self.grid):
					for y in range(1, 14):
						self.floor_canvas.draw_hline(y * 16, (255, 255, 255));
					for x in range(1, 15):
						self.floor_canvas.draw_vline(x * 16, (255, 255, 255));
				
				mouse_pos = imgui_io.mouse_pos;
				brush_pos = mouse_pos - canvas_pos;
				if brush_pos.x >= 0 and brush_pos.x < (15 * 16) and brush_pos.y >= 0 and brush_pos.y < (14 * 16):
					self.floor_cursor = (int(brush_pos.x // 16), int(brush_pos.y // 16));
					if imgui.is_mouse_down(0):
						idx = self.floor_cursor[1] * 15 + self.floor_cursor[0];
						self.theme["floor_map"][idx] = self.floor_brush;
					self.floor_canvas.draw_rect(self.floor_cursor[0] * 16, self.floor_cursor[1] * 16, 16, 16, (255, 0, 0));

				self.floor_canvas.render(1);

				floor_tiles = preview_bank.get("sprite", self.theme["floor_tiles"]);
				per_line = 0;
				for (idx, frame) in enumerate(floor_tiles.frame_textures):
					tint = (0.5, 0.5, 0.5, 1) if idx == self.floor_brush else (1, 1, 1, 1);
					if imgui.image_button(f"##{id(frame)}", frame, (32, 32), tint_col=tint):
						self.floor_brush = idx;
					imgui.same_line();
					per_line += 1;
					if per_line >= 5:
						imgui.new_line();
						per_line = 0;
				imgui.new_line();
			
			if len(self.theme['window_rect']) < 4:
				self.theme['window_rect'] = [8, 8, 112, 64];
			rect = self.theme['window_rect'];
			_, rect[0:2] = imgui.input_int2("XY", rect[0:2]);
			_, rect[2:] = imgui.input_int2("WH", rect[2:]);
			
			self.size = imgui.get_window_size();
			imgui.end();
		
		if not self.open:
			ThemeEditor._ = None;


#########################################################
## MESH2D EDITOR

class Mesh2DEditor:
	_ = None;

	def __init__(self):
		if Mesh2DEditor._ != None:
			return None;
		Mesh2DEditor._ = self;

		self.polyline = [];
		self.edge_buffer = [];
		self.trash = None;

		self.canvas = Canvas(240, 120);
		self.canvas_scale = 2;
		self.size = (720, 720);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;

		self.grid_dist = 4;
		self.show_grid = True;
		self.show_verts = False;
		
		self.edit_mode = False;
		self.was_left_click = False;
		self.was_right_click = False;
		self.last_edge_closed = True;
		self.last_click = imgui.ImVec2(-1, -1);
	
		self.mesh_pool = asset_docs["mesh2d"].entries;
		self.mesh = self.mesh_pool[0] if len(self.mesh_pool) > 0 else None;
		if(not self.mesh is None):
			self.open_mesh(self.mesh);
	
	def in_bounds(self, p):
		if p.x < 0:
			return False;
		if p.x >= self.canvas.width:
			return False;
		if p.y < 0:
			return False;
		if p.y >= self.canvas.height:
			return False;
		return True;

	def open_mesh(self, mesh):
		self.mesh = mesh;
		self.polyline = [];
		self.edge_buffer = [];
		if not mesh is None:
			V = mesh['verts'];
			E = mesh['edges'];
			n_E = mesh['edge_count'];
			e_idx = 0;
			while e_idx < n_E:
				v0_idx = E[e_idx*2+0];
				v1_idx = E[e_idx*2+1];
				v0_x = V[v0_idx*2+0];
				v0_y = V[v0_idx*2+1];
				v1_x = V[v1_idx*2+0];
				v1_y = V[v1_idx*2+1];
				self.polyline.append([
					imgui.ImVec2(v0_x, v0_y),
					imgui.ImVec2(v1_x, v1_y)
				]);
				e_idx += 1;
	
	def shunt(self):
		corner = imgui.ImVec2(256, 256);
		for [v0, v1] in self.polyline:
			corner.x = min(v0.x, v1.x, corner.x);
			corner.y = min(v0.y, v1.y, corner.y);
		for [v0, v1] in self.polyline:
			v0 -= corner;
			v1 -= corner;
	def center(self):
		tl = imgui.ImVec2(256, 256);
		br = imgui.ImVec2(-1, -1);
		for [v0, v1] in self.polyline:
			tl.x = min(v0.x, v1.x, tl.x);
			tl.y = min(v0.y, v1.y, tl.y);
			br.x = max(v0.x, v1.x, br.x);
			br.y = max(v0.y, v1.y, br.y);
		middle = (tl+br)/2;
		shift = imgui.ImVec2(120, 60) - middle;
		for [v0, v1] in self.polyline:
			v0 += shift;
			v1 += shift;
	def translate(self, delta):
		for [v0, v1] in self.polyline:
			v0 += delta;
			v1 += delta;

	def write_mesh(self):
		polyline = [(v0, v1) for [v0, v1] in self.polyline];
		polyline = list(set(polyline));
		flat_polyline = [];
		for p in polyline:
			flat_polyline += p;
		
		vert_map = OrderedDict();
		for v in flat_polyline:
			if not v in vert_map:
				vert_map[v] = len(vert_map);
		
		flat_edges = [];
		for v in flat_polyline:
			flat_edges.append(vert_map[v]);
		flat_verts = [];
		for v in vert_map.keys():
			flat_verts += [int(v.x), int(v.y)];
		
		self.mesh['verts'] = flat_verts.copy();
		self.mesh['vert_count'] = len(flat_verts) // 2;
		self.mesh['edges'] = flat_edges.copy();
		self.mesh['edge_count'] = len(flat_edges) // 2;
	
	def close_mesh(self):
		if not self.mesh is None:
			self.write_mesh();
			self.mesh = None;
	
	def buffer_vertex(self, v):
		self.edge_buffer.append(imgui.ImVec2(v));
		if len(self.edge_buffer) == 2:
			l = self.edge_buffer[0];
			if v != l:
				self.polyline.append(self.edge_buffer);
				self.edge_buffer = [imgui.ImVec2(v)];
			else:
				self.edge_buffer = [];
	
	def delete_vertex(self, v):
		if v in self.edge_buffer:
			self.edge_buffer = [];
		i = 0;
		while i < len(self.polyline):
			if v in self.polyline[i]:
				del self.polyline[i];
				i -= 1;
			i += 1;
	
	def snap(self, p):
		p.x /= self.grid_dist;
		p.y /= self.grid_dist;
		p.x = round(p.x);
		p.y = round(p.y);
		p.x *= self.grid_dist;
		p.y *= self.grid_dist;
		p.x = min(max(p.x, 0), self.canvas.width-1);
		p.y = min(max(p.y, 0), self.canvas.height-1);
		return p;

	def render():
		if Mesh2DEditor._ == None:
			return;
		self = Mesh2DEditor._;

		if self.open:
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin("Mesh2D Editor", self.open, flags=self.window_flags);

			if imgui.begin_combo(f"Meshes", self.mesh["name"] if not self.mesh is None else "None"):
				for mesh in self.mesh_pool:
					selected = mesh == self.mesh;
					if imgui.selectable(mesh["name"], selected)[0]:
						self.close_mesh();
						self.open_mesh(mesh);
					if selected:
						imgui.set_item_default_focus();
				imgui.end_combo();

			canvas_pos = imgui.get_cursor_screen_pos();
			mouse_pos = imgui_io.mouse_pos;
			brush_pos = mouse_pos - canvas_pos;
			brush_pos /= self.canvas_scale;
			in_bounds = self.in_bounds(brush_pos);
			vertex = self.snap(brush_pos);

			if self.edit_mode:
				if self.last_click == imgui.ImVec2(-1, -1):
					self.last_click = vertex;
				if in_bounds:
					if imgui.is_mouse_down(0) and not self.was_left_click:
						if imgui_io.key_shift:
							tl = imgui.ImVec2(256, 256);
							br = imgui.ImVec2(-1, -1);
							for [v0, v1] in self.polyline:
								tl.x = min(v0.x, v1.x, tl.x);
								tl.y = min(v0.y, v1.y, tl.y);
								br.x = max(v0.x, v1.x, br.x);
								br.y = max(v0.y, v1.y, br.y);
							middle = (tl+br)/2;
							delta = vertex - middle;
							self.translate(delta);
							self.last_click = vertex;
						else:
							self.buffer_vertex(vertex);
							self.was_left_click = True;
					if not imgui.is_mouse_down(0):
						self.was_left_click = False;
					
					if imgui.is_mouse_down(1) and not self.was_right_click:
						self.delete_vertex(vertex);
						self.was_right_click = True;
					if not imgui.is_mouse_down(1):
						self.was_right_click = False;
			
			self.canvas.clear((0, 0, 0));
			if self.show_grid:
				for y in range(0, self.canvas.height, self.grid_dist):
					for x in range(0, self.canvas.width, self.grid_dist):
						self.canvas.draw_pixel(x, y, (128, 128, 128));
				self.canvas.draw_rect(0, 0, self.canvas.width, self.canvas.height, (128, 128, 128));
			
			for [v0, v1] in self.polyline:
				self.canvas.draw_line(v0.x, v0.y, v1.x, v1.y, (255, 255, 255));
				if self.show_verts:
					self.canvas.draw_circle(v0.x, v0.y, 1, (255, 255, 255));
					self.canvas.draw_circle(v1.x, v1.y, 1, (255, 255, 255));
			
			if self.edit_mode:
				if len(self.edge_buffer) > 0:
					last = self.edge_buffer[-1];
					self.canvas.draw_line(last.x, last.y, vertex.x, vertex.y, (255, 255, 255));
				self.canvas.draw_circle(vertex.x, vertex.y, 2, (255, 255, 255));
				
			self.canvas.render(self.canvas_scale);
			_, self.show_grid = imgui.checkbox("Show grid", self.show_grid);
			imgui.same_line();
			_, self.show_verts = imgui.checkbox("Show verts", self.show_verts);
			imgui.same_line();
			imgui.push_item_width(72);
			_, self.grid_dist = imgui.input_int("Cell size", self.grid_dist);
			imgui.pop_item_width();

			_, self.edit_mode = imgui.checkbox("Edit Mode", self.edit_mode);
			if self.edit_mode:
				imgui.same_line();
				if imgui.button("Shunt"):
					self.shunt();
				imgui.same_line();
				if imgui.button("Center"):
					self.center();
				if imgui.button("Clear"):
					self.trash = self.polyline, self.edge_buffer;
					self.polyline = [];
					self.edge_buffer = [];
				imgui.same_line();
				if imgui.button("Restore") and self.trash != None:
					self.polyline, self.edge_buffer = self.trash;
					self.trash = None;
			
			self.size = imgui.get_window_size();
			imgui.end();
		
		if not self.open:
			self.close_mesh();
			Mesh2DEditor._ = None;


#########################################################
## ITEM REFORMER

class ItemReform:
	def __init__(self, name, filt, path, values):
		self.name = name;
		self.filt = filt;
		self.path = path;
		self.values = values;
	
	def __path_assign(object, path, value):
		match path:
			case []:
				return;
			case [token]:
				object[token] = value;
			case head, *tail:
				ItemReform.__path_assign(object[head], tail, value);

	def apply(self, items):
		operands = filter(self.filt, items);
		for operand in operands:
			ItemReform.__path_assign(operand, self.path, self.values[operand["tier"]]);

class ItemReformer:
	_ = None;

	def __init__(self):
		if ItemReformer._ != None:
			return None;
		ItemReformer._ = self;

		self.canvas = Canvas(240, 128);
		self.size = (640, 480);
		self.scale = 240 / self.canvas.height;
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;
		
		self.items = asset_docs["item"].entries;
		self.reforms = [
			ItemReform (
				"Food Prices",
				lambda i: i["type"] == "tool" and i["tool_data"]["type"] == "food",
				["price"],
				[tier*3 for tier in range(4)]
			),
			ItemReform (
				"Book and Toy Prices",
				lambda i: i["type"] == "tool" and (i["tool_data"]["type"] == "book" or i["tool_data"]["type"] == "toy"),
				["price"],
				[tier*7 for tier in range(4)]
			),
			ItemReform (
				"Food Power",
				lambda i: i["type"] == "tool" and i["tool_data"]["type"] == "food",
				["tool_data", "dv"],
				[tier for tier in range(4)]
			),
			ItemReform (
				"Book Power",
				lambda i: i["type"] == "tool" and i["tool_data"]["type"] == "book",
				["tool_data", "df"],
				[tier for tier in range(4)]
			),
			ItemReform (
				"Toy Power",
				lambda i: i["type"] == "tool" and i["tool_data"]["type"] == "toy",
				["tool_data", "ds"],
				[tier for tier in range(4)]
			),
		];

	def render():
		if ItemReformer._ == None:
			return;
		self = ItemReformer._;

		if self.open:
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"Item Reformer", self.open, flags=self.window_flags);

			for reform in self.reforms:
				imgui.push_id(str(id(reform)));
				if imgui.collapsing_header(reform.name):
					for tier in range(1, 4):
						imgui.push_id(str(tier));
						imgui.text(f"Tier {"I"*tier}:");
						imgui.same_line();
						_, reform.values[tier] = imgui.input_int("", reform.values[tier]);
						imgui.pop_id();
					if imgui.button("Apply"):
						reform.apply(self.items);
				imgui.pop_id();

			self.size = imgui.get_window_size();
			imgui.end();
		
		if not self.open:
			ItemReformer._ = None;


#########################################################
## DIALOGUE EDITOR

class DialogueEditor:
	_ = None;

	def __init__(self):
		if DialogueEditor._ != None:
			return None;
		DialogueEditor._ = self;

		self.size = (1000, 600);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;
		
		self.nodes = asset_docs["dialogue"].entries;
		self.node = None;
	
	def node_selector(self):
		if imgui.begin_combo(f"Dialogue", self.node["name"] if not self.node is None else "None"):
			for node in self.nodes:
				selected = node == self.node;
				if imgui.selectable(node["name"], selected)[0]:
					self.node = node;
				if selected:
					imgui.set_item_default_focus();
			imgui.end_combo();

	def verify_node_integrity(self):
		if not "lines" in self.node or self.node["lines"] == None:
			self.node["lines"] = [];
		if not "edges" in self.node or self.node["edges"] == None:
			self.node["edges"] = [];
	
	def verify_edge_integrity(self, edge):
		if not "text" in edge:
			edge["text"] = "Maybe";
		if not "node" in edge or edge["node"] == None:
			edge["node"] = "";
		if not "proc" in edge or edge["proc"] == None:
			edge["proc"] = "";
	
		if "type" in edge:
			del edge["type"];
		if "link" in edge:
			del edge["link"];

	def render():
		if DialogueEditor._ == None:
			return;
		self = DialogueEditor._;

		if self.open:
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"Dialogue Editor", self.open, flags=self.window_flags);

			self.node_selector();

			if self.node != None:
				self.verify_node_integrity();

				if imgui.collapsing_header("Lines"):
					for idx in range(len(self.node["lines"])):
						_, self.node["lines"][idx] = imgui.input_text(str(idx), self.node["lines"][idx]);
					if imgui.button("New##line"):
						self.node["lines"].append("Hello, world!");

				if imgui.collapsing_header("Edges"):
					for (idx, edge) in enumerate(self.node["edges"]):
						self.verify_edge_integrity(edge);
						imgui.push_id(idx);
						if imgui.collapsing_header(f"{edge["text"]}####{idx}"):
							_, edge["text"] = imgui.input_text("Text", edge["text"]);
							if imgui.begin_combo("Node", edge["node"]):
								for node in self.nodes:
									selected = node["name"] == edge["node"];
									if imgui.selectable(node["name"], selected)[0]:
										edge["node"] = node["name"];
									if selected:
										imgui.set_item_default_focus();
								imgui.end_combo();
							_, edge["proc"] = imgui.input_text("Proc", edge["proc"]);
						imgui.pop_id();
					if imgui.button("New##edge"):
						self.node["edges"].append({});

			self.size = imgui.get_window_size();
			imgui.end();
		
		if not self.open:
			DialogueEditor._ = None;


#########################################################
## EDITOR GUI

splash_img = Image.open("editor/splash.png");
splash_tex = make_texture(splash_img.tobytes(), splash_img.width, splash_img.height);

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

input_states = {};

def track_inputs():
	for key in range(glfw.KEY_SPACE, glfw.KEY_LAST+1):
		if not key in input_states:
			input_states[key] = [glfw.get_key(handle, key), False];
		else:
			input_states[key] = [glfw.get_key(handle, key), input_states[key][0]];

def is_held(key):
	return input_states[key][0];

def is_pressed(key):
	return input_states[key][0] and not input_states[key][1];

while not glfw.window_should_close(handle):
	time_last = time;
	time = glfw.get_time();
	delta_time = time - time_last;

	glfw.poll_events();
	impl.process_inputs();

	track_inputs();

	if is_held(glfw.KEY_LEFT_SUPER) and is_pressed(glfw.KEY_S):
		for doc in asset_docs.values():
			print(f"Saving {doc.name}!");
			doc.save();

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
				for doc in asset_docs.values():
					if imgui.menu_item_simple(str(doc.path), selected = document != None and doc.name == document.name):
						document = doc;
				imgui.end_menu();
			if imgui.menu_item_simple("Save"):
				for doc in asset_docs.values():
					print(f"Saving {doc.name}!");
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
				if imgui.menu_item_simple("Rank"):
					all_keys = document.schema.keys();
					rank_keys = [k for k in all_keys if document.schema.is_ranked(k)];
					if len(rank_keys) > 0:
						document.entries.sort(key = lambda n: n[rank_keys[0]]);
				imgui.end_menu();
			if imgui.menu_item_simple("New"):
				document.spawn_new_entry();
			imgui.end_menu();
		
		if imgui.begin_menu("Tools"):
			if imgui.menu_item_simple("Prop Viewer"):
				PropViewer();
			if imgui.menu_item_simple("Animation Viewer"):
				AnimationViewer();
			if imgui.menu_item_simple("Theme Editor"):
				ThemeEditor();	
			if imgui.menu_item_simple("Mesh2D Editor"):
				Mesh2DEditor();
			if imgui.menu_item_simple("Item Reformer"):
				ItemReformer();	
			if imgui.menu_item_simple("Dialogue Editor"):
				DialogueEditor();
			imgui.end_menu();

		imgui.end_main_menu_bar();
	
	if document != None:
		document.refresh();
		DocumentRenderer.render(document);
	else:
		imgui.set_scroll_x(0);
		imgui.set_scroll_y(0);
		imgui.image(splash_tex, (splash_img.width, splash_img.height));
	if PropViewer._ != None:
		PropViewer.render();
	if AnimationViewer._ != None:
		AnimationViewer.render();
	if ThemeEditor._ != None:
		ThemeEditor.render();
	if Mesh2DEditor._ != None:
		Mesh2DEditor.render();
	if ItemReformer._ != None:
		ItemReformer.render();
	if DialogueEditor._ != None:
		DialogueEditor.render();

	imgui.end();
	imgui.render();
	impl.render(imgui.get_draw_data());
	imgui.end_frame();

	glfw.swap_buffers(handle);

impl.shutdown();
imgui.destroy_context();

glfw.destroy_window(handle);
glfw.terminate();

