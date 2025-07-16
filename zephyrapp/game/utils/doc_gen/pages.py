#!/usr/bin/env python3

from pathlib import Path;
import re;

in_root = Path("utils/doc_gen/pages").absolute();
out_root = Path("docs/").absolute();

class LeafPage:
	def __init__(self, path):
		self.in_path = path;
		parent = path.relative_to(in_root).parent;
		name = path.stem;
		self.out_path = (out_root/parent/name).with_suffix(".html");
		self.rel_path = self.out_path.relative_to(out_root);

		text = path.read_text();
		line = re.search(r"#\s*PAGE\s*\(.*\)\n", text).group();
		expr = re.search(r"PAGE\s*\(.*\)", line).group();
		tokens = re.split(r"\s|\(|\)", expr);
		tokens = [t for t in tokens if len(t) > 0];
		self.title = tokens[1];

class NodePage:
	def __init__(self, path):
		self.in_path = path;
		self.rel_path = path.relative_to(in_root);
		self.out_path = out_root/self.rel_path;
		self.children = [];
	
		self.title = self.rel_path.stem.title();

	def add_child(self, child):
		self.children.append(child);

	def indexed(self):
		for child in self.children:
			if child.out_path.name == "index.html":
				return True;
		return False;

tree = NodePage(in_root);

def __construct_tree(node, path):
	for child_path in path.iterdir():
		if not child_path.is_dir() and child_path.suffix == ".py":
			node.add_child(LeafPage(child_path));
	for child_path in path.iterdir():
		if child_path.is_dir() and not child_path.stem.startswith("__"):
			child_node = NodePage(child_path);
			node.add_child(child_node);
			__construct_tree(child_node, child_path);

def contruct_tree():
	__construct_tree(tree, in_root);
