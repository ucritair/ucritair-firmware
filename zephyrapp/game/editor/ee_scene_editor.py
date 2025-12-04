from imgui_bundle import imgui;
import glfw;

from ee_cowtools import *;
from ee_canvas import Canvas, CanvasGrid, CanvasIO;
from ee_assets import *;
from ee_sprites import SpriteBank, EditorSprite, SpritePreview;
from ee_input import InputManager;

#########################################################
## SCENE EDITOR

class PropGrid:
	def __init__(self):
		self.size = (512, 512);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;
	
	def io(self):
		pass;
	
	def draw(self):
		props = AssetManager.get_assets("prop");
		row_size = 4;

		imgui.set_next_window_size(self.size);
		_, self.open = imgui.begin("Prop Grid", self.open, flags=self.window_flags);
		if self.open:
			for i in range(len(props)):
				imgui.begin_group();
				SpritePreview.draw_thumbnail(props[i]["sprite"], self.size[0]/row_size);
				imgui.text(props[i]["name"]);
				imgui.end_group();
				imgui.same_line();

				if i > 0 and i % row_size == 0 or i == len(props)-1:
					imgui.new_line();
			
			self.size = imgui.get_window_size();
			imgui.end();

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

	def get_canvas_aabb(parent, self):
		x0, y0 = parent.canvas_grid.untransform_point(self["position"]);
		sprite = PropHelper.get_editor_sprite(self);
		x1, y1 = x0 + sprite.frame_width-1, y0 + sprite.frame_height-1;
		aabb = x0, y0, x1, y1;
		return aabb;

class BackgroundPainter:
	def __init__(self, parent):
		self.parent = parent;

		self.palette = self.parent.scene["background"]["palette"];
		self.palette_idx = 0;

		self.size = (256, 512);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);

		self.open = True;
	
	def io(self):
		if not self.parent.cursor_in_bounds:
			return;
	
		if InputManager.is_pressed(glfw.MOUSE_BUTTON_LEFT):
			cursor = self.parent.canvas_io.cursor;
			cursor = self.parent.canvas_grid.transform_point(cursor);
			if InputManager.is_held(glfw.KEY_LEFT_SHIFT):
				trash = [];
				for (idx, tile) in enumerate(self.parent.scene["background"]["tiles"]):
					x, y = tile["position"];
					if x == cursor[0] and y == cursor[1]:
						trash.append(idx);
				for idx in trash:
					del self.parent.scene["background"]["tiles"][idx];
			else:
				self.parent.scene["background"]["tiles"].append(
					{
						"position" : [cursor[0], cursor[1]],
						"frame" : self.palette_idx
					}
				);
	
	def render(self):
		imgui.set_next_window_size(self.size);
		if self.open:
			imgui.push_id(str(id(self.palette)));
			_, self.open = imgui.begin("Floor Painter", self.open, flags=self.window_flags);
			if self.palette == None:
				self.palette = imgui_asset_selector(id(self.palette), "sprite", self.palette);
			else:
				palette_sprite = SpriteBank.get(self.palette);

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
	def __init__(self):
		self.canvas_size = (1024, 1024);
		self.tile_size = 16;
		self.canvas = Canvas(self.canvas_size[0], self.canvas_size[1]);
		self.canvas_io = CanvasIO(self.canvas);
		self.canvas_grid = CanvasGrid(
			self.canvas,
			((self.canvas_size[0]//self.tile_size)//2, (self.canvas_size[1]//self.tile_size)//2),
			self.tile_size
		);

		self.floor_painter = None;

		self.scene = None;

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
		self.show_bounds = True;
	
	def is_scene_loaded(self):
		return self.scene in AssetManager.get_assets("scene");

	def y_sort_pass(self):
		def y_key(p):
			x, y = self.canvas_grid.untransform_point(p["position"]);
			prop = AssetManager.search("prop", p["prop"]);
			sprite = AssetManager.search("sprite", prop["sprite"]);
			yf = y + sprite["height"];
			return yf;

		for layer in self.scene["layers"]:
				layer.sort(key = lambda p: y_key(p));

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
		x, y = self.canvas_io.cursor;
		if x < 0 or x >= self.canvas_size[0] or y < 0 or y >= self.canvas_size[1]:
			self.cursor_in_bounds = False;
		else:
			self.cursor_in_bounds = True;
	
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
					aabb = PropHelper.get_canvas_aabb(self, prop);
					if aabb_point_intersect(aabb, self.canvas_io.cursor):
						self.selection = prop;
						self.highlight = prop;
						self.active_layer = layer;

						x0, y0, x1, y1 = aabb;
						cx, cy = self.canvas_io.cursor;
						dx, dy = cx - x0, cy - y0;
						self.selection_delta = (dx, dy);
						return;
		elif InputManager.is_released(glfw.MOUSE_BUTTON_LEFT):
			self.selection = None;
	
		if self.selection != None:
			if InputManager.is_held(glfw.MOUSE_BUTTON_LEFT):
				aabb = PropHelper.get_canvas_aabb(self, self.selection);
				x0, y0, x1, y1 = aabb;
				cx, cy = self.canvas_io.cursor;
				cdx, cdy = cx - x0, cy - y0;
				sdx, sdy = self.selection_delta;
				self.selection["position"] = self.canvas_grid.transform_point((x0-sdx+cdx, y0-sdy+cdy));
	
		if InputManager.is_held(glfw.KEY_LEFT_SUPER) and InputManager.is_pressed(glfw.KEY_C):
			self.copy_prop(self.highlight);
		if InputManager.is_held(glfw.KEY_LEFT_SUPER) and InputManager.is_pressed(glfw.KEY_V):
			if self.active_layer != None and self.copied != None:
				self.paste_prop(self.active_layer, self.canvas_grid.transform_point(self.canvas_io.cursor));
	
		if InputManager.is_pressed(glfw.KEY_F):
			self.floor_painter = PropGrid();
	
	def gui_draw_prop(self, prop):
		imgui.set_next_item_open(prop == self.highlight);
		if imgui.tree_node(f"{prop["prop"]} ({id(prop)})####({id(prop)})"):
			self.highlight = prop;
			prop_asset = PropHelper.get_prop_asset(prop);
			prop_asset = imgui_asset_selector(id(prop), "prop", prop_asset);
			prop["prop"] = prop_asset["name"];
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
		if imgui.collapsing_header(f"Bounds"):
			imgui.push_item_width(128);
			self.scene["bounds"] = imgui_aabb_xyxy(id(self.scene), self.scene["bounds"]);
			imgui.pop_item_width();
		if imgui.collapsing_header(f"Layers"):
			imgui.push_item_width(512);
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
			imgui.pop_item_width();
			if imgui.button("+"):
				self.scene["layers"].append(SpawnHelper.spawn_layer());
	
	def canvas_draw_background(self):
		self.canvas.clear(tuple(self.scene["background"]["colour"]));
		if self.floor_painter != None:
			self.canvas_grid.draw_lines((128, 128, 128, 255));
		for tile in self.scene["background"]["tiles"]:
			x, y = self.canvas_grid.untransform_point(tile["position"]);
			sprite = SpriteBank.get(self.scene["background"]["palette"]);
			idx = tile["frame"];
			self.canvas.draw_image(x, y, sprite.frame_images[idx]);
	
	def canvas_draw_prop(self, prop, show_sprite=True, show_aabb=False, show_blockers=False, show_triggers=False, show_name=False):
		prop_asset = PropHelper.get_prop_asset(prop);
		esprite = PropHelper.get_editor_sprite(prop);
	
		if show_blockers:
			for blocker in prop_asset["blockers"]:
				aabb = move_aabb(blocker, prop["position"]);
				aabb = self.canvas_grid.untransform_aabb(aabb);
				self.canvas.draw_aabb(aabb, (255, 0, 0));
	
		if show_triggers:
			for trigger in prop_asset["triggers"]:
				aabb = move_aabb(trigger["aabb"], prop["position"]);
				aabb = self.canvas_grid.untransform_aabb(aabb);
				self.canvas.draw_aabb(aabb, (0, 255, 0));

				x0, y0, x1, y1 = aabb;
				w, h = x1-x0, y1-y0;
				cx, cy = x0+w/2, y0+h/2;
				tx, ty = trigger["direction"];
				self.canvas.draw_line(cx, cy, cx+tx*self.tile_size, cy+ty*self.tile_size, (0, 255, 0));
		
		if show_sprite:
			frame = esprite.frame_images[0 if prop["variant"] == -1 else prop["variant"]];
			x, y = self.canvas_grid.untransform_point(prop["position"]);
			self.canvas.draw_image(x, y, frame);
		
		if show_aabb:
			aabb = PropHelper.get_canvas_aabb(self, prop);
			self.canvas.draw_aabb(aabb, (255, 255, 255));
	
		if show_name:
			aabb = PropHelper.get_canvas_aabb(self, prop);
			x0, y0, x1, y1 = aabb;
			self.canvas.draw_text((x1 + 2, y0), prop_asset["name"], 10, (255, 255, 255));
	
	def canvas_draw_layers(self):
		for layer in self.scene["layers"]:
			for prop in layer:
				self.canvas_draw_prop(prop, show_blockers=self.show_gizmos, show_triggers = self.show_gizmos);
	
	def canvas_draw_axes(self):
		x, y = self.canvas.width/2, self.canvas.height/2;
		self.canvas.draw_line(x, 0, x, self.canvas.height, (64, 64, 64));
		self.canvas.draw_line(0, y, self.canvas.width, y, (64, 64, 64));

	def canvas_draw_selection(self):
		if self.selection != None:
			self.canvas_draw_prop(self.selection, False, True, True, True, True);
		if self.highlight != None:
			self.canvas_draw_prop(self.highlight, False, True, False, False, True);

	def canvas_draw_viewport(self):
		x0, y0 = self.canvas.width/2, self.canvas.height/2;
		x0, y0 = x0 - 120, y0 - 160;
		x1, y1 = x0+240-1, y0+320-1;
		self.canvas.draw_aabb((x0, y0, x1, y1), (0, 0, 255));
	
	def canvas_draw_bounds(self):
		aabb = self.canvas_grid.untransform_aabb(self.scene["bounds"]);
		self.canvas.draw_aabb(aabb, (255, 0, 0));

	def draw(self):
		self.hygiene_pass();

		if self.is_scene_loaded():
			if self.floor_painter != None:
				self.floor_painter.io();
			else:
				self.selection_io();

			imgui.begin_group();

			self.canvas_draw_background();
			if self.show_axes:
				self.canvas_draw_axes();
			
			self.canvas_draw_layers();
			self.canvas_draw_selection();

			if self.show_bounds:
				self.canvas_draw_bounds();
			if self.show_viewport:
				self.canvas_draw_viewport();

			# Cursor IO has to come right before canvas draw
			self.canvas.render();
			self.canvas_io.tick();
			self.cursor_io(imgui.get_cursor_screen_pos());
		
			_, self.show_axes = imgui.checkbox("Show axes", self.show_axes);
			imgui.same_line();
			_, self.show_viewport = imgui.checkbox("Show viewport", self.show_viewport);
			imgui.same_line();
			_, self.show_gizmos = imgui.checkbox("Show gizmos", self.show_gizmos);
			imgui.same_line();
			_, self.show_bounds = imgui.checkbox("Show bounds", self.show_bounds);
		
			imgui.end_group();
			imgui.same_line();
			imgui.begin_group();
		
			self.gui_draw_scene();
		
			imgui.end_group();
		else:
			self.scene = imgui_asset_selector(id(self.scene), "scene", self.scene);
			
		if self.floor_painter != None:
			self.floor_painter.draw();