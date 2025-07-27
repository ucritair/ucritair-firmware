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
		self.buffer = bytearray(bytes(width * height * 4));
		self.texture = make_texture(self.buffer, width, height);
		self.draw_flags = ();

	def clear(self, c):
		for y in range(self.height):
			for x in range(self.width):
				i = (y * self.width + x) * 4;
				self.buffer[i+0] = c[0];
				self.buffer[i+1] = c[1];
				self.buffer[i+2] = c[2];
				self.buffer[i+3] = 255;

	def draw_pixel(self, x, y, c):
		x = int(x);
		y = int(y);

		if x < 0 or x >= self.width:
			return;
		if y < 0 or y >= self.height:
			return;

		i = (y * self.width + x) * 4;
		self.buffer[i+0] = c[0];
		self.buffer[i+1] = c[1];
		self.buffer[i+2] = c[2];
	
	def draw_hline(self, y, c):
		for x in range(self.width):
			self.draw_pixel(x, y, c);
	
	def draw_vline(self, x, c):
		for y in range(self.height):
			self.draw_pixel(x, y, c);

	def draw_rect(self, x, y, w, h, c):
		x = int(x);
		y = int(y);
		w = int(w);
		h = int(h);

		if Canvas.DrawFlags.CENTER_X in self.draw_flags:
			x -= w // 2;
		if Canvas.DrawFlags.CENTER_Y in self.draw_flags:
			y -= h // 2;
		elif Canvas.DrawFlags.BOTTOM in self.draw_flags:
			y -= h;

		for dy in range(y, y+h):
			self.draw_pixel(x, dy, c);
			self.draw_pixel(x+w-1, dy, c);
		for dx in range(x, x+w):
			self.draw_pixel(dx, y, c);
			self.draw_pixel(dx, y+h-1, c);

	def draw_line(self, xi, yi, xf, yf, c):
		xi = int(xi);
		yi = int(yi);
		xf = int(xf);
		yf = int(yf);
	
		steep = abs(yf-yi) > abs(xf-xi);
		if steep:
			temp = xi;
			xi = yi;
			yi = temp;

			temp = xf;
			xf = yf;
			yf = temp;

		# if the line heads left, swap its start and end points
		leftward = xi > xf;
		if leftward:
			temp = xi;
			xi = xf;
			xf = temp;

			temp = yi;
			yi = yf;
			yf = temp;
		
		dx = xf - xi;
		dy = yf - yi;

		# account for line heading up or down
		y_step = 1 if (yf > yi) else -1;
		y = yi;
		
		# approximate d_err as abs(dy) / (dx ~= 0.5)
		d_err = abs(dy) * 2;
		err = 0;

		# if line is steep, we swap x,y in the draw call to undo our earlier transposition
		# we employ a branch between two for loops to avoid branching within one loop
		if steep:
			for x in range(xi, xf):
				xw = x;
				if y >= 0 and y < self.width and xw >= 0 and xw < self.height:
					self.draw_pixel(y, xw, c);

				err += d_err;
				if err > dx:
					y += y_step;
					err -= 2*dx;
		else:
			for x in range(xi, xf):
				yw = y;
				if x >= 0 and x < self.width and yw >= 0 and yw < self.height:
					self.draw_pixel(x, yw, c);

				err += d_err;
				if err > dx:
					y += y_step;
					err -= dx*2;

	def draw_circle(self, x, y, r, c):
		x = int(x);
		y = int(y);
		r = int(r);
	
		f = 1 - r;
		ddfx = 0;
		ddfy = -2 * r;
		dx = 0;
		dy = r;

		self.draw_pixel(x, y + r, c);
		self.draw_pixel(x, y - r, c);
		self.draw_pixel(x + r, y, c);
		self.draw_pixel(x - r, y, c);

		while dx < dy:
			if f >= 0:
				dy -= 1;
				ddfy += 2;
				f += ddfy;

			dx += 1;
			ddfx += 2;
			f += ddfx + 1;

			self.draw_pixel(x + dx, y + dy, c);
			self.draw_pixel(x - dx, y + dy, c);
			self.draw_pixel(x + dx, y - dy, c);
			self.draw_pixel(x - dx, y - dy, c);
			self.draw_pixel(x + dy, y + dx, c);
			self.draw_pixel(x - dy, y + dx, c);
			self.draw_pixel(x + dy, y - dx, c);
			self.draw_pixel(x - dy, y - dx, c);
	
	def draw_image(self, x, y, image):
		x = int(x);
		y = int(y);

		if Canvas.DrawFlags.CENTER_X in self.draw_flags:
			x -= image.width // 2;
		if Canvas.DrawFlags.CENTER_Y in self.draw_flags:
			y -= image.height // 2;
		elif Canvas.DrawFlags.BOTTOM in self.draw_flags:
			y -= image.height;

		pixels = image.load();
		for yr in range(0, image.height):
			yw = y+yr;
			if yw < 0 or yw >= self.height:
				continue;
			for xr in range(0, image.width):
				xw = x+xr;
				if xw < 0 or xw >= self.width:
					continue;
				c = pixels[xr, yr];
				if c[3] < 128:
					continue;

				i = (yw * self.width + xw) * 4;
				self.buffer[i+0] = c[0];
				self.buffer[i+1] = c[1];
				self.buffer[i+2] = c[2];
	
	def render(self, scale):
		glBindTexture(GL_TEXTURE_2D, self.texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, self.width, self.height, GL_RGBA, GL_UNSIGNED_BYTE, self.buffer);
		imgui.image(self.texture, (self.width * scale, self.height * scale));