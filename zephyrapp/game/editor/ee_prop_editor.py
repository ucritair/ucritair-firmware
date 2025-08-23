from imgui_bundle import imgui;
import glfw;
from enum import Enum;

from ee_cowtools import *;
from ee_canvas import Canvas, CanvasGrid, CanvasIO;
from ee_assets import *;
from ee_sprites import SpriteBank;
from ee_input import InputManager;
from ee_procs import ProcExplorer;

#########################################################
## PROP EDITOR

class EditMode(Enum):
	BLOCKERS=0,
	TRIGGERS=1

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
		self.tile_size = 16;

		self.canvas = Canvas(self.canvas_size[0], self.canvas_size[1], scale=2);
		self.canvas_io = CanvasIO(self.canvas);
		self.canvas_grid = CanvasGrid(
			self.canvas,
			((self.canvas_size[0]//self.tile_size)//2, (self.canvas_size[1]//self.tile_size)//2),
			self.tile_size
		);
	
		self.prop = None;
		
		self.edit_mode = EditMode.BLOCKERS;
		self.editee = None;
		self.edit_context = None;
	
		self.show_sprite = True;
		self.show_aabb = False;
		self.variant = 0;
	
	def gui_editee_selector(self):
		if self.prop == None:
			return;

		candidates = [];
		match self.edit_mode:
			case EditMode.BLOCKERS:
				candidates = self.prop["blockers"];
			case EditMode.TRIGGERS:
				candidates = [t["aabb"] for t in self.prop["triggers"]];
		
		self.editee = imgui_selector(
			id(self.editee),
			candidates, self.editee,
			lambda x: str(candidates.index(x)) if x in candidates else "None"
		);
	
	def gui_addition_panel(self):
		trash = [];

		if self.prop == None:
			return;
	
		if imgui.collapsing_header("Blockers"):
			for (idx, blocker) in enumerate(self.prop["blockers"]):
				imgui.text(f"X0: {blocker[0]}, Y0: {blocker[1]}, X1: {blocker[2]}, Y1: {blocker[3]}");
				imgui.same_line();
				if imgui.button(f"-##{id(blocker)}"):
					trash.append(("blockers", idx));
			if imgui.button("+##blocker"):
				self.prop["blockers"].append([0, 0, 1, 1]);
		
		if imgui.collapsing_header("Triggers"):
			for (idx, trigger) in enumerate(self.prop["triggers"]):
				imgui.text(f"X0: {trigger["aabb"][0]}, Y0: {trigger["aabb"][1]}, X1: {trigger["aabb"][2]}, Y1: {trigger["aabb"][3]}");
				imgui.same_line();
				if imgui.button(f"-##{id(trigger)}"):
					trash.append(("triggers", idx));
				_, trigger["direction"] = imgui.input_int2(f"Direction##{id(trigger)}", trigger["direction"]);
				_, trigger["proc"] = imgui.input_text(f"Proc##{id(trigger)}", trigger["proc"]);
				imgui.same_line();
				if imgui.button(f"...##{id(trigger)}"):
					ProcExplorer("src/procs");
				trigger["proc"] = ProcExplorer.render(trigger["proc"]) if ProcExplorer.live() else trigger["proc"];
			if imgui.button("+##trigger"):
				self.prop["triggers"].append({
					"aabb" : [0, 0, 1, 1],
					"direction" : [0, 1],
					"proc" : ""
				});
	
		for key, idx in trash:
			del self.prop[key][idx];
	
	def is_editing(self):
		return self.editee != None and self.edit_context != None;
	
	def canvas_aabb_edit_io(self):
		if self.prop == None:
			return;
		if self.editee == None:
			return;
	
		aabb = self.canvas_grid.untransform_aabb(self.editee);

		if InputManager.is_pressed(glfw.MOUSE_BUTTON_LEFT) and not self.is_editing():	
			for i in range(4):
				if abs(self.canvas_io.cursor[i % 2] - aabb[i]) <= 1:
					self.edit_context = {"edge_idx" : i};
					break;
		if self.is_editing():
			edge_idx = self.edit_context["edge_idx"];
			delta = self.canvas_io.cursor[edge_idx % 2] - self.canvas_grid.untransform_aabb(self.editee)[edge_idx];
			delta //= self.tile_size;

			self.editee[edge_idx] += delta;
			self.editee[2] = max(self.editee[2], self.editee[0]);
			self.editee[3] = max(self.editee[3], self.editee[1]);

			if InputManager.is_released(glfw.MOUSE_BUTTON_LEFT):
				self.edit_context = None;

	def canvas_draw_prop(self, show_sprite=True, show_aabb=False, show_blockers=False, show_triggers=False):
		if self.prop == None:
			return;

		sprite = SpriteBank.get(self.prop["sprite"]);
		position = self.canvas_grid.untransform_point((0, 0));
		x, y = position;

		if show_sprite:
			frame = sprite.frame_images[self.variant];
			self.canvas.draw_image(x, y, frame);
		
		if show_aabb:
			aabb = [x, y, x+sprite.frame_width-1, y+sprite.frame_height-1];
			self.canvas.draw_aabb(aabb, (255, 255, 255));
	
		if show_blockers:
			for blocker in self.prop["blockers"]:
				aabb = self.canvas_grid.untransform_aabb(blocker);
				self.canvas.draw_aabb(aabb, (255, 0, 0));
	
		if show_triggers:
			for trigger in self.prop["triggers"]:
				aabb = self.canvas_grid.untransform_aabb(trigger["aabb"]);
				self.canvas.draw_aabb(aabb, (0, 255, 0));

				x0, y0, x1, y1 = aabb;
				w, h = x1-x0, y1-y0;
				cx, cy = x0+w/2, y0+h/2;
				tx, ty = trigger["direction"];
				self.canvas.draw_line(cx, cy, cx+tx*self.tile_size, cy+ty*self.tile_size, (0, 255, 0));

	def render():
		if PropEditor._ == None:
			return;
		self = PropEditor._;

		if self.open:
			imgui.set_next_window_size(self.window_size);
			_, self.open = imgui.begin(f"Prop Editor", self.open, flags=self.window_flags);

			self.prop = imgui_asset_selector(id(self.prop), "prop", self.prop);
			self.edit_mode = imgui_enum_selector(id(self.edit_mode), EditMode, self.edit_mode);
			self.gui_editee_selector();

			self.canvas.clear((128, 128, 128));
			self.canvas_grid.draw_lines((64, 64, 64));
			self.canvas_draw_prop(
				show_sprite=self.show_sprite,
				show_aabb=self.show_aabb,
				show_blockers=self.edit_mode == EditMode.BLOCKERS and self.editee != None,
				show_triggers=self.edit_mode == EditMode.TRIGGERS and self.editee != None
			);
			self.canvas.render();

			self.canvas_io.tick();
			self.canvas_aabb_edit_io();

			_, self.show_sprite = imgui.checkbox("Show sprite", self.show_sprite);
			imgui.same_line();
			_, self.show_aabb = imgui.checkbox("Show AABB", self.show_aabb);

			if self.prop != None and self.prop["palette"]:
				_, self.variant = imgui.slider_int("Variant", self.variant, 0, SpriteBank.get(self.prop["sprite"]).frame_count-1);

			self.gui_addition_panel();
			
			self.window_size = imgui.get_window_size();
			imgui.end();
		
		if not self.open:
			PropEditor._ = None;