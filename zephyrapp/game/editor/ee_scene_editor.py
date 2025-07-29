from imgui_bundle import imgui;
import glfw;

from ee_cowtools import *;
from ee_canvas import Canvas;
from ee_assets import *;
from ee_sprites import SpriteBank;
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
			"position" : [x, y]
		};

	def spawn_blocker(aabb):
		x0, y0, x1, y1 = aabb;
		return [x0, y0, x1, y1];

	def spawn_trigger(aabb, proc):
		x0, y0, x1, y1 = aabb;
		return {
			"aabb" : [x0, y0, x1, y1],
			"proc" : proc
		};

class PropHelper:
	def get_prop_asset(self):
		return AssetManager.search("prop", self["prop"]);

	def get_editor_sprite(self):
		prop_asset = PropHelper.get_prop_asset(self);
		return SpriteBank.get(prop_asset["sprite"]);

	def get_aabb(self):
		esprite = PropHelper.get_editor_sprite(self);
		x0, y0 = self["position"];
		w, h = esprite.frame_width, esprite.frame_height;
		return [x0, y0, x0+w-1, y0+h-1];

	def add_blocker(self):
		esprite = PropHelper.get_editor_sprite(self);
		w, h = esprite.frame_width, esprite.frame_height/2;
		x0, y0 = 0, 0;
		y0 = y0 + esprite.frame_height - h;

		prop_asset = PropHelper.get_prop_asset(self);
		blocker = SpawnHelper.spawn_blocker([x0, y0, x0+w-1, y0+h-1]);
		prop_asset["blockers"].append(blocker);

	def add_trigger(self):
		esprite = PropHelper.get_editor_sprite(self);
		w, h = esprite.frame_width, esprite.frame_height/2;
		x0, y0 = 0, 0;
		y0 = y0 + esprite.frame_height - h;

		prop_asset = PropHelper.get_prop_asset(self);
		trigger = SpawnHelper.spawn_trigger([x0, y0, x0+w-1, y0+h-1], "");
		prop_asset["triggers"].append(trigger);
		

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

		self.scene = None;
		
		self.show_viewport = True;
		self.show_axes = True;

		self.canvas_cursor = None;
		self.selection = None;
		self.highlight = None;
		self.selection_delta = None;
	
	def is_scene_loaded(self):
		return self.scene in AssetManager.get_assets("scene");
	
	def cursor_io(self, canvas_pos):
		cursor = InputManager.get_imgui_cursor();
		x, y = cursor - canvas_pos;
		self.canvas_cursor = int(x), int(y);
	
	def selection_io(self):
		if InputManager.is_pressed(glfw.MOUSE_BUTTON_LEFT):
			self.selection = None;
			self.highlight = None;
			for idx in range(len(self.scene["layers"])):
				layer = self.scene["layers"][len(self.scene["layers"])-idx-1];
				for prop in layer:
					if aabb_point_intersect(PropHelper.get_aabb(prop), self.canvas_cursor):
						self.selection = prop;
						self.highlight = prop;
						x0, y0, x1, y1 = PropHelper.get_aabb(self.selection);
						cx, cy = self.canvas_cursor;
						dx, dy = cx - x0, cy - y0;
						self.selection_delta = (dx, dy);
						return;
		elif InputManager.is_released(glfw.MOUSE_BUTTON_LEFT):
			self.selection = None;
	
		if self.selection != None:
			if InputManager.is_held(glfw.MOUSE_BUTTON_LEFT):
				x0, y0, x1, y1 = PropHelper.get_aabb(self.selection);
				cx, cy = self.canvas_cursor;
				frame_dx, frame_dy = cx - x0, cy - y0;
				dx, dy = self.selection_delta;
				ddx, ddy = frame_dx - dx, frame_dy - dy;
				self.selection["position"] = [int(x0+ddx), int(y0+ddy)];
	
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
		if imgui.tree_node(f"{prop["prop"]} ({id(prop)})####({id(prop)})"):
			prop["prop"] = self.gui_asset_name_combo(id(prop), "prop", prop["prop"]);
			_, prop["position"] = imgui.input_int2("XY", prop["position"]);

			prop_asset = PropHelper.get_prop_asset(prop);
			prop_asset["sprite"] = self.gui_asset_name_combo(id(prop_asset), "sprite", prop_asset["sprite"]);

			if imgui.tree_node("Blockers"):
				for blocker in prop_asset["blockers"]:
					imgui.push_id(str(id(blocker)));
					blocker = self.gui_aabb_input(blocker);
					imgui.separator();
					imgui.pop_id();
				if imgui.button("+"):
					PropHelper.add_blocker(prop);
				imgui.tree_pop();
			
			if imgui.tree_node("Triggers"):
				for trigger in prop_asset["triggers"]:
					imgui.push_id(str(id(trigger)));
					trigger.aabb = self.gui_aabb_input(trigger.aabb);
					imgui.separator();
					imgui.pop_id();
				if imgui.button("+"):
					PropHelper.add_trigger(prop);
				imgui.tree_pop();
			imgui.tree_pop();
	
	def gui_draw_scene(self):
		if imgui.collapsing_header(f"Layers"):
			for (idx, layer) in enumerate(self.scene["layers"]):
				imgui.push_id(str(idx));			
				if imgui.tree_node(f"Layer {idx}"):
					for prop in layer:
							self.gui_draw_prop(prop);
					if imgui.button("+"):
						prop = SpawnHelper.spawn_prop("null_prop", [0, 0]);
						layer.append(prop);
					imgui.tree_pop();
				imgui.pop_id();
			if imgui.button("+"):
				self.scene["layers"].append(SpawnHelper.spawn_layer());
	
	def canvas_draw_prop(self, prop, show_sprite=True, show_aabb=False, show_blockers=False, show_triggers=False, show_name=False):
		prop_asset = PropHelper.get_prop_asset(prop);
		esprite = PropHelper.get_editor_sprite(prop);

		if show_sprite:
			frame = esprite.frame_images[0];
			self.canvas.draw_image(prop["position"][0], prop["position"][1], frame);
		
		if show_aabb:
			aabb = PropHelper.get_aabb(prop);
			self.canvas.draw_aabb(aabb, (255, 255, 255));
	
		if show_blockers:
			for blocker in prop_asset["blockers"]:
				aabb = move_aabb(blocker, prop["position"]);
				self.canvas.draw_aabb(aabb, (255, 0, 0));
	
		if show_triggers:
			for trigger in prop_asset["triggers"]:
				aabb = move_aabb(trigger["aabb"], prop["position"]);
				self.canvas.draw_aabb(aabb, (0, 255, 0));
	
		if show_name:
			aabb = PropHelper.get_aabb(prop);
			x0, y0, x1, y1 = aabb;
			self.canvas.draw_text((x1 + 2, y0), prop_asset["name"], 10, (255, 255, 255));
	
	def canvas_draw_layers(self):
		for layer in self.scene["layers"]:
			for prop in layer:
				self.canvas_draw_prop(prop);
	
	def canvas_draw_axes(self):
		if self.show_axes:
			x, y = self.scene["origin"];
			x, y = x + self.canvas.width/2, y + self.canvas.height/2;
			self.canvas.draw_line(x, 0, x, self.canvas.height, (64, 64, 64));
			self.canvas.draw_line(0, y, self.canvas.width, y, (64, 64, 64));

	def canvas_draw_selection(self):
		if self.selection != None:
			self.canvas_draw_prop(self.selection, False, True, True, True, True);
		if self.highlight != None:
			self.canvas_draw_prop(self.highlight, False, True, False, False, True);

	def canvas_draw_viewport(self):
		if self.show_viewport:
			x0, y0 = self.scene["origin"];
			x0, y0 = x0 + self.canvas.width/2 - 120, y0 + self.canvas.height/2 - 160;
			x1, y1 = x0+240-1, y0+320-1;
			self.canvas.draw_aabb((x0, y0, x1, y1), (0, 0, 255));

	def render():
		if SceneEditor._ == None:
			return;
		self = SceneEditor._;

		if self.open:
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"Scene Editor", self.open, flags=self.window_flags);

			if self.is_scene_loaded():
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
			
				self.gui_draw_scene();
			else:
				self.scene = self.gui_asset_combo(id(self.scene), "scene", self.scene);
			
			self.size = imgui.get_window_size();
			imgui.end();
		
		if not self.open:
			SceneEditor._ = None;