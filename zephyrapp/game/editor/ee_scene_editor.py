from imgui_bundle import imgui;
import glfw;

from ee_cowtools import *;
from ee_canvas import Canvas;
from ee_assets import *;
from ee_sprites import SpriteBank;
from ee_input import InputManager;

#########################################################
## SCENE EDITOR

class Prop:
	def __init__(self, position, name, sprite):
		self.position = position;
		self.name = name;
		self.sprite = sprite;
	
	def update(self):
		self.esprite = SpriteBank.get(self.sprite);

	def get_aabb(self):
		x0, y0 = self.position;
		w, h = self.esprite.frame_width, self.esprite.frame_height;
		return [x0, y0, x0+w-1, y0+h-1];

class Layer:
	def __init__(self):
		self.props = [];

	def add_prop(self, prop):
		self.props.append(prop);

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

		self.scenes = AssetManager.get_assets("scene");
		self.scene = None;

		self.origin = [self.size[0]//2-120, self.size[1]//2-160];
		self.layers = [];
		self.selected_prop = None;

		self.canvas = Canvas(self.size[0], self.size[1]);
		self.show_viewport = True;
		self.show_axes = True;

		self.canvas_cursor = None;
		self.selection_delta = None;
	
	def _cursor_io(self, canvas_pos):
		cursor = InputManager.get_imgui_cursor();
		self.canvas_cursor = cursor - canvas_pos;
	
	def _select_prop(self):
		if InputManager.is_pressed(glfw.MOUSE_BUTTON_LEFT):
			if self.selected_prop == None:
				for idx in range(len(self.layers)):
					layer = self.layers[len(self.layers)-idx-1];
					for prop in layer.props:
						if aabb_point_intersect(prop.get_aabb(), self.canvas_cursor):
							self.selected_prop = prop;
							x0, y0, x1, y1 = self.selected_prop.get_aabb();
							dx, dy = self.canvas_cursor[0] - x0, self.canvas_cursor[1] - y0;
							self.selection_delta = (dx, dy);
							return;
			else:
				self.selected_prop = None;
	
	def _move_selection(self):
		if self.selected_prop == None:
			return;
		x0, y0, x1, y1 = self.selected_prop.get_aabb();
		frame_dx, frame_dy = self.canvas_cursor[0] - x0, self.canvas_cursor[1] - y0;
		ddx, ddy = frame_dx - self.selection_delta[0], frame_dy - self.selection_delta[1];
		self.selected_prop.position[0] += ddx;
		self.selected_prop.position[1] += ddy;
	
	def _inspector_asset_combo(self, identifier, asset_type, value):
		result = value;
		if imgui.begin_combo(f"##{identifier}", value):
			assets = AssetManager.get_assets(asset_type);
			for asset in assets:
				selected = result == asset["name"];
				if imgui.selectable(asset["name"], selected)[0]:
					result = asset["name"];
				if selected:
					imgui.set_item_default_focus();
			imgui.end_combo();
		return result;
	
	def _inspector_render_prop(self, prop):
		if imgui.tree_node(f"{prop.name} ({id(prop)})"):
			prop.sprite = self._inspector_asset_combo(id(prop), "sprite", prop.sprite);
			imgui.tree_pop();
	
	def _canvas_render_prop(self, prop):
		self.canvas.draw_image(prop.position[0], prop.position[1], prop.esprite.frame_images[0]);
	
	def _canvas_render_layers(self):
		for layer in self.layers:
			for prop in layer.props:
				self._canvas_render_prop(prop);
	
	def _canvas_render_gizmos(self):
		if self.show_axes:
			self.canvas.draw_line(self.size[0]/2, 0, self.size[0]/2, self.size[1], (64, 64, 64));
			self.canvas.draw_line(0, self.size[1]/2, self.size[0], self.size[1]/2, (64, 64, 64));

		if self.selected_prop != None:
			sel_aabb = self.selected_prop.get_aabb();
			self.canvas.draw_aabb(sel_aabb, (255, 0, 0));
	
		if self.show_viewport:
			x0, y0 = self.origin;
			x1, y1 = x0+240-1, y0+320-1;
			self.canvas.draw_aabb((x0, y0, x1, y1), (0, 0, 255));

	def render():
		if SceneEditor._ == None:
			return;
		self = SceneEditor._;

		if self.open:
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"Scene Editor", self.open, flags=self.window_flags);

			props = [];
			for layer in self.layers:
				props += layer.props;
			for prop in props:
				prop.update();
			
			self._cursor_io(imgui.get_cursor_screen_pos());
			self._select_prop();
			self._move_selection();

			self.canvas.clear((128, 128, 128));
			self._canvas_render_layers();
			self._canvas_render_gizmos();
			self.canvas.render(1);

			_, self.show_axes = imgui.checkbox("Show axes", self.show_axes);
			imgui.same_line();
			_, self.show_viewport = imgui.checkbox("Show viewport", self.show_viewport);

			if imgui.collapsing_header(f"Layers"):
				for (idx, layer) in enumerate(self.layers):
					if imgui.tree_node(f"Layer {idx}"):
						for prop in layer.props:
								self._inspector_render_prop(prop)
						if imgui.button("+"):
							prop = Prop([0, 0], f"Prop", "null_sprite");
							layer.add_prop(prop);
						imgui.tree_pop();
				if imgui.button("+"):
					self.layers.append(Layer());
			
			self.size = imgui.get_window_size();
			imgui.end();
		
		if not self.open:
			SceneEditor._ = None;