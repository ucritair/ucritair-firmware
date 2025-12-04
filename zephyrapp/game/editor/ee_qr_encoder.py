from imgui_bundle import imgui;
import qrcode;

from ee_cowtools import *;
from ee_assets import *;
from ee_tool_window import ToolWindow;
from ee_canvas import Canvas;

class QREncoder:
	def __init__(self):
		self.data = "";
		self.code = None;
		self.canvas = None;
		self.save_path = "";
	
	def draw(self):
		_, self.data = imgui.input_text("Data", self.data);
		imgui.same_line();
		if imgui.button("Encode"):
			self.code = qrcode.make(self.data).get_image().convert("RGBA");
			self.canvas = Canvas(self.code.width, self.code.height);
		
		if self.code != None:
			self.canvas.clear((0, 0, 0, 0));
			self.canvas.draw_image(0, 0, self.code);
			self.canvas.render();

			_, self.save_path = imgui.input_text("Path", self.save_path);
			imgui.same_line();
			if imgui.button("Save"):
				self.code.save(self.save_path);
			
