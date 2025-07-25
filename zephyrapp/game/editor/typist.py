#!/usr/bin/env python3

import json;
import copy;
import re;

primitives = set([
	"int",
	"float",
	"bool",
	"string"
]);
primitive_defaults = {
	"int" : 0,
	"float" : 0,
	"bool" : False,
	"string" : ""
};

enum_match_regex = r"enum\(([A-z]+)+(\,+\s*[A-z]+)*\)";
enum_parse_regex = r"enum|\(|,|\s|\)";
def match_enum(s):
	return re.match(enum_match_regex, s);
def parse_enum(s):
	tokens = re.split(enum_parse_regex, s);
	tokens = [t for t in tokens if t != ""];
	return tokens;

file_match_regex = r"\*.[A-z]+";
def match_file(s):
	return re.match(file_match_regex, s);

class Typist:
	def __init__(self, schema):
		self.root = copy.deepcopy(schema);

	def __spawn(self, node):
		if isinstance(node, dict):
			result = {};
			for (key, child) in node.items():
				result[key] = self.__spawn(child["type"]);
			return result;
	
		if isinstance(node, list):
			return [self.__spawn(node[0])];
	
		if node in primitives:
			return primitive_defaults[node];

		if match_enum(node):
			return parse_enum(node)[0];
	
		if match_file(node):
			return "";

		return None;

	def spawn(self):
		return self.__spawn(self.root);

	def __verify(self, node, schema_node):
		for (key, child) in schema_node.items():
			if not key in node:
				node[key] = self.__spawn(child["type"]);

	def verify(self, node):
		verified = copy.deepcopy(node);
		self.__verify(verified, self.root);
		return verified;

node_path_match_regex = r"[A-z]+(\/[A-z]+)*";
node_abspath_match_regex = r"\/([A-z]+\/?)*";
node_path_parse_regex = r"/";
def parse_node_path(s):
	tokens = re.split(node_path_parse_regex, s);
	tokens = [t for t in tokens if t != ""];
	return tokens;

class TypeExplorer:
	def __init__(self, schema):
		self.root = schema;
		self.path = [self.root];
		self.path_string = [];
	
	def __head(self):
		return self.path[-1];

	def __push(self, key):
		self.path.append(self.__head()[key]["type"]);
		self.path_string.append(key);

	def __pop(self):
		self.path.pop(-1);
		self.path_string.pop(-1);
	
	def __reset(self):
		self.path = [self.root];
		self.path_string = [];

	def check(self, key):
		return key in self.__head();

	def get_type(self, key):
		return self.__head()[key]["type"];

	def has_permission(self, key, perm):
		return perm in self.__head()[key]["permissions"];

	def __navigate_to(self, node, path):
		if path == []:
			return node;
		head = path[0];
		self.__push(head);
		return self.__navigate_to(node[head]["type"], path[1:]);

	def navigate_to(self, path_str):
		is_abs_path = re.match(node_abspath_match_regex, path_str);
		is_path = is_abs_path or re.match(node_path_match_regex, path_str);
		if not is_abs_path and not is_path:
			return None;
		if is_abs_path:
			self.__reset();
		path = parse_node_path(path_str);
		print(path);
		return self.__navigate_to(self.__head(), path);

	def get_path_string(self):
		string = "/";
		for part in self.path_string:
			string += f"{part}/";
		return string;

with open("data/items.json") as file:
	json_data = json.load(file);
	typist = Typist(json_data["schema"]);
	instance = typist.spawn();
	instance = typist.verify(instance);
	print(instance);

	explorer = TypeExplorer(json_data["schema"]);
	explorer.navigate_to("/tool_data/food_data");
	print(explorer.get_path_string());
	explorer.navigate_to("food_group");
	print(explorer.get_path_string());
	explorer.navigate_to("/");
	print(explorer.get_path_string());
	