#!/usr/bin/env python3

import configparser;
import os;
import OpenGL;
OpenGL.FULL_LOGGING = True;
from OpenGL.GL import *;
import glfw;
from imgui_bundle import imgui;
from imgui_bundle.python_backends.glfw_backend import GlfwRenderer;
from pathlib import Path;
from PIL import Image;
from playsound3 import playsound;
from stat import *;
import wave;
import numpy as np;
from collections import OrderedDict;

import ee_types;
from ee_cowtools import *;
from ee_canvas import Canvas;
from ee_assets import *;

from ee_scene_editor import SceneEditor;
from ee_sprites import SpriteBank;
from ee_input import InputManager;

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

AssetManager.initialize(["sprites", "sounds", "meshes", "data"]);
SpriteBank.initialize();
InputManager.initialize(handle, impl);

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
			listings = [s["name"] for s in AssetManager.get_assets("sprite")];
			listings.sort();
			
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"Sprite Explorer", self.open, flags=self.window_flags);
			for item in listings:
				Preview.thumbnail(item);
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
		for asset_type in AssetManager.types():
			self.entries[asset_type] = {};
	
	def init(self, asset_type, name):
		try:
			asset = AssetManager.search(asset_type, name);
			match asset_type:
				case "sprite":
					self.entries[asset_type][name] = PreviewSprite(asset);
					self.entries[asset_type][asset["path"]] = self.entries[asset_type][name];
				case "sound":
					self.entries[asset_type][name] = PreviewSound(asset);
					self.entries[asset_type][asset["path"]] = self.entries[asset_type][name];
				case _:
					self.entries[asset_type][name] = None;
		except:
			self.entries[asset_type][name] = None;
		return self.entries[asset_type][name];
	
	def get(self, asset_type, name):
		if not name in self.entries[asset_type]:
			self.init(asset_type, name);
		return self.entries[asset_type][name];
		
preview_bank = PreviewBank();
for asset_type in AssetManager.types():
	for asset in AssetManager.get_assets(asset_type):
		preview_bank.init(asset_type, asset["name"]);

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
	
	def render(name):
		if preview_bank.get("sprite", name) != None:
			Preview.render_sprite(name);
		elif preview_bank.get("sound", name) != None:
			Preview.render_sound(name);

	def thumbnail(name):
		if preview_bank.get("sprite", name) != None:
			Preview.thumbnail_sprite(name);


#########################################################
## DOCUMENT GUI

class DocumentRenderer:
	_search_term = "";
	_topmost_id = None;
	
	def _render_node(title, T, node, identifier):
		identifier = str(identifier)+title;

		if isinstance(T, ee_types.Object):
			imgui.push_id(identifier);
			if imgui.tree_node(title):
				for e in T.elements:
					if e.name in node:
						if not e.get_attribute("read-only"):
							node[e.name] = DocumentRenderer._render_node(e.name, e.T, node[e.name], id(node));
				imgui.tree_pop();
			imgui.pop_id();
			DocumentRenderer._topmost_id = identifier;
			return node;
		
		elif isinstance(T, ee_types.List):
			imgui.push_id(identifier);
			if imgui.tree_node(title):
				for i in range(len(node)):
					imgui.push_id(i);
					node[i] = DocumentRenderer._render_node(f"{title}[{i}]", T.T, node[i], id(node));
					imgui.pop_id();
				imgui.tree_pop();
			imgui.pop_id();
			return node;
		
		elif isinstance(T, ee_types.Asset):
			if T.name in AssetManager.types():
				Preview.render(node);

				imgui.text(title);
				imgui.same_line();
				
				match T.name:
					case "sprite":
						_, result = imgui.input_text(f"##{identifier}", node);
						imgui.same_line();
						if imgui.button(f"...##{identifier}"):
							SpriteExplorer(identifier);
						if SpriteExplorer.is_active(identifier):
							SpriteExplorer.render();
							ready, output = SpriteExplorer.harvest();
							result = output if ready else result;
						return result;
					case _:
						result = node;
						if imgui.begin_combo(f"##{identifier}", node):
							assets = AssetManager.get_assets(T.name);
							for asset in assets:
								selected = result == asset["name"];
								if imgui.selectable(asset["name"], selected)[0]:
									result = asset["name"];
								if selected:
									imgui.set_item_default_focus();
							imgui.end_combo();
						return result;
			else:
				imgui.text(title);
				imgui.same_line();
				_, result = imgui.input_text(f"##{identifier}", str(node));
				return result;
		
		elif isinstance(T, ee_types.File):
			Preview.render(node);

			imgui.text(title);
			imgui.same_line();

			_, result = imgui.input_text(f"##{identifier}", str(node));
			imgui.same_line();
			if imgui.button(f"...##{identifier}"):
				FileExplorer(identifier, AssetManager.active_document.directory, T.pattern);
			if FileExplorer.is_active(identifier):
				FileExplorer.render();
				ready, output = FileExplorer.harvest();
				result = output if ready else result;
			return result;

		elif isinstance(T, ee_types.Enum):
			imgui.text(title);
			imgui.same_line();

			result = node;
			if imgui.begin_combo(f"##{identifier}", result):
				for item in T.values:
					selected = result == item;
					if imgui.selectable(item, selected)[0]:
						result = item;
					if selected:
						imgui.set_item_default_focus();	
				imgui.end_combo();
			return result;

		elif isinstance(T, ee_types.Primitive):
			imgui.text(title);
			imgui.same_line();

			match type(T):
				case ee_types.Int:
					_, result = imgui.input_int(f"##{identifier}", node);
					return result;
				case ee_types.Float:
					_, result = imgui.input_float(f"##{identifier}", node);
					return result;
				case ee_types.Bool:
					return imgui.checkbox(f"##{identifier}", node)[1];
				case ee_types.String:
					_, result = imgui.input_text(f"##{identifier}", node);
					return result;

	def _context_popup(doc : AssetDocument, idx):
		if imgui.begin_popup_context_item(DocumentRenderer._topmost_id):
			if imgui.menu_item_simple("Delete"):
				doc.delete_entry(idx);
			if imgui.menu_item_simple("Duplicate"):
				doc.duplicate_entry(idx);
			imgui.end_popup();
	
	def render(doc : AssetDocument):
		_, DocumentRenderer._search_term = imgui.input_text("Search", DocumentRenderer._search_term);
		subset = list(filter(lambda e: len(DocumentRenderer._search_term) == 0 or DocumentRenderer._search_term in e["name"], doc.instances));

		for (idx, entry) in enumerate(doc.instances):
			if entry in subset:
				root = doc.typist.root();
				name = DocumentHelper.get_name(doc, idx);
				number = DocumentHelper.get_number(doc, idx);
				DocumentRenderer._render_node(f"{name} {number}####{id(entry)}", root.T, entry, id(entry));
				DocumentRenderer._context_popup(doc, idx);


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

		self.props = list(filter(lambda x: x["type"] == "prop", AssetManager.get_assets("item")));
		self.parent_prop = self.props[0];
		self.child_prop = self.parent_prop;

	def __draw_prop(self, x, y, prop):
		shape = prop["prop_data"]["shape"];
		tlx = x - (shape[0] * 16) // 2;
		tly = y - (shape[1] * 16);
		self.canvas.draw_flags = Canvas.DrawFlags(0);
		for sy in range(shape[1]):
			for sx in range(shape[0]):
				self.canvas.draw_rect_old(tlx + sx * 16, tly + sy * 16, 16, 16, (255, 0, 0));

		self.canvas.draw_flags = Canvas.DrawFlags.CENTER_X | Canvas.DrawFlags.BOTTOM;
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

		self.sprites = AssetManager.get_assets("sprite");
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
			self.canvas.draw_flags = Canvas.DrawFlags.CENTER_X | Canvas.DrawFlags.CENTER_Y;
			self.canvas.draw_image(draw_x, draw_y, preview.frame_images[self.frame]);
			if self.show_AABB:
				self.canvas.draw_rect_old(draw_x, draw_y, preview.width, preview.height/preview.frame_count, (255, 0, 0));
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
			
			_, self.show_AABB = imgui.checkbox("Show AABB", self.show_AABB);

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

		self.themes = AssetManager.get_assets("theme");
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
			
			if self.theme["tile_wall"] or self.theme["tile_floor"]:
				_, self.grid = imgui.checkbox("Show grid", self.grid);
			
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
						self.wall_canvas.draw_line(0, y*16, self.wall_canvas.width-1, y*16, (255, 255, 255));
					for x in range(1, 15):
						self.wall_canvas.draw_line(x*16, 0, x*16, self.wall_canvas.height-1, (255, 255, 255));

				mouse_pos = imgui_io.mouse_pos;
				brush_pos = mouse_pos - canvas_pos;
				if brush_pos.x >= 0 and brush_pos.x < (15 * 16) and brush_pos.y >= 0 and brush_pos.y < (6 * 16):
					self.wall_cursor = (int(brush_pos.x // 16), int(brush_pos.y // 16));
					if imgui.is_mouse_down(0):
						idx = self.wall_cursor[1] * 15 + self.wall_cursor[0];
						self.theme["wall_map"][idx] = self.wall_brush;
					self.wall_canvas.draw_rect_old(self.wall_cursor[0] * 16, self.wall_cursor[1] * 16, 16, 16, (255, 0, 0));
				
				window_rect = self.theme['window_rect']
				if len(window_rect) == 4:
					self.wall_canvas.draw_rect_old(window_rect[0], window_rect[1], window_rect[2], window_rect[3], (0, 0, 255));

				self.wall_canvas.render(1);

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
					self.wall_canvas.draw_rect_old(window_rect[0], window_rect[1], window_rect[2], window_rect[3], (0, 0, 255));
				
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
						self.floor_canvas.draw_line(0, y*16, self.floor_canvas.width-1, y*16, (255, 255, 255));
					for x in range(1, 15):
						self.floor_canvas.draw_line(x*16, 0, x*16, self.floor_canvas.height-1, (255, 255, 255));
				
				mouse_pos = imgui_io.mouse_pos;
				brush_pos = mouse_pos - canvas_pos;
				if brush_pos.x >= 0 and brush_pos.x < (15 * 16) and brush_pos.y >= 0 and brush_pos.y < (14 * 16):
					self.floor_cursor = (int(brush_pos.x // 16), int(brush_pos.y // 16));
					if imgui.is_mouse_down(0):
						idx = self.floor_cursor[1] * 15 + self.floor_cursor[0];
						self.theme["floor_map"][idx] = self.floor_brush;
					self.floor_canvas.draw_rect_old(self.floor_cursor[0] * 16, self.floor_cursor[1] * 16, 16, 16, (255, 0, 0));

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
	
		self.mesh_pool = AssetManager.get_assets("mesh2d");
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
				self.canvas.draw_rect_old(0, 0, self.canvas.width, self.canvas.height, (128, 128, 128));
			
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
		
		self.items = AssetManager.get_assets("item");
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
		
		self.nodes = AssetManager.get_assets("dialogue");
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

	def render():
		if DialogueEditor._ == None:
			return;
		self = DialogueEditor._;

		if self.open:
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"Dialogue Editor", self.open, flags=self.window_flags);

			self.node_selector();

			if self.node != None:
				if imgui.collapsing_header("Lines"):
					for idx in range(len(self.node["lines"])):
						_, self.node["lines"][idx] = imgui.input_text(str(idx), self.node["lines"][idx]);
					if imgui.button("New##line"):
						self.node["lines"].append("Hello, world!");

				if imgui.collapsing_header("Edges"):
					for (idx, edge) in enumerate(self.node["edges"]):
						imgui.push_id(idx);
						if imgui.tree_node(f"{edge["text"]}####{idx}"):
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
							imgui.tree_pop();
						imgui.pop_id();
					if imgui.button("New##edge"):
						new = asset_docs["dialogue"].type_helper.prototype("/edges", True);
						self.node["edges"].append(new);

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

while not glfw.window_should_close(handle):
	time_last = time;
	time = glfw.get_time();
	delta_time = time - time_last;

	glfw.poll_events();
	impl.process_inputs();

	InputManager.update();

	if InputManager.is_held(glfw.KEY_LEFT_SUPER) and InputManager.is_pressed(glfw.KEY_S):
		for document in AssetManager.documents:
			print(f"Saving {document.path}!");
			document.save();
	
	if AssetManager.active_document != None:
		AssetManager.active_document.refresh();

	glEnable(GL_PROGRAM_POINT_SIZE);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	imgui.new_frame();

	imgui.set_next_window_pos((0, 0));
	imgui.set_next_window_size((window_width, window_height));
	imgui.begin("Editor", flags=window_flags | (splash_flags if AssetManager.active_document == None else 0));

	if imgui.begin_main_menu_bar():
		if imgui.begin_menu("File"):
			if imgui.begin_menu("Open"):
				for document in AssetManager.documents:
					selected = document == AssetManager.active_document;
					if imgui.menu_item_simple(str(document.path), selected = selected):
						AssetManager.active_document = document;
				imgui.end_menu();
			if imgui.menu_item_simple("Save"):
				for document in AssetManager.documents:
					print(f"Saving {document.path}!");
					document.save();
			if imgui.menu_item_simple("Close", enabled=AssetManager.active_document != None):
				AssetManager.active_document = None;
			imgui.end_menu();

		if imgui.begin_menu("Assets", enabled=AssetManager.active_document != None):
			if imgui.begin_menu("Sort", enabled=AssetManager.active_document != None):
				if imgui.menu_item_simple("Name"):
					DocumentHelper.sort_by_name(AssetManager.active_document);
				if imgui.menu_item_simple("Number"):
					DocumentHelper.sort_by_number(AssetManager.active_document);
				if imgui.menu_item_simple("Rank"):
					DocumentHelper.sort_by_rank(AssetManager.active_document);
				imgui.end_menu();
			if imgui.menu_item_simple("New"):
				AssetManager.active_document.spawn_entry();
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
			if imgui.menu_item_simple("Scene Editor"):
				SceneEditor();
			imgui.end_menu();
		imgui.end_main_menu_bar();
	
	if AssetManager.active_document == None:
		imgui.set_scroll_x(0);
		imgui.set_scroll_y(0);
		imgui.image(splash_tex, (splash_img.width, splash_img.height));
	else:
		DocumentRenderer.render(AssetManager.active_document);
	
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
	if SceneEditor._ != None:
		SceneEditor.render();

	imgui.end();
	imgui.render();
	impl.render(imgui.get_draw_data());
	imgui.end_frame();

	glfw.swap_buffers(handle);

impl.shutdown();
imgui.destroy_context();

glfw.destroy_window(handle);
glfw.terminate();

