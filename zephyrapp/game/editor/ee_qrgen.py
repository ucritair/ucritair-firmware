from imgui_bundle import imgui;

from ee_cowtools import *;
from ee_canvas import Canvas, CanvasGrid, CanvasIO;
from ee_assets import *;
from ee_input import InputManager;

import qrcode;

class QREncoder:
	_ = None;

	def __init__(self):
		if QREncoder._ != None:
			return None;
		QREncoder._ = self;

		self.size = (720, 720);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);
		
		self.data = "";
		self.code = None;
		self.canvas = None;
	
	def render():
		self = QREncoder._;
		if self == None:
			return;

		imgui.set_next_window_size(self.size);
		_, open = imgui.begin(f"QR Encoder", self != None, flags=self.window_flags);

		_, self.data = imgui.input_text("Data", self.data);
		imgui.same_line();
		if imgui.button("Encode"):
			self.code = qrcode.make(self.data);
			self.canvas = Canvas(self.code.width, self.code.height);
		if self.code != None:
			self.canvas.clear((0, 0, 0, 0));
			self.canvas.draw_image(0, 0, self.code.get_image());
			print(self.code.get_image());
			self.canvas.render();

		imgui.end();
		if not open:
			QREncoder._ = None;
			
