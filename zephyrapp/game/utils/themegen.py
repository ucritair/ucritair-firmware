#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;

json_file = open("data/themes.json", "r");
json_data = json.load(json_file);
json_entries = json_data["entries"];
json_file.close();

header = open("data/theme_assets.h", "w");
header.write("#pragma once\n");
header.write("\n");
header.write("#include \"cat_room.h\"\n");
header.write("\n");
for (idx, theme) in enumerate(json_entries):
	header.write(f"extern const CAT_room_theme {theme["name"]}_theme;\n");
header.close();

def write_map(f, m, w, h):
	f.write("\t{\n");
	for y in range(h):
		f.write("\t\t");
		for x in range(w):
			idx = y * h + w;
			f.write(f"{m[idx]},");
		f.write("\n");
	f.write("\t},\n")

source = open("data/theme_assets.c", "w");
source.write("#include \"theme_assets.h\"\n");
source.write("\n");
source.write("#include \"sprite_assets.h\"\n");
source.write("\n");
for (idx, theme) in enumerate(json_entries):
	source.write(f"const CAT_room_theme {theme["name"]}_theme =\n");
	source.write("{\n");
	source.write(f"\t.name = \"{theme["display_name"]}\",\n");
	source.write(f"\t.wall_tiles = &{theme["wall_tiles"]},\n");
	source.write(f"\t.wall_map = (uint8_t[])\n");
	write_map(source, theme["wall_map"], 15, 6);
	source.write(f"\t.floor_tiles = &{theme["floor_tiles"]},\n");
	source.write(f"\t.floor_map = (uint8_t[])\n");
	write_map(source, theme["floor_map"], 15, 14);
	source.write("};\n");
	source.write("\n");
source.close();






