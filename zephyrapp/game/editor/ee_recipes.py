from imgui_bundle import imgui;
import glfw;

from ee_cowtools import *;
from ee_canvas import Canvas, CanvasGrid, CanvasIO;
from ee_assets import *;
from ee_sprites import SpriteBank, EditorSprite;
from ee_input import InputManager;

class RecipeEditor:
	def __init__(self):	
		self.cell_size = 48
		self.canvas_size = (self.cell_size * 3, self.cell_size * 3);
		self.canvas = Canvas(self.canvas_size[0], self.canvas_size[1], 2);
		self.canvas_io = CanvasIO(self.canvas);
		self.canvas_grid = CanvasGrid(self.canvas, (0, 0), self.cell_size);
	
		self.recipe = None;
		self.input_coords = None;
		self.input_idx = None;
		self.copy_buffer = None;
	
	def get_input_cursor(self):
		cursor = self.canvas_io.get_cursor();
		if not cursor is None:
			cursor = self.canvas_grid.transform_point(cursor);
			x, y = cursor;
			return x, y;
		return None;
	
	def draw_input_grid(self):
		self.canvas.clear((0, 0, 0));
		self.canvas_grid.draw_lines((128, 128, 128));
		
		if self.recipe != None:
			for y in range(3):
				for x in range(3):
					idx = y * 3 + x;
					item_name = self.recipe["inputs"][idx];
					item = AssetManager.search("item", item_name);
					if item == None:
						continue;
					sprite = SpriteBank.get(item["sprite"]);
					self.canvas.draw_flags = Canvas.DrawFlags.CENTER_X | Canvas.DrawFlags.CENTER_Y;
					self.canvas.draw_image(
						x * self.cell_size + self.cell_size/2,
						y * self.cell_size + self.cell_size/2,
						sprite.frame_images[0]
					);
					self.canvas.draw_flags = ();
		
		cursor = self.get_input_cursor();
		if cursor != None:
			x, y = cursor;
			if x >= 0 and x < 3 and y >= 0 and y < 3:
				self.canvas_grid.draw_cell(cursor, (192, 192, 192));
				if InputManager.is_pressed(glfw.MOUSE_BUTTON_LEFT):
					self.input_coords = (x, y);	
					self.input_idx = int(y * 3 + x);
		if self.input_coords != None:
			self.canvas_grid.draw_cell(self.input_coords, (255, 255, 255));

		self.canvas.render();
	
	def draw_output_box(self):
		if self.recipe != None:
			item_name = self.recipe["output"];
			item = AssetManager.search("item", item_name);
			sprite = SpriteBank.get(item["sprite"]) if item != None else SpriteBank.get("null_sprite");

			true_height = sprite.frame_height;
			display_height = self.cell_size * self.canvas.scale;
			scale_factor = display_height / true_height;
			display_width = sprite.frame_width * scale_factor;
			imgui.image(sprite.frame_textures[0], (display_width, display_height));

			imgui.same_line();
			imgui.set_next_item_width(self.canvas_size[0] * self.canvas.scale - display_width);

			item = imgui_asset_selector(id(item), "item", item);
			self.recipe["output"] = item["name"] if item != None else "";
	
	def draw_input_gui(self):
		if self.recipe is None:
			return;
		if self.input_idx is None:
			return;
		item_name = self.recipe["inputs"][self.input_idx];
		item = AssetManager.search("item", item_name);
		item = imgui_asset_selector(id(item), "item", item);
		self.recipe["inputs"][self.input_idx] = item["name"] if item != None else "";
		if InputManager.is_pressed(glfw.KEY_X):
			self.recipe["inputs"][self.input_idx] = "";
		if InputManager.is_held(glfw.KEY_LEFT_SUPER) and InputManager.is_pressed(glfw.KEY_C):
			self.copy_buffer = self.recipe["inputs"][self.input_idx];
		if InputManager.is_held(glfw.KEY_LEFT_SUPER) and InputManager.is_pressed(glfw.KEY_V):
			if self.copy_buffer != None:
				self.recipe["inputs"][self.input_idx] = self.copy_buffer;
	
	def draw(self):
		self.canvas_io.tick();

		imgui.begin_group();
		self.draw_input_grid();
		imgui.end_group();

		imgui.same_line();

		imgui.begin_group();
		imgui.set_next_item_width(self.canvas_size[0] * self.canvas.scale);
		self.recipe = imgui_asset_selector(id(self.recipe), "recipe", self.recipe);
		self.draw_output_box();
		self.draw_input_gui();
		imgui.end_group();
			
