#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;

json_file = open("data/fish.json", "r");
json_data = json.load(json_file);
json_entries = json_data['entries'];
json_file.close();

header = open("data/fish_assets.h", "w");
header.write("#pragma once\n");
header.write("\n");
header.write(f"#define FISH_COUNT {len(json_entries)}\n");
header.write("\n");
header.write("#include \"caring/cat_study.h\"\n");
header.write("\n");
for (idx, theme) in enumerate(json_entries):
	header.write(f"extern const CAT_room_theme {theme['name']}_theme;\n");
header.write("\n");
header.write("extern const CAT_room_theme* themes_list[];\n");
header.close();

def write_map(f, m, w, h):
	f.write("\t{\n");
	for y in range(h):
		f.write("\t\t");
		for x in range(w):
			idx = y * w + x;
			f.write(f"{m[idx]},");
		f.write("\n");
	f.write("\t},\n")

source = open("data/theme_assets.c", "w");
source.write("#include \"theme_assets.h\"\n");
source.write("\n");
source.write("#include \"sprite_assets.h\"\n");
source.write("\n");
for (idx, theme) in enumerate(json_entries):
	source.write(f"const CAT_room_theme {theme['name']}_theme =\n");
	source.write("{\n");
	source.write(f"\t.name = \"{theme['display_name']}\",\n");
	source.write(f"\t.wall_tiles = &{theme['wall_tiles']},\n");
	source.write(f"\t.wall_map = (uint8_t[])\n");
	write_map(source, theme['wall_map'], 15, 6);
	source.write(f"\t.floor_tiles = &{theme['floor_tiles']},\n");
	source.write(f"\t.floor_map = (uint8_t[])\n");
	write_map(source, theme['floor_map'], 15, 14);
	source.write("};\n");
	source.write("\n");
source.write("\n");
source.write("const CAT_room_theme* themes_list[] =\n");
source.write("{\n");
for (idx, theme) in enumerate(json_entries):
	source.write(f"\t&{theme['name']}_theme,\n");
source.write("};\n");
source.close();






