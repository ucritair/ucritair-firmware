from imgui_bundle import imgui;

from ee_cowtools import *;
from ee_canvas import Canvas;

#########################################################
## SCENE EDITOR

class SceneEditor:
	def __init__(self):
		if SceneEditor._ != None:
			return None;
		SceneEditor._ = self;

		self.size = (720, 480);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;

		self.themes = asset_docs["theme"].entries;
		self.theme = self.themes[0];

		self.wall_canvas = self.wall_canvas = Canvas(240, 96);
		self.floor_canvas = Canvas(240, 224);

		self.cursor = (0, 0);
		self.wall_brush = 0;
		self.floor_brush = 0;

	def render():
		if SceneEditor._ == None:
			return;
		self = SceneEditor._;

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
					self.wall_canvas.draw_rect(self.wall_cursor[0] * 16, self.wall_cursor[1] * 16, 16, 16, (255, 0, 0));
				
				window_rect = self.theme['window_rect']
				if len(window_rect) == 4:
					self.wall_canvas.draw_rect(window_rect[0], window_rect[1], window_rect[2], window_rect[3], (0, 0, 255));

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
			SceneEditor._ = None;