import json;
from pathlib import Path;
import copy;

class Database:
	def __init__(self):
		self.bank = {};

	def add_document(self, path):
		path = Path(path);
		file = open(path, "r");

		data = json.load(file);
		keys = data.keys();
		type = next(k for k in keys if k != "instances");
		instances = data["instances"];

		self.bank[type] = {};
		for instance in instances:
			self.bank[type][instance["name"]] = instance;

	def _search(self, node, path):
		if path == []:
			return node;
		return self._search(node[path[0]], path[1:]);
		
	def search(self, path):
		path = path.split("/");
		return self._search(self.bank, path);

class Triptych:
	def __init__(self, path):
		path = Path(path);
		self.parent_dir = path.parent;

		json_file = open(path, "r");
		json_data = json.load(json_file);
		json_file.close();

		self.type_name = next(k for k in json_data.keys() if k != "instances");
		self.instances = copy.deepcopy(json_data["instances"]);
		self.ctype_name = f"CAT_{self.type_name}";
		
		self.header_path = self.parent_dir/Path(f"{self.type_name}_assets.h");
		self.header_file = open(self.header_path, "w");
		
		self.source_path = self.parent_dir/Path(f"{self.type_name}_assets.c");
		self.source_file = open(self.source_path, "w");
		self.source_indent_level = 0;

	def __del__(self):
		self.header_file.close();
		self.source_file.close();
	
	def hwrite(self, string="\n"):
		self.header_file.write(string);

	def header_intro(self, dependencies=[]):
		self.hwrite("#pragma once\n");
		self.hwrite();
		for dependency in dependencies:
			self.hwrite(f"#include {dependency}\n");
		self.hwrite();
		for instance in self.instances:
			self.hwrite(f"extern const {self.ctype_name} {instance["name"]};\n");
		self.hwrite();
	
	def swrite(self, string="\n", no_indent=False):
		indent = "";
		if not no_indent:
			indent = "\t" * self.source_indent_level;
		self.source_file.write(indent + string);
	
	def source_indent(self):
		self.source_indent_level += 1;
	
	def source_unindent(self):
		self.source_indent_level -= 1;

	def source_intro(self, dependencies=[]):
		self.swrite(f"#include \"{self.header_path.name}\"\n");
		for dependency in dependencies:
			self.swrite(f"#include {dependency}\n");
		self.swrite();

	def begin_asset_def(self, instance):
		self.swrite(f"const {self.ctype_name} {instance["name"]} = {{\n");
		self.source_indent();
	
	def end_asset_def(self):
		self.source_unindent();
		self.swrite("};\n");

	