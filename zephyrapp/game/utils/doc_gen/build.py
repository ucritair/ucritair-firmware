#!/usr/bin/env python3

import importlib.util
import os;
import pages;
from pathlib import Path;
import importlib;
import sys;
from html_writer import HTMLWriter;
import common;
import thumbnails;

build_root = pages.out_root;

thumbnails.load(Path("sprites"), build_root/Path("images/thumbnails"));
pages.contruct_tree();

def prepare_path():
	path = os.environ.get("PYTHONPATH", str(build_root)).split(os.pathsep);
	if build_root not in path:
		path.append(str(build_root));
	os.environ["PYTHONPATH"] = os.pathsep.join(path);

def import_module(path):
	name = path.stem;
	spec = importlib.util.spec_from_file_location(name, path);
	module = importlib.util.module_from_spec(spec);
	sys.modules[name] = module;
	spec.loader.exec_module(module);
	return module;

def build_tree(node):
	if isinstance(node, pages.LeafPage):
		print(f"Generating page {node.out_path} from {node.in_path}...");
		module = import_module(node.in_path);
		os.makedirs(os.path.dirname(node.out_path), exist_ok=True);
		html = HTMLWriter(node.out_path);
		html.start(node.title);
		module.build(html);
		html.end();
	
	elif isinstance(node, pages.NodePage):
		if not node.indexed():
			os.makedirs(os.path.dirname(node.out_path), exist_ok=True);
			html = HTMLWriter(node.out_path/"index.html");
			html.start(node.title);
			common.title(html, node.title);
			html.image("/images/banner_1.png", style=f"height: 35vh;");
			common.navigator(html, node);
			html.end();	
		for child in node.children:
			build_tree(child);

build_tree(pages.tree);

