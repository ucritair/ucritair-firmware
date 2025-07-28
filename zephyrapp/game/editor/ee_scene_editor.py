from imgui_bundle import imgui;
import glfw;

from ee_cowtools import *;
from ee_canvas import Canvas;
from ee_assets import *;
from ee_sprites import SpriteBank;
from ee_input import InputManager;

#########################################################
## SCENE EDITOR

class Trigger:
	def __init__(self, aabb, proc):
		self.aabb = aabb;
		self.proc = proc;

class Prop:
	def __init__(self, position, name, sprite):
		self.position = position;
		self.name = name;
		self.sprite = sprite;
		self.blockers = [];
		self.triggers = [];
	
	def update(self):
		self.esprite = SpriteBank.get(self.sprite);

class PropHelper:
	def get_aabb(self):
		x0, y0 = self.position;
		w, h = self.esprite.frame_width, self.esprite.frame_height;
		return [x0, y0, x0+w-1, y0+h-1];

	def add_blocker(self):
		w, h = self.esprite.frame_width, self.esprite.frame_height/2;
		x0, y0 = 0, 0;
		y0 = y0 + self.esprite.frame_height - h;
		self.blockers.append([x0, y0, x0+w-1, y0+h-1]);

	def add_trigger(self):
		w, h = self.esprite.frame_width, self.esprite.frame_height/2;
		x0, y0 = 0, 0;
		y0 = y0 + self.esprite.frame_height - h;
		trigger = Trigger([x0, y0, x0+w-1, y0+h-1], "");
		self.triggers.append(trigger);

class Layer:
	def __init__(self):
		self.props = [];

	def add_prop(self, prop):
		self.props.append(prop);

	def update(self):
		for prop in self.props:
			prop.update();

class Scene:
	def __init__(self):
		self.name = "";
		self.layers = [];
		self.origin = [0, 0];

	def update(self):
		for layer in self.layers:
			layer.update();

	def import_asset(self, asset):
		pass;

	def export_asset(self):
		pass;
		

class SceneEditor:
	_ = None;

	def __init__(self):
		if SceneEditor._ != None:
			return None;
		SceneEditor._ = self;

		self.size = (1000, 600);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;
		self.canvas = Canvas(self.size[0], self.size[1]);

		self.scenes = AssetManager.get_assets("scene");
		self.scene_asset = None;
		self.scene = Scene();
		
		self.show_viewport = True;
		self.show_axes = True;

		self.canvas_cursor = None;
		self.selection = None;
		self.highlight = None;
		self.selection_delta = None;
	
	def is_scene_loaded(self):
		return self.scene_asset in [s["name"] for s in self.scenes];
	
	def cursor_io(self, canvas_pos):
		cursor = InputManager.get_imgui_cursor();
		self.canvas_cursor = cursor - canvas_pos;
	
	def selection_io(self):
		if InputManager.is_pressed(glfw.MOUSE_BUTTON_LEFT):
			self.selection = None;
			self.highlight = None;
			for idx in range(len(self.scene.layers)):
				layer = self.scene.layers[len(self.scene.layers)-idx-1];
				for prop in layer.props:
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
				self.selection.position = x0+ddx, y0+ddy;
	
	def gui_asset_combo(self, identifier, asset_type, value):
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
		x0, y0 = imgui.input_float2("XY", [x0, y0])[1];
		w, h = imgui.input_float2("WH", [w, h])[1];
		return x0, y0, x0+w, y0+h;
	
	def gui_draw_prop(self, prop):
		if imgui.tree_node(f"{prop.name} ({id(prop)})####({id(prop)})"):
			_, prop.name = imgui.input_text("Name", prop.name);
			prop.sprite = self.gui_asset_combo(id(prop), "sprite", prop.sprite);
			_, prop.position = imgui.input_float2("XY", prop.position);

			if imgui.tree_node("Blockers"):
				for blocker in prop.blockers:
					imgui.push_id(str(id(blocker)));
					blocker = self.gui_aabb_input(blocker);
					imgui.separator();
					imgui.pop_id();
				if imgui.button("+"):
					PropHelper.add_blocker(prop);
				imgui.tree_pop();
			
			if imgui.tree_node("Triggers"):
				for trigger in prop.triggers:
					imgui.push_id(str(id(trigger)));
					trigger.aabb = self.gui_aabb_input(trigger.aabb);
					imgui.separator();
					imgui.pop_id();
				if imgui.button("+"):
					PropHelper.add_trigger(prop);
				imgui.tree_pop();
			imgui.tree_pop();
	
	def gui_draw_scene(self):
		_, self.show_axes = imgui.checkbox("Show axes", self.show_axes);
		imgui.same_line();
		_, self.show_viewport = imgui.checkbox("Show viewport", self.show_viewport);

		if imgui.collapsing_header(f"Layers"):
			for (idx, layer) in enumerate(self.scene.layers):
				imgui.push_id(str(idx));			
				if imgui.tree_node(f"Layer {idx}"):
					for prop in layer.props:
							self.gui_draw_prop(prop);
					if imgui.button("+"):
						prop = Prop([0, 0], f"Prop", "null_sprite");
						layer.add_prop(prop);
					imgui.tree_pop();
				imgui.pop_id();
			if imgui.button("+"):
				self.scene.layers.append(Layer());
	
	def canvas_draw_prop(self, prop, show_sprite=True, show_aabb=False, show_blockers=False, show_triggers=False, show_name=False):
		if show_sprite:
			self.canvas.draw_image(prop.position[0], prop.position[1], prop.esprite.frame_images[0]);
		
		if show_aabb:
			aabb = PropHelper.get_aabb(prop);
			x0, y0, x1, y1 = PropHelper.get_aabb(prop);
			self.canvas.draw_aabb(aabb, (255, 255, 255));
	
		if show_blockers:
			for blocker in prop.blockers:
				aabb = move_aabb(blocker, prop.position);
				self.canvas.draw_aabb(aabb, (255, 0, 0));
	
		if show_triggers:
			for trigger in prop.triggers:
				aabb = move_aabb(trigger.aabb, prop.position);
				self.canvas.draw_aabb(aabb, (0, 255, 0));
	
		if show_name:
			self.canvas.draw_text((x1 + 2, y0), prop.name, 10, (255, 255, 255));
	
	def canvas_draw_layers(self):
		for layer in self.scene.layers:
			for prop in layer.props:
				self.canvas_draw_prop(prop);
	
	def canvas_draw_axes(self):
		if self.show_axes:
			x, y = self.scene.origin;
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
			x0, y0 = self.scene.origin;
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

			self.scene.update();
			self.selection_io();

			if self.is_scene_loaded():
				imgui.text(self.scene_asset);
			self.canvas.clear((128, 128, 128));
			self.canvas_draw_axes();
			self.canvas_draw_layers();
			self.canvas_draw_selection();
			self.canvas_draw_viewport();
			# Cursor IO has to come right before canvas draw
			self.cursor_io(imgui.get_cursor_screen_pos());
			self.canvas.render(1);

			if self.is_scene_loaded():
				self.gui_draw_scene();
			else:
				self.scene_asset = self.gui_asset_combo(id(self.scene_asset), "scene", self.scene_asset);
			
			self.size = imgui.get_window_size();
			imgui.end();
		
		if not self.open:
			SceneEditor._ = None;