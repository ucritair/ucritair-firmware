from imgui_bundle import imgui;
from ee_cowtools import foldl;

class ToolWindow:
	def __init__(self, T, title, size=(720, 720), flags=[], hidden=False):
		self.T = T;
		self.instance = None;

		self.title = title;
		self.size = size;
		self.flags = foldl(lambda a, b : a | b, 0, flags);
		self.hidden = hidden;

	def open(self):
		if self.instance == None:
			self.instance = self.T();
	
	def is_open(self):
		return self.instance != None;

	def get(self):
		return self.instance;

	def should_close(self):
		if self.instance == None:
			return True;
		if not callable(getattr(self.instance, "should_close", None)):
			return False;
		return self.instance.should_close();

	def get_result(self):
		if self.instance == None:
			return None;
		if not callable(getattr(self.instance, "get_result", None)):
			return None;
		return self.instance.get_result();

	def draw(self):
		if self.instance == None:
			return;
	
		imgui.set_next_window_size(self.size);
		_, open = imgui.begin(self.title, self.instance != None and not self.should_close(), flags=self.flags);
		self.instance.draw();
		self.size = imgui.get_window_size();
		imgui.end();
		if not open:
			self.instance = None;
	

class ToolWindowRegistry:
	table = {};
	
	def register(tool):
		ToolWindowRegistry.table[tool.T] = tool;

	def lookup(T):
		return ToolWindowRegistry.table[T];

	def all():
		return ToolWindowRegistry.table.values();