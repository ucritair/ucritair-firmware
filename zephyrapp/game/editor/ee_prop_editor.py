from imgui_bundle import imgui;
import glfw;
from enum import Enum;

from ee_cowtools import *;
from ee_canvas import Canvas;
from ee_assets import *;
from ee_sprites import SpriteBank;
from ee_input import InputManager;

#########################################################
## PROP EDITOR

class EditMode(Enum):
	BLOCKERS=0,
	TRIGGERS=1

class SpawnHelper:
	def spawn_blocker(aabb = [0, 0, 8, 8]):
		x0, y0, x1, y1 = aabb;
		return [x0, y0, x1, y1];

	def spawn_trigger(aabb = [0, 0, 8, 8], proc=""):
		x0, y0, x1, y1 = aabb;
		return {
			"aabb" : [x0, y0, x1, y1],
			"proc" : proc
		};

class CanvasHelper:
	def transform_point(canvas, point):
		x, y = point;
		return x + canvas.width/2, y + canvas.height/2;

	def transform_aabb(canvas, aabb):
		x0, y0, x1, y1 = aabb;
		x0, y0 = CanvasHelper.transform_point(canvas, [x0, y0]);
		x1, y1 = CanvasHelper.transform_point(canvas, [x1, y1]);
		return x0, y0, x1, y1;

class PropEditor:
	_ = None;

	def __init__(self):
		if PropEditor._ != None:
			return None;
		PropEditor._ = self;

		self.window_size = (680, 680);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		self.open = True;

		self.canvas_size = (256, 256);
		self.canvas_scale = 2;
		self.canvas = Canvas(self.canvas_size[0], self.canvas_size[1]);
	
		self.prop = None;
		self.edit_mode = EditMode.BLOCKERS;
		self.editee = None;
		self.edit_context = None;

		self.canvas_cursor = [0, 0];
		self.canvas_cursor_delta = [0, 0];
	
		self.show_sprite = True;
		self.show_aabb = False;
	
	def gui_prop_selector(self):
		if imgui.begin_combo(f"##{id(self.prop)}", self.prop["name"] if self.prop != None else "None"):
			assets = AssetManager.get_assets("prop");
			for asset in assets:
				selected = self.prop == asset;
				if imgui.selectable(asset["name"], selected)[0]:
					self.prop = asset;
				if selected:
					imgui.set_item_default_focus();
			imgui.end_combo();
	
	def gui_edit_mode_selector(self):
		if self.prop == None:
			return;
	
		if imgui.begin_combo(f"##{id(self.edit_mode)}", self.edit_mode.name):
			for mode in EditMode:
				selected = self.edit_mode == mode;
				if imgui.selectable(mode.name, selected)[0]:
					self.edit_mode = mode;
				if selected:
					imgui.set_item_default_focus();
			imgui.end_combo();
	
	def gui_editee_selector(self):
		if self.prop == None:
			return;
	
		if self.edit_mode == EditMode.BLOCKERS or self.edit_mode == EditMode.TRIGGERS:
			options = self.prop["blockers"] if self.edit_mode == EditMode.BLOCKERS else [t["aabb"] for t in self.prop["triggers"]];
			selected_idx = options.index(self.editee) if self.editee in options else -1;
			if imgui.begin_combo(f"##{id(self.editee)}", str(selected_idx) if selected_idx != -1 else "None"):
				for idx in range(len(options)):
					selected = idx == selected_idx;
					if imgui.selectable(str(idx), selected)[0]:
						self.editee = options[idx];
					if selected:
						imgui.set_item_default_focus();
				imgui.end_combo();
	
	def gui_addition_panel(self):
		if self.prop == None:
			return;
	
		if imgui.collapsing_header("Blockers"):
			if imgui.button("+"):
				self.prop["blockers"].append(SpawnHelper.spawn_blocker());
		if imgui.collapsing_header("Triggers"):
			if imgui.button("+"):
				self.prop["triggers"].append(SpawnHelper.spawn_trigger());
	
	def canvas_cursor_io(self, canvas_pos):
		cursor = InputManager.get_imgui_cursor();
		x, y = cursor - canvas_pos;
		canvas_cursor = [int(x)/self.canvas_scale, int(y)/self.canvas_scale];
		canvas_cursor = [round(canvas_cursor[0]/8)*8, round(canvas_cursor[1]/8)*8]
		self.canvas_cursor_delta = [canvas_cursor[0] - self.canvas_cursor[0], canvas_cursor[1] - self.canvas_cursor[1]];
		self.canvas_cursor = canvas_cursor;
	
	def is_editing(self):
		return self.editee != None and self.edit_context != None;
	
	def canvas_aabb_edit_io(self):
		if self.prop == None:
			return;
		if (self.edit_mode != EditMode.BLOCKERS and self.edit_mode != EditMode.TRIGGERS) or self.editee == None:
			return;
	
		sprite = SpriteBank.get(self.prop["sprite"]);
		sprite_aabb = [0, 0, sprite.frame_width-1, sprite.frame_height-1];
		sprite_position = [-sprite.frame_width/2, -sprite.frame_height];
		sprite_aabb = move_aabb(sprite_aabb, sprite_position);

		aabb = move_aabb(self.editee, sprite_position);
		aabb = CanvasHelper.transform_aabb(self.canvas, aabb);

		if InputManager.is_pressed(glfw.MOUSE_BUTTON_LEFT) and not self.is_editing():	
			for i in range(4):
				cursor_part = 0 if i % 2 == 0 else 1;
				if abs(self.canvas_cursor[cursor_part] - aabb[i]) <= 1:
					self.edit_context = {"aabb_edge" : i};
					break;
		if self.is_editing():
			edge = self.edit_context["aabb_edge"];
			delta = self.canvas_cursor_delta[edge % 2];
			self.editee[edge] += delta;
			self.editee[2] = max(self.editee[2], self.editee[0]);
			self.editee[3] = max(self.editee[3], self.editee[1]);
			if InputManager.is_released(glfw.MOUSE_BUTTON_LEFT):
				self.edit_context = None;

	def canvas_draw_axes(self):
		x, y = self.canvas.width/2, self.canvas.height/2;
		self.canvas.draw_line(x, 0, x, self.canvas.height, (64, 64, 64));
		self.canvas.draw_line(0, y, self.canvas.width, y, (64, 64, 64));

	def canvas_draw_prop(self, show_sprite=True, show_aabb=False, show_blockers=False, show_triggers=False):
		if self.prop == None:
			return;

		sprite = SpriteBank.get(self.prop["sprite"]);
		sprite_aabb = [0, 0, sprite.frame_width-1, sprite.frame_height-1];
		sprite_position = [-sprite.frame_width/2, -sprite.frame_height];

		if show_sprite:
			frame = sprite.frame_images[0];
			aabb = CanvasHelper.transform_aabb(self.canvas, move_aabb(sprite_aabb, sprite_position));
			self.canvas.draw_image(aabb[0], aabb[1], frame);
		
		if show_aabb:
			aabb = CanvasHelper.transform_aabb(self.canvas, move_aabb(sprite_aabb, sprite_position));
			self.canvas.draw_aabb(aabb, (255, 255, 255));
	
		if show_blockers:
			for blocker in self.prop["blockers"]:
				aabb = CanvasHelper.transform_aabb(self.canvas, move_aabb(blocker, sprite_position));
				self.canvas.draw_aabb(aabb, (255, 0, 0));
	
		if show_triggers:
			for trigger in self.prop["triggers"]:
				aabb = CanvasHelper.transform_aabb(self.canvas, move_aabb(trigger["aabb"], sprite_position));
				self.canvas.draw_aabb(aabb, (0, 255, 0));

				x0, y0, x1, y1 = aabb;
				w, h = x1-x0, y1-y0;
				cx, cy = x0+w/2, y0+h/2;
				tx, ty = trigger["direction"];
				self.canvas.draw_line(cx, cy, cx+tx*16, cy+ty*16, (0, 255, 0));

	def render():
		if PropEditor._ == None:
			return;
		self = PropEditor._;

		if self.open:
			imgui.set_next_window_size(self.window_size);
			_, self.open = imgui.begin(f"Prop Editor", self.open, flags=self.window_flags);

			self.gui_prop_selector();
			self.gui_edit_mode_selector();
			self.gui_editee_selector();

			self.canvas.clear((128, 128, 128));
			self.canvas_draw_axes();
			self.canvas_draw_prop(
				show_sprite=self.show_sprite,
				show_aabb=self.show_aabb,
				show_blockers=self.edit_mode == EditMode.BLOCKERS and self.editee != None,
				show_triggers=self.edit_mode == EditMode.TRIGGERS and self.editee != None
			);
			# Cursor IO has to come right before canvas draw
			self.canvas_cursor_io(imgui.get_cursor_screen_pos());
			self.canvas_aabb_edit_io();
			self.canvas.render(self.canvas_scale);

			_, self.show_sprite = imgui.checkbox("Show sprite", self.show_sprite);
			imgui.same_line();
			_, self.show_aabb = imgui.checkbox("Show AABB", self.show_aabb);

			self.gui_addition_panel();
			
			self.window_size = imgui.get_window_size();
			imgui.end();
		
		if not self.open:
			PropEditor._ = None;