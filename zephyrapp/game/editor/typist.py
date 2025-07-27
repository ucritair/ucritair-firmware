#!/usr/bin/env python3

import json;
import copy;
import re;

_primitives = set([
	"int",
	"float",
	"bool",
	"string"
]);
_primitive_defaults = {
	"int" : 0,
	"float" : 0,
	"bool" : False,
	"string" : ""
};
def is_primitive(T):
	return isinstance(T, str) and T in _primitives;

_enum_match_regex = r"enum\(([A-z]+)+(\,+\s*[A-z]+)*\)";
_enum_parse_regex = r"enum|\(|,|\s|\)";
def _match_enum(s):
	return re.match(_enum_match_regex, s);
def parse_enum(s):
	tokens = re.split(_enum_parse_regex, s);
	tokens = [t for t in tokens if t != ""];
	return tokens;
def is_enum(T):
	return isinstance(T, str) and _match_enum(T);

_filetype_match_regex = r"\*.[A-z]+";
def _match_filetype(s):
	return re.match(_filetype_match_regex, s);
def is_filetype(T):
	return isinstance(T, str) and _match_filetype(T);

class Typist:
	def __init__(self, schema):
		self.root = copy.deepcopy(schema);

	def _spawn(self, node):
		if isinstance(node, dict):
			result = {};
			for (key, child) in node.items():
				result[key] = self._spawn(child["type"]);
			return result;
	
		if isinstance(node, list):
			return [self._spawn(node[0])];
	
		if node in _primitives:
			return _primitive_defaults[node];

		if _match_enum(node):
			return parse_enum(node)[0];
	
		if _match_filetype(node):
			return "";

		return None;

	def spawn(self):
		return self._spawn(self.root);

	def _validate(self, node, schema_node, clean=False):
		for (key, child) in schema_node.items():
			if not key in node:
				node[key] = self._spawn(child["type"]);
		if clean:
			trash = [];
			for key in node:
				if not key in schema_node:
					trash.append(key);
			for key in trash:
				del node[key];

	def validate(self, node, clean=False):
		verified = copy.deepcopy(node);
		self._validate(verified, self.root, clean);
		return verified;

_node_path_match_regex = r"[A-z]+(\/[A-z]+)*";
_node_abspath_match_regex = r"\/([A-z]+\/?)*";
_node_path_parse_regex = r"/";
def _match_node_path(s):
	return re.match(_node_path_match_regex, s);
def _match_node_abspath(s):
	return re.match(_node_abspath_match_regex, s);
def _parse_node_path(s):
	tokens = re.split(_node_path_parse_regex, s);
	tokens = [t for t in tokens if t != ""];
	return tokens;

class TypeExplorer:
	def __init__(self, schema):
		self.root = copy.deepcopy(schema);
		self.path = [self.root];
		self.path_string = [];
	
	def _head(self):
		return self.path[-1];

	def _at_root(self):
		return len(self.path) == 1;

	def _push(self, key):
		if self._at_root():
			self.path.append(self._head()[key]);
		else:
			self.path.append(self._head()["type"][key]);
		self.path_string.append(key);

	def _pop(self):
		self.path.pop(-1);
		self.path_string.pop(-1);
	
	def _reset(self):
		self.path = [self.root];
		self.path_string = [];

	def get_type(self):
		if self._at_root():
			return self._head();
		elif isinstance(self._head(), list):
			return self._head()[0];
		return self._head()["type"];

	def check(self, key):
		return key in self.get_type();

	def has_permission(self, perm):
		if self._at_root():
			return False;
		return perm in self._head()["permissions"];

	def _navigate_to(self, path):
		if path == []:
			return self._head();
		next = path[0];
		self._push(next);
		self._navigate_to(path[1:]);

	def navigate_to(self, path_str):
		is_abs_path = _match_node_abspath(path_str);
		is_path = is_abs_path or _match_node_path(path_str);
		if not is_abs_path and not is_path:
			return;
		if is_abs_path:
			self._reset();
		path = _parse_node_path(path_str);
		self._navigate_to(path);

	def back(self):
		self._pop();

	def get_path(self):
		string = "/";
		for part in self.path_string:
			string += f"{part}/";
		return string;


file = open("data/scenes.json", "r");
data = json.load(file);
file.close();
schema = data["schema"];
entries = data["entries"];
typist = Typist(schema);
type_explorer = TypeExplorer(schema);
entry = typist.spawn(); #entries[0];
entry = typist.validate(entry, clean=True);

print(entry);

def traverse(node, level):
	print(level, type_explorer.get_path());
	print(node);
	T = type_explorer.get_type();
	if isinstance(T, dict):
		for key in node:
			type_explorer.navigate_to(key);
			traverse(node[key], level+1);
			type_explorer.back();
	elif isinstance(T, list):
		for item in node:
			traverse(item, level+1);

traverse(entry, 0);
	