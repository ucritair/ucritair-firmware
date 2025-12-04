from pathlib import Path;
from imgui_bundle import imgui;
from ee_cowtools import foldl;
import os;

from ee_tool_window import ToolWindow;

class FileExplorer:
	def __init__(self):
		self.target = None;
		self.result = None;
	
	def configure(self, target, anchor, glob):
		self.target = target;
		self.anchor = Path(anchor);
		self.glob = glob;
		self.current = self.anchor;

	def draw(self):
		listings = [self.current.parent.absolute()];
		for entry in self.current.iterdir():
			hidden = foldl(lambda a, b : a or b, False, [str(p).startswith(".") for p in entry.parts]);
			if entry.absolute().is_dir() and not hidden:
				listings.append(entry);
		for entry in self.current.glob(self.glob):
			if not entry in listings:
				listings.append(entry);
		listings.sort();
		
		for item in listings:
			name = ".." if item == self.current.parent.absolute() else item.name;
			name = f"{name}/" if item.is_dir() else name;
			if imgui.menu_item_simple(name):
				if item.is_dir():
					self.current = item;
				else:
					self.result = Path(os.path.relpath(item.absolute(), self.anchor.absolute()));
	
	def is_targeting(self, target):
		return target == self.target;

	def should_close(self):
		return self.result != None;
		
	def get_result(self):
		return self.result;