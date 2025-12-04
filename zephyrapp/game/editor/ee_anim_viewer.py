from imgui_bundle import imgui;
from ee_assets import AssetManager;
from ee_canvas import Canvas;
from ee_sprites import SpriteBank;
import ee_context;

class AnimationViewer:
	def __init__(self):
		self.canvas = Canvas(240, 128, 2);
		self.scale = 240 / self.canvas.height;

		self.sprites = AssetManager.get_assets("sprite");
		self.sprite_idx = 0;
		self.frame = 0;
		
		self.animate = True;
		self.show_AABB = False;
		self.timer = 0;

	def draw(self):
		sprite = self.sprites[self.sprite_idx];
		preview = SpriteBank.get(sprite["name"]);

		self.canvas.clear((128, 128, 128));
		draw_x = self.canvas.width/2;
		draw_y = self.canvas.height/2;
		self.canvas.draw_flags = Canvas.DrawFlags.CENTER_X | Canvas.DrawFlags.CENTER_Y;
		self.canvas.draw_image(draw_x, draw_y, preview.frame_images[self.frame]);
		if self.show_AABB:
			self.canvas.draw_rect_old(draw_x, draw_y, preview.frame_width, preview.frame_height, (255, 0, 0));
		self.canvas.render();

		if preview.frame_count > 1:	
			animate_changed, self.animate = imgui.checkbox("Animate", self.animate);
			if animate_changed:
				self.frame = 0;
			if self.animate:
				self.timer += ee_context.delta_time;
				if self.timer >= 0.2:
					self.timer = 0;
					self.frame += 1;
					if self.frame >= preview.frame_count:
						self.frame = 0;
			else:
				_, self.frame = imgui.slider_int("Frame", self.frame, 0, preview.frame_count-1);
		
		_, self.show_AABB = imgui.checkbox("Show AABB", self.show_AABB);

		if imgui.begin_combo(f"Sprite", sprite["name"]):
			for (idx, entry) in enumerate(self.sprites):
				selected = idx == self.sprite_idx;
				if imgui.selectable(entry["name"], selected)[0]:
					self.sprite_idx = idx;
					self.frame = 0;
				if selected:
					imgui.set_item_default_focus();
			imgui.end_combo();