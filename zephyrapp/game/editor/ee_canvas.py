from enum import Flag, auto;
from imgui_bundle import imgui;
from PIL import Image, ImageDraw;

from ee_cowtools import *;

class Canvas:
	class DrawFlags(Flag):
		CENTER_X = auto()
		CENTER_Y = auto()
		BOTTOM = auto()

	def __init__(self, width, height):
		self.width = width;
		self.height = height;
		self.image = Image.new("RGBA", (self.width, self.height), (0, 0, 0, 0));
		self.texture = make_texture(self.image.tobytes(), width, height);
		self.draw = ImageDraw.Draw(self.image);
		self.draw_flags = ();

	def clear(self, c):
		self.draw.rectangle((0, 0, self.width, self.height), fill=c);

	def draw_pixel(self, x, y, c):
		self.draw.point((x, y), fill=c);

	def draw_rect(self, x, y, w, h, c):
		if Canvas.DrawFlags.CENTER_X in self.draw_flags:
			x -= w // 2;
		if Canvas.DrawFlags.CENTER_Y in self.draw_flags:
			y -= h // 2;
		elif Canvas.DrawFlags.BOTTOM in self.draw_flags:
			y -= h;
		self.draw.rectangle((x, y, x+w-1, y+h-1), outline=c);

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
	
	def render(self, scale):
		glBindTexture(GL_TEXTURE_2D, self.texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, self.width, self.height, GL_RGBA, GL_UNSIGNED_BYTE, self.image.tobytes());
		imgui.image(self.texture, (self.width * scale, self.height * scale));