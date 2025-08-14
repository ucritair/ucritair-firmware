#!/usr/bin/env python3

from pathlib import Path;
import re;
from imgui_bundle import imgui;
from ee_cowtools import *;

#########################################################
## PROC REGISTRY

def _parse_c_signatures(text):
	statements = re.findall(r"void\s*([A-z_]+)\s*\(\);", text);
	return statements;

class ProcRegistry:
	def __init__(self, path):
		self.directory = Path(path);
		self.signatures = [];

		for filepath in self.directory.iterdir():
			if filepath.name == "cat_procs.h":
				with open(filepath, "r") as file:
					self.signatures += _parse_c_signatures(file.read());

class ProcExplorer:
	_ = None;

	def __init__(self, path):
		if ProcExplorer._ != None:
			return None;
		ProcExplorer._ = self;

		self.size = (640, 480);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);

		self.registry = ProcRegistry(path);
	
	def live():
		return ProcExplorer._ != None;

	def render(current):
		self = ProcExplorer._;
		if self == None:
			return current
		
		listings = [s for s in self.registry.signatures];
		listings.sort();
		
		imgui.set_next_window_size(self.size);
		_, open = imgui.begin(f"Proc Explorer", self != None, flags=self.window_flags);

		result = current;
		for item in listings:
			if imgui.menu_item_simple(item):
				result = item;
				ProcExplorer._ = None;
		if imgui.button("Clear"):
			result = "";
			ProcExplorer._ = None;

		self.size = imgui.get_window_size();
		imgui.end();

		if not open:
			ProcExplorer._ = None;
		
		return result;

pr = ProcRegistry("src/procs");
		