#!/usr/bin/env python3

import abc;
import re;
import glob;
import json;

class Type(abc.ABC):
	@abc.abstractmethod
	def prototype(self):
		pass;
	@abc.abstractmethod
	def validate(self, value):
		pass;

class Primitive(Type):
	pass;
class Int(Primitive):
	def prototype(self):
		return 0;
	def validate(self, value):
		return isinstance(value, int);
	def __repr__(self):
		return "Int";
class Float(Primitive):
	def prototype(self):
		return 0;
	def validate(self, value):
		return isinstance(value, float);
	def __repr__(self):
		return "Float";
class Bool(Primitive):
	def prototype(self):
		return False;
	def validate(self, value):
		return isinstance(value, bool);
	def __repr__(self):
		return "Bool";
class String(Primitive):
	def prototype(self):
		return "";
	def validate(self, value):
		return isinstance(value, str);
	def __repr__(self):
		return "String";

class Enum(Type):
	def __init__(self, expression):
		parse_regex = r"enum|\(|,|\s|\)";
		values = re.split(parse_regex, expression);
		values = [v for v in values if len(v) > 0];
		self.values = values;
	def prototype(self):
		return self.values[0];
	def validate(self, value):
		return value in self.values;
	def __repr__(self):
		return f"Enum({", ".join(self.values)})";

class File(Type):
	def __init__(self, pattern):
		self.pattern = pattern;
		self.regex = glob.translate(self.pattern);
	def prototype(self):
		return "";
	def validate(self, value):
		return re.match(self.regex, value);
	def __repr__(self):
		return f"File({self.pattern})";

class Asset(Type):
	def __init__(self, name):
		self.name = name;
	def prototype(self):
		return "";
	def validate(self, value):
		return isinstance(value, str);
	def __repr__(self):
		return f"Asset({self.name})";

class List(Type):
	def __init__(self, T):
		self.T = T;
	def prototype(self):
		if isinstance(self.T, List):
			return [self.T.prototype()];
		else:
			return [];
	def validate(self, value):
		if not isinstance(value, list):
			return False;
		valid = True;
		for item in value:
			valid = valid and self.T.validate(item);
		return valid;
	def __repr__(self):
		return f"List({self.T})";

class Object(Type):
	def __init__(self, elements):
		self.elements = elements;
	def prototype(self):
		instance = {};
		for element in self.elements:
			instance[element.name] = element.T.prototype();
		return instance;

	def validate(self, value):
		if not isinstance(value, dict):
			return False;
	
		exclusion_pass = True;
		canon_keys = [e.name for e in self.elements];
		for key in value:
			exclusion_pass &= key in canon_keys;
		if not exclusion_pass:
			return False;

		inclusion_pass = True;
		for element in self.elements:
			inclusion_pass &= element.name in value and element.T.validate(value[element.name]);
		if not inclusion_pass:
			return False;

		return True;

	def __repr__(self):
		return f"Object({", ".join([str(e.T) for e in self.elements])})";

class Element:
	def __init__(self, name, T, attributes = [], conditions = []):
		self.name = name;
		self.T = T;
		self.attributes = attributes;
		self.conditions = conditions;
	def __repr__(self):
		return f"Element({self.name}, {self.T})";

	def get_type(self):
		return self.T;

	def has_attribute(self, attribute):
		return attribute in self.attributes;

	def prototype(self, inner = False):
		T = self.T;
		if inner:
			while isinstance(T, List):
				T = T.T;
		return T.prototype();

	def validate(self, value):
		return self.T.validate(value);
			
class Typist:
	def _search(self, element, path_tokens):
		if path_tokens == []:
			return element;
		token = path_tokens[0];
		T = element.T;
		while isinstance(T, List):
			T = T.T;
		next_element = next(e for e in T.elements if e.name == token);
		return self._search(next_element, path_tokens[1:]);

	def search(self, path_str):
		path_tokens = re.split(r"/", path_str);
		path_tokens = [t for t in path_tokens if t != ""];
		return self._search(self.root, path_tokens);

	def _head(self):
		return self.path[-1];

	def _push(self, key):
		T = self._head().T;
		while isinstance(T, List):
			T = T.T;
		matches = [e for e in T.elements if e.name == key];
		self.path.append(matches[0]);
	
	def _pop(self):
		self.path.pop(-1);
	
	def _reset(self):
		self.path = [self.root];
	
	def _navigate(self, path_tokens):
		if path_tokens == []:
			return self._head();
		next = path_tokens[0];
		self._push(next);
		self._navigate(path_tokens[1:]);

	def navigate(self, path_str):
		is_abs_path = re.match(r"\/([A-z]+\/?)*", path_str);
		if is_abs_path:
			self._reset();
		path_tokens = re.split(r"/", path_str);
		path_tokens = [t for t in path_tokens if t != ""];
		self._navigate(path_tokens);
	
	def get_head(self):
		return self._head();
	
	def get_path(self):
		path_str = "/";
		for p in self.path:
			path_str += f"{p.name}/";
		return path_str;

	def _construct_type(self, expr):
		if isinstance(expr, list):
			return List(self._construct_type(expr[0]));
	
		if isinstance(expr, dict):
			elements = [];
			for name in expr:
				element_expr = expr[name];
				element = self._construct_element(name, element_expr);
				elements.append(element);
			return Object(elements);
	
		if isinstance(expr, str):
			if expr == "int":
				return Int();
			if expr == "float":
				return Float();
			if expr == "bool":
				return Bool();
			if expr == "string":
				return String();

			if re.match(r"enum\(([A-z]+)+(\,+\s*[A-z]+)*\)", expr):
				return Enum(expr);

			if re.match(r"\*.[A-z]+", expr):
				return File(expr);

		return Asset(expr);

	def _construct_element(self, name, expr):
		T = self._construct_type(expr["type"]);
		attributes = expr["attributes"] if "attributes" in expr else [];
		conditions = expr["conditions"] if "conditions" in expr else [];
		return Element(name, T, attributes, conditions);

	def __init__(self, name, expr):
		self.root = self._construct_element(name, expr);
		self.path = [self.root];

class TypeHelper:
	def __init__(self, typist):
		self.typist = typist;
	
	def _search(self, instance, path):
		path_tokens = re.split(r"/", path);
		path_tokens = [t for t in path_tokens if t != ""];
		node = instance;
		while path_tokens != []:
			next_key = path_tokens.pop(0);
			node = node[next_key];
		return node;

	def _evaluate_conditions(self, instance, element):
		conditions = element.conditions;
		evaluation = True;
		for condition in conditions:
			evaluation &= self._search(instance, condition["key"]) == condition["value"];
		return evaluation;

	def delete(self, instance, path):
		path_tokens = re.split(r"/", path);
		path_tokens = [t for t in path_tokens if t != ""];
		node = instance;
		while len(path_tokens) > 1:
			next_key = path_tokens.pop(0);
			node = node[next_key];
		del node[path_tokens[0]];
	
	def _rectify(self, instance, node, element):
		if not isinstance(element.T, Object):
			return;
	
		for e in element.T.elements:
			if not e.name in node:
				node[e.name] = e.prototype();
		
		violations = [];
		for e in element.T.elements:
			if not self._evaluate_conditions(instance, e):
				violations.append(e.name);
		for key in violations:
			del node[key];

		violations = [];
		canon_keys = [e.name for e in element.T.elements];
		for key in node:
			if not key in canon_keys:
				violations.append(key);
		for key in violations:
			del node[key];
	
		for e in element.T.elements:
			if e.name in node:
				self._rectify(instance, node[e.name], e);			

	def prototype(self):
		root = self.typist.search("/");
		return root.prototype();

	def validate(self, instance):
		root = self.typist.search("/");
		return root.validate(instance);

	def rectify(self, instance):
		root = self.typist.search("/");
		self._rectify(instance, instance, root);

file = open("editor/type.json", "r");
json_data = json.load(file);
file.close();
type_name = next(n for n in json_data.keys() if n != "instances");
typist = Typist(type_name, json_data[type_name]);
helper = TypeHelper(typist);
instance = helper.prototype();
print(instance);
print(helper.validate(instance));
print(helper.validate({}));
helper.rectify(instance)
print(instance);