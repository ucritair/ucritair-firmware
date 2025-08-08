from imgui_bundle import imgui;
import glfw;

from ee_cowtools import *;
from ee_canvas import Canvas;
from ee_assets import *;
from ee_sprites import SpriteBank, EditorSprite;
from ee_input import InputManager;

#########################################################
## SCENE EDITOR

class SpawnHelper:
	def spawn_layer():
		return [];

	def spawn_prop(prop, position):
		x, y = position;
		return {
			"prop" : prop,
			"position" : [x, y],
			"variant" : -1
		};

class PropHelper:
	def get_prop_asset(self):
		return AssetManager.search("prop", self["prop"]);

	def get_editor_sprite(self) -> EditorSprite:
		prop_asset = PropHelper.get_prop_asset(self);
		return SpriteBank.get(prop_asset["sprite"]);

	def get_aabb(self):
		esprite = PropHelper.get_editor_sprite(self);
		x0, y0 = self["position"];
		w, h = esprite.frame_width, esprite.frame_height;
		return [x0, y0, x0+w-1, y0+h-1];

class CanvasHelper:
	def transform_point(canvas, point):
		x, y = point;
		return x + canvas.width/2, y + canvas.height/2;

	def transform_aabb(canvas, aabb):
		x0, y0, x1, y1 = aabb;
		x0, y0 = CanvasHelper.transform_point(canvas, [x0, y0]);
		x1, y1 = CanvasHelper.transform_point(canvas, [x1, y1]);
		return x0, y0, x1, y1;

class FloorPainter:
	def __init__(self, parent, palette):
		self.parent = parent;

		self.palette = palette;
		self.palette_idx = 0;

		self.size = (256, 512);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);

		self.open = True;
	
	def spatial_collision_pass(self, prop):
		layer = self.parent.active_layer;
		trash = [];
		a = PropHelper.get_aabb(prop);
		for i in range(len(layer)):
			b = PropHelper.get_aabb(layer[i]);
			if aabb_equals(a, b):
				return False;
		return True;
	
	def io(self):
		if not self.parent.cursor_in_bounds:
			return;
	
		if InputManager.is_held(glfw.MOUSE_BUTTON_LEFT):
			dupe = copy.deepcopy(self.palette);
			dupe["position"] = self.parent.tile_cursor;
			dupe["variant"] = self.palette_idx;
			if self.spatial_collision_pass(dupe):
				self.parent.active_layer.append(dupe);
	
	def render(self):
		palette_sprite = PropHelper.get_editor_sprite(self.palette);

		imgui.set_next_window_size(self.size);
		if self.open:
			imgui.push_id(str(id(self.palette)));
			_, self.open = imgui.begin("Floor Painter", self.open, flags=self.window_flags);

			for i in range(palette_sprite.frame_count):
				imgui.push_id(str(i));
				frame = palette_sprite.frame_textures[i];

				tint = (0.5, 0.5, 0.5, 1) if i == self.palette_idx else (1, 1, 1, 1);
				if imgui.image_button(f"##{i}", frame, (32, 32), tint_col=tint):
					self.palette_idx = i;

				imgui.pop_id();
			self.size = imgui.get_window_size();
			imgui.end();
			imgui.pop_id();


class SceneEditor:
	_ = None;

	def __init__(self):
		if SceneEditor._ != None:
			return None;
		SceneEditor._ = self;

		self.size = (1024, 680);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;

		self.canvas_size = (1024, 1024);
		self.canvas = Canvas(self.canvas_size[0], self.canvas_size[1]);
		self.tile_size = 16;

		self.floor_painter = None;

		self.scene = None;

		self.canvas_cursor = None;
		self.world_cursor = None;
		self.tile_cursor = None;
		self.selection_delta = None;
		self.cursor_in_bounds = False;

		self.selection = None;
		self.highlight = None;
		self.active_layer = None;
		self.copied = None;
		self.trash = [];
	
		self.show_viewport = True;
		self.show_axes = True;
		self.show_gizmos = False;
		self.snap = True;
	
	def is_scene_loaded(self):
		return self.scene in AssetManager.get_assets("scene");

	def y_sort_pass(self):
		for layer in self.scene["layers"]:
				layer.sort(key = lambda p: p["position"][1] + AssetManager.search("sprite", AssetManager.search("prop", p["prop"])["sprite"])["height"]);

	def hygiene_pass(self):	
		if self.is_scene_loaded():
			for thing in self.trash:
				for layer in self.scene["layers"]:
					if thing in layer:
						layer.remove(thing);
			self.trash = [];
			
			for layer in self.scene["layers"]:
				for prop in layer:
					if AssetManager.search("prop", prop["prop"]) == None:
						prop["prop"] = "null_prop";
					enforce_key(prop, "variant", -1);
					if not PropHelper.get_prop_asset(prop)["palette"]:
						prop["variant"] = -1;
	
			self.y_sort_pass();

		if self.floor_painter != None and not self.floor_painter.open:
			self.floor_painter = None;
	
	def snap_position(self, xy):
		x, y = xy;
		return [(x // self.tile_size) * self.tile_size, (y // self.tile_size) * self.tile_size];
	
	def cursor_io(self, canvas_pos):
		cursor = InputManager.get_imgui_cursor();
		x, y = cursor - canvas_pos;
		if x < 0 or x >= self.canvas_size[0] or y < 0 or y >= self.canvas_size[1]:
			self.cursor_in_bounds = False;
		else:
			self.cursor_in_bounds = True;
		x, y = clamp(x, 0, self.canvas_size[0]), clamp(y, 0, self.canvas_size[1]);
		self.canvas_cursor = int(x), int(y);
		x, y = x - self.canvas.width/2, y - self.canvas.height/2;
		self.world_cursor = int(x), int(y);
		self.tile_cursor = self.snap_position(self.world_cursor);
	
	def copy_prop(self, prop):
		self.copied = prop;
	
	def paste_prop(self, layer, position):
		dupe = copy.deepcopy(self.copied);
		dupe["position"] = position;
		layer.append(dupe);
	
	def selection_io(self):
		if not self.cursor_in_bounds:
			return;
	
		if InputManager.is_pressed(glfw.MOUSE_BUTTON_LEFT):
			self.selection = None;
			self.highlight = None;
			self.active_layer = None;
			for idx in range(len(self.scene["layers"])):
				layer = self.scene["layers"][len(self.scene["layers"])-idx-1];
				for prop in layer:
					if aabb_point_intersect(PropHelper.get_aabb(prop), self.world_cursor):
						self.selection = prop;
						self.highlight = prop;
						self.active_layer = layer;
						x0, y0, x1, y1 = PropHelper.get_aabb(self.selection);
						cx, cy = self.world_cursor;
						dx, dy = cx - x0, cy - y0;
						self.selection_delta = (dx, dy);
						return;
		elif InputManager.is_released(glfw.MOUSE_BUTTON_LEFT):
			self.selection = None;
	
		if self.selection != None:
			if InputManager.is_held(glfw.MOUSE_BUTTON_LEFT):
				x0, y0, x1, y1 = PropHelper.get_aabb(self.selection);
				cx, cy = self.world_cursor;
				frame_dx, frame_dy = cx - x0, cy - y0;
				dx, dy = self.selection_delta;
				ddx, ddy = frame_dx - dx, frame_dy - dy;
				self.selection["position"] = [int(x0+ddx), int(y0+ddy)];
				if self.snap:
					x, y = self.selection["position"];
					snap_x, snap_y = round(x / self.tile_size) * self.tile_size, round(y / self.tile_size) * self.tile_size;
					self.selection["position"] = [snap_x, snap_y];
	
		if InputManager.is_held(glfw.KEY_LEFT_SUPER) and InputManager.is_pressed(glfw.KEY_C):
			self.copy_prop(self.highlight);
		if InputManager.is_held(glfw.KEY_LEFT_SUPER) and InputManager.is_pressed(glfw.KEY_V):
			if self.active_layer != None and self.copied != None:
				self.paste_prop(self.active_layer, self.tile_cursor);
	
		if InputManager.is_pressed(glfw.KEY_F) and self.highlight != None and PropHelper.get_prop_asset(self.highlight)["palette"]:
			self.floor_painter = FloorPainter(self, self.highlight);
	
		if InputManager.is_pressed(glfw.KEY_BACKSPACE) and self.highlight != None:
			self.trash.append(self.highlight);
	
	def gui_asset_combo(self, identifier, asset_type, value):
		result = value;
		if imgui.begin_combo(f"##{identifier}", result if result != None else "None"):
			assets = AssetManager.get_assets(asset_type);
			for asset in assets:
				selected = result != None and result == asset["name"];
				if imgui.selectable(asset["name"], selected)[0]:
					result = asset;
				if selected:
					imgui.set_item_default_focus();
			imgui.end_combo();
		return result;
	
	def gui_asset_name_combo(self, identifier, asset_type, value):
		result = str(value);
		if imgui.begin_combo(f"##{identifier}", result):
			assets = AssetManager.get_assets(asset_type);
			for asset in assets:
				selected = result == asset["name"];
				if imgui.selectable(asset["name"], selected)[0]:
					result = asset["name"];
				if selected:
					imgui.set_item_default_focus();
			imgui.end_combo();
		return result;

	def gui_aabb_input(self, aabb):
		x0, y0, x1, y1 = aabb;
		w, h = x1-x0, y1-y0;
		x0, y0 = imgui.input_int2("XY", [int(x0), int(y0)])[1];
		w, h = imgui.input_int2("WH", [int(w), int(h)])[1];
		return x0, y0, x0+w, y0+h;
	
	def gui_draw_prop(self, prop):
		imgui.set_next_item_open(prop == self.highlight);
		if imgui.tree_node(f"{prop["prop"]} ({id(prop)})####({id(prop)})"):
			self.highlight = prop;
			prop["prop"] = self.gui_asset_name_combo(id(prop), "prop", prop["prop"]);
			if PropHelper.get_prop_asset(prop)["palette"]:
				_, prop["variant"] = imgui.slider_int("Variant", prop["variant"], 0, PropHelper.get_editor_sprite(prop).frame_count-1);
			if imgui.button("-"):
				self.trash.append(prop);
			imgui.tree_pop();
	
	def swap_layers(self, i, j):
		if i < 0 or j < 0:
			return;
		if i >= len(self.scene["layers"]) or j >= len(self.scene["layers"]):
			return;
		self.scene["layers"][i], self.scene["layers"][j] = self.scene["layers"][j], self.scene["layers"][i];
	
	def gui_draw_scene(self):
		if imgui.collapsing_header(f"Layers"):
			for (idx, layer) in enumerate(self.scene["layers"]):
				imgui.push_id(str(idx));
				imgui.set_next_item_open(self.active_layer == layer);
				if imgui.tree_node(f"Layer {idx}"):
					if self.active_layer == None:
						self.active_layer = layer;
					for prop in layer:
							self.gui_draw_prop(prop);
					if imgui.button("+"):
						prop = SpawnHelper.spawn_prop("null_prop", [0, 0]);
						layer.append(prop);
					if imgui.button("Up"):
						self.swap_layers(idx, idx-1);
					if imgui.button("Down"):
						self.swap_layers(idx, idx+1);
					imgui.tree_pop();
				imgui.pop_id();
			if imgui.button("+"):
				self.scene["layers"].append(SpawnHelper.spawn_layer());
	
	def canvas_draw_prop(self, prop, show_sprite=True, show_aabb=False, show_blockers=False, show_triggers=False, show_name=False):
		prop_asset = PropHelper.get_prop_asset(prop);
		esprite = PropHelper.get_editor_sprite(prop);
	
		if show_blockers:
			for blocker in prop_asset["blockers"]:
				aabb = move_aabb(blocker, prop["position"]);
				aabb = CanvasHelper.transform_aabb(self.canvas, aabb);
				self.canvas.draw_aabb(aabb, (255, 0, 0));
	
		if show_triggers:
			for trigger in prop_asset["triggers"]:
				aabb = move_aabb(trigger["aabb"], prop["position"]);
				aabb = CanvasHelper.transform_aabb(self.canvas, aabb);
				self.canvas.draw_aabb(aabb, (0, 255, 0));

				x0, y0, x1, y1 = aabb;
				w, h = x1-x0, y1-y0;
				cx, cy = x0+w/2, y0+h/2;
				tx, ty = trigger["direction"];
				self.canvas.draw_line(cx, cy, cx+tx*16, cy+ty*16, (0, 255, 0));
		
		if show_sprite:
			frame = esprite.frame_images[0 if prop["variant"] == -1 else prop["variant"]];
			x, y = CanvasHelper.transform_point(self.canvas, prop["position"]);
			self.canvas.draw_image(x, y, frame);
		
		if show_aabb:
			aabb = PropHelper.get_aabb(prop);
			aabb = CanvasHelper.transform_aabb(self.canvas, aabb);
			self.canvas.draw_aabb(aabb, (255, 255, 255));
	
		if show_name:
			aabb = PropHelper.get_aabb(prop);
			aabb = CanvasHelper.transform_aabb(self.canvas, aabb);
			x0, y0, x1, y1 = aabb;
			self.canvas.draw_text((x1 + 2, y0), prop_asset["name"], 10, (255, 255, 255));
	
	def canvas_draw_layers(self):
		for layer in self.scene["layers"]:
			for prop in layer:
				self.canvas_draw_prop(prop, show_blockers=self.show_gizmos, show_triggers = self.show_gizmos);
	
	def canvas_draw_axes(self):
		if self.show_axes:
			x, y = self.canvas.width/2, self.canvas.height/2;
			self.canvas.draw_line(x, 0, x, self.canvas.height, (64, 64, 64));
			self.canvas.draw_line(0, y, self.canvas.width, y, (64, 64, 64));

	def canvas_draw_selection(self):
		if self.selection != None:
			self.canvas_draw_prop(self.selection, False, True, True, True, True);
		if self.highlight != None:
			self.canvas_draw_prop(self.highlight, False, True, False, False, True);

	def canvas_draw_viewport(self):
		if self.show_viewport:
			x0, y0 = self.canvas.width/2, self.canvas.height/2;
			x0, y0 = x0 - 120, y0 - 160;
			x1, y1 = x0+240-1, y0+320-1;
			self.canvas.draw_aabb((x0, y0, x1, y1), (0, 0, 255));

	def render():
		if SceneEditor._ == None:
			return;
		self = SceneEditor._;

		if self.open:
			self.hygiene_pass();

			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"Scene Canvas", self.open, flags=self.window_flags);

			if self.is_scene_loaded():

				imgui.set_next_window_size((256, 680));
				imgui.begin(f"Scene Data", self.open, flags=self.window_flags);
				self.gui_draw_scene();
				imgui.end();

				if self.floor_painter != None:
					self.floor_painter.io();
				else:
					self.selection_io();

				self.canvas.clear((128, 128, 128));
				self.canvas_draw_axes();
				self.canvas_draw_layers();
				self.canvas_draw_selection();
				self.canvas_draw_viewport();

				# Cursor IO has to come right before canvas draw
				self.cursor_io(imgui.get_cursor_screen_pos());
				self.canvas.render(1);
			
				_, self.show_axes = imgui.checkbox("Show axes", self.show_axes);
				imgui.same_line();
				_, self.show_viewport = imgui.checkbox("Show viewport", self.show_viewport);
				imgui.same_line();
				_, self.show_gizmos = imgui.checkbox("Show gizmos", self.show_gizmos);
				imgui.same_line();
				_, self.snap = imgui.checkbox("Snap", self.snap);
			else:
				self.scene = self.gui_asset_combo(id(self.scene), "scene", self.scene);
			
			self.size = imgui.get_window_size();
			imgui.end();
		
			if self.floor_painter != None:
				self.floor_painter.render();
		
		if not self.open:
			SceneEditor._ = None;