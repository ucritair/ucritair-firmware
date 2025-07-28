from imgui_bundle import imgui;

from ee_cowtools import *;
from ee_canvas import Canvas;
from ee_assets import *;
from ee_sprites import SpriteBank;

#########################################################
## SCENE EDITOR

class Prop:
	def __init__(self, position, name, sprite):
		self.position = position;
		self.name = name;
		self.sprite = sprite;

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

		SpriteBank.Initialize();

		self.size = (1000, 600);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;

		self.scenes = AssetManager.get_assets("scene");
		self.scene = None;

		self.layers = [];
		self.canvas = Canvas(self.size[0], self.size[1]);
	
	def _inspector_render_prop(self, prop):
		if imgui.tree_node(f"{prop.name} ({id(prop)})"):
			imgui.tree_pop();
	
	def _canvas_render_prop(self, prop):
		sprite = SpriteBank.get(prop.sprite);
		self.canvas.draw_image(prop.position[0], prop.position[1], sprite.frame_images[0]);

	def render():
		if SceneEditor._ == None:
			return;
		self = SceneEditor._;

		if self.open:
			imgui.set_next_window_size(self.size);
			_, self.open = imgui.begin(f"Scene Editor", self.open, flags=self.window_flags);

			self.canvas.clear((128, 128, 128));
			for layer in self.layers:
				for prop in layer.props:
					self._canvas_render_prop(prop);
			self.canvas.render(1);

			if imgui.collapsing_header(f"Layers"):
				for (idx, layer) in enumerate(self.layers):
					if imgui.tree_node(f"Layer {idx}"):
						for prop in layer.props:
								self._inspector_render_prop(prop)
						if imgui.button("+"):
							prop = Prop((0, 0), f"Prop", "null_sprite");
							layer.add_prop(prop);
						imgui.tree_pop();
				if imgui.button("+"):
					self.layers.append(Layer());
			
			self.size = imgui.get_window_size();
			imgui.end();
		
		if not self.open:
			SceneEditor._ = None;