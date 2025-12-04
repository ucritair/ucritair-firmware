#!/usr/bin/env python3

from pathlib import Path;
import re;
from imgui_bundle import imgui;
from ee_cowtools import *;

#########################################################
## PROC REGISTRY

def _parse_c_signatures(text):
	signatures = re.findall(r"void\s*([A-z_]+)\s*\(CAT_prop_instance\s*\*\s+([A-z_]+)\);", text);
	proc_names = [proc for proc, arg in signatures];
	return proc_names;

class ProcRegistry:
	def __init__(self, path):
		self.directory = Path(path);
		self.signatures = [];

		for filepath in self.directory.iterdir():
			if filepath.name == "cat_procs.h":
				with open(filepath, "r") as file:
					self.signatures += _parse_c_signatures(file.read());
		