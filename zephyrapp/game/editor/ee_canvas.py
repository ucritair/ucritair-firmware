from enum import Flag, auto;
from imgui_bundle import imgui;
from PIL import Image, ImageDraw;

from ee_cowtools import *;
from ee_input import InputManager;

class Canvas:
	class DrawFlags(Flag):
		CENTER_X = auto()
		CENTER_Y = auto()
		BOTTOM = auto()

	def __init__(self, width, height, scale=1):
		self.width = width;
		self.height = height;
		self.scale = scale;

		self.image = Image.new("RGBA", (self.width, self.height), (0, 0, 0, 0));
		self.texture = make_texture(self.image.tobytes(), width, height);
		self.draw = ImageDraw.Draw(self.image);
		self.draw_flags = ();
	
		self.position = None;
		self.cursor = None;

	def clear(self, c):
		self.draw.rectangle((0, 0, self.width, self.height), fill=c);

	def draw_pixel(self, x, y, c):
		self.draw.point((x, y), fill=c);

	def draw_rect_old(self, x, y, w, h, c):
		if Canvas.DrawFlags.CENTER_X in self.draw_flags:
			x -= w // 2;
		if Canvas.DrawFlags.CENTER_Y in self.draw_flags:
			y -= h // 2;
		elif Canvas.DrawFlags.BOTTOM in self.draw_flags:
			y -= h;
		self.draw.rectangle((x, y, x+w-1, y+h-1), outline=c);
	
	def draw_aabb(self, aabb, c):
		x0, y0, x1, y1 = aabb;
		w, h = x1-x0, y1-y0;
		if Canvas.DrawFlags.CENTER_X in self.draw_flags:
			x0 -= w // 2;
		if Canvas.DrawFlags.CENTER_Y in self.draw_flags:
			y0 -= h // 2;
		elif Canvas.DrawFlags.BOTTOM in self.draw_flags:
			y0 -= h;
		self.draw.rectangle((x0, y0, x1, y1), outline=c);

	def draw_line(self, xi, yi, xf, yf, c):
		self.draw.line((xi, yi, xf, yf), fill=c);

	def draw_circle(self, x, y, r, c):
		self.draw.circle((x, y), r, outline=c);
	
	def draw_image(self, x, y, image):
		if Canvas.DrawFlags.CENTER_X in self.draw_flags:
			x -= image.width // 2;
		if Canvas.DrawFlags.CENTER_Y in self.draw_flags:
			y -= image.height // 2;
		elif Canvas.DrawFlags.BOTTOM in self.draw_flags:
			y -= image.height;
		self.image.paste(image, (int(x), int(y)), mask=image);
	
	def draw_text(self, xy, text, s, c):
		self.draw.text(xy, text, font_size=s, stroke_fill=c);
	
	def render(self):
		self.position = imgui.get_cursor_screen_pos();
		glBindTexture(GL_TEXTURE_2D, self.texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, self.width, self.height, GL_RGBA, GL_UNSIGNED_BYTE, self.image.tobytes());
		imgui.image(self.texture, (self.width * self.scale, self.height * self.scale));

class CanvasIO:
	def __init__(self, canvas):
		self.canvas = canvas;
		self.cursor = None;
	
	def tick(self):
		cursor = InputManager.get_imgui_cursor() - self.canvas.position;
		cursor = cursor.x / self.canvas.scale, cursor.y / self.canvas.scale;
		self.cursor = cursor;

	def get_cursor(self):
		return self.cursor;

class CanvasGrid:
	def __init__(self, canvas, origin, size):
		self.canvas = canvas;
		x, y = origin;
		self.origin = int(x), int(y);
		self.size = int(size);

	def transform_point(self, point):
		x, y = point;
		x, y = x // self.size, y // self.size;
		x, y = x - self.origin[0], y - self.origin[1];
		return x, y;

	def untransform_point(self, point):
		x, y = point;
		x, y = x + self.origin[0], y + self.origin[1];
		x, y = x * self.size, y * self.size;
		return x, y;

	def transform_aabb(self, aabb):
		x0, y0, x1, y1 = aabb;
		x0y0, x1y1 = self.transform_point((x0, y0)), self.transform_point((x1, y1));
		x0, y0 = x0y0;
		x1, y1 = x1y1;
		return x0, y0, x1, y1;

	def untransform_aabb(self, aabb):
		x0, y0, x1, y1 = aabb;
		x0y0, x1y1 = self.untransform_point((x0, y0)), self.untransform_point((x1, y1));
		x0, y0 = x0y0;
		x1, y1 = x1y1;
		return x0, y0, x1, y1;

	def draw_lines(self, colour):
		for y in range(self.size, self.canvas.height, self.size):
			self.canvas.draw_line(0, y, self.canvas.width, y, colour);
		for x in range(self.size, self.canvas.width, self.size):
			self.canvas.draw_line(x, 0, x, self.canvas.height, colour);