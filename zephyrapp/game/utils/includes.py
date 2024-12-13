#!/usr/bin/env python3

import sys;
import os;

if(len(sys.argv) != 1):
	print("usage: includes.py");
	exit();

src_dir = os.fsencode("src");
include_map = {};

for includer in os.listdir(src_dir):
	includer = includer.decode("utf-8");
	path = os.path.join("src", os.fsdecode(includer));
	include_map[includer] = [];
	if path.endswith(".h") or path.endswith(".c"):
		with open(path, "r") as file:
			lines = file.read().split("\n");
			lines = [line.strip() for line in lines];
			lines = [line.split(" ") for line in lines];
			for tokens in lines:
				if(tokens[0] == "#include"):
					included = tokens[1];
					included = included.replace("\"", "");
					included = included.replace("<", "");
					included = included.replace(">", "");
					include_map[includer].append(included);

print(include_map);
	
		