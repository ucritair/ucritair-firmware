#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;
import re;
from PIL import Image;

source_file = open("src/cat_sprite.c", "r");
json_file = open("sprites/sprites.json", "w");

lines = source_file.readlines();
lines = [l.strip() for l in lines];
lines = [l for l in lines if l.startswith("INIT") or l.startswith("COPY")];

root = [];
for (idx, line) in enumerate(lines):
	strtup = line[line.find("(")+1:line.rfind(")")];
	tokens = strtup.split(",");
	tokens = [t.strip().strip("\"") for t in tokens];

	mode = "init" if line.startswith("INIT") else "copy";
	data = {};
	data["mode"] = mode;
	data["id"] = idx;
	data["name"] = tokens[0];
	if mode == "init":
		data["path"] = tokens[1];
		data["frames"] = int(tokens[2]);
		sprite = Image.open(data["path"]);
		data["width"] = sprite.size[0];
		data["height"] = sprite.size[1] // data["frames"];
	else:
		data["source"] = next(d["id"] for d in root if d["name"] == tokens[1]);
	root.append(data);

json_data = json.dumps(root, indent=4);
json_file.write(json_data);
json_file.close();

source_file.close();
