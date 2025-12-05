import json;
from pathlib import Path;
import copy;

class AssetJSON:
	def __init__(self, json_data):
		self.type_name = next(k for k in json_data.keys() if k != "instances");
		self.ctype_name = f"CAT_{self.type_name}";
		self.code = json_data["code"] if "code" in json_data else None;
		self.instances = copy.deepcopy(json_data["instances"]);

class AssetHeaderWriter:
	def __init__(self, data, file):
		self.data = data;
		self.file = file;
	
	def write(self, string="", end="\n"):
		self.file.write(string+end);
	
	def write_header(self, dependencies=[]):
		self.write("#pragma once");
		self.write();
		for dependency in dependencies:
			self.write(f"#include {dependency}");
		self.write();
		for instance in self.data.instances:
			code_prefix = f"{self.data.code}_" if self.data.code != None else "";
			self.write(f"extern const {self.data.ctype_name} {code_prefix}{instance["name"]};");
		self.write();

	def write_list(self):
		self.write(f"extern const {self.data.ctype_name}* {self.data.ctype_name}_list[];");
		self.write(f"#define {self.data.ctype_name.upper()}_COUNT {len(self.data.instances)}");
		self.write();

	def __del__(self):
		self.file.close();

class AssetSourceWriter:
	def __init__(self, data, file):
		self.data = data;
		self.file = file;
		self.indent = 0;
	
	def _write_indent(self):
		self.file.write("\t" * self.indent);
	
	def write(self, string="", end="\n"):
		self._write_indent();
		self.file.write(string+end);
	
	def write_header(self, dependencies=[]):
		for dependency in dependencies:
			self.write(f"#include {dependency}");
		self.write();
	
	def start_body(self, name, type=None):
		if type == None:
			type = self.data.ctype_name;
		code_prefix = f"{self.data.code}_" if self.data.code != None else "";
		self.write(f"const {type} {code_prefix}{name} =");
		self.write("{");
		self.indent += 1;
	
	def start_block(self, name=None, type=None):
		preamble = "";
		if name != None and len(name) > 0:
			preamble += f".{name} = ";
		if type != None and len(type) > 0:
			preamble += f"({type})";
		if len(preamble) > 0:
			self.write(preamble);
		self.write("{");
		self.indent += 1;

	def end_block(self):
		self.indent -= 1;
		self.write("},");
		
	def variable(self, name, value, type=None):
		self.write(f".{name} = {f"({type}) " if type != None else ""}{value},");
	
	def literal(self, value):
		self.write(f"{value},");
	
	def end_body(self):
		self.indent -= 1;
		self.write("};");
	
	def write_list(self):
		code_prefix = f"{self.data.code}_" if self.data.code != None else "";
		self.write(f"const {self.data.ctype_name}* {self.data.ctype_name}_list[] =");
		self.write("{");
		self.indent += 1;
		for instance in self.data.instances:
			self.literal(f"&{code_prefix}{instance["name"]}");
		self.indent -= 1;
		self.write("};");

	def __del__(self):
		self.file.close();

class Triptych:
	def __init__(self, json_path, header_path=None, source_path=None):
		self.json_path = json_path;
		json_file = open(json_path, "r");
		self.json_data = AssetJSON(json.load(json_file));
		json_file.close();

		if header_path == None:
			header_path = f"cat_{self.type_name}_assets.h";
		self.header_path = header_path;
		self.header_writer = AssetHeaderWriter(self.json_data, open(self.header_path, "w"));

		if source_path == None:
			source_path = f"cat_{self.type_name}_assets.c";
		self.source_path = source_path;
		self.source_writer = AssetSourceWriter(self.json_data, open(self.source_path, "w"));

	