#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;

json_file = open("data/fish.json", "r");
json_data = json.load(json_file);
json_entries = json_data['instances'];
json_file.close();

header = open("data/fish_assets.h", "w");
header.write("#pragma once\n");
header.write("\n");
header.write(f"#define FISH_COUNT {len(json_entries)}\n");
header.write("\n");
header.write("#include \"caring/cat_study.h\"\n");
header.write("\n");
for (idx, fish) in enumerate(json_entries):
	header.write(f"extern const CAT_fish {fish['name']}_fish;\n");
header.write("\n");
header.write("extern const CAT_fish* fish_list[];\n");
header.close();

source = open("data/fish_assets.c", "w");
source.write("#include \"fish_assets.h\"\n");
source.write("#include \"mesh2d_assets.h\"\n");
source.write("\n");
for (idx, fish) in enumerate(json_entries):
	source.write(f"const CAT_fish {fish['name']}_fish =\n");
	source.write("{\n");
	source.write(f"\t.name = \"{fish['display_name']}\",\n");
	source.write(f"\t.proverb = \"{fish['proverb']}\",\n");
	source.write(f"\t.grade_constraint = {fish['grade_constraint']},\n");
	source.write("\n");
	source.write(f"\t.min_length = {fish['min_length']},\n");
	source.write(f"\t.max_length = {fish['max_length']},\n");
	source.write(f"\t.min_lustre = {fish['min_lustre']},\n");
	source.write(f"\t.max_lustre = {fish['max_lustre']},\n");
	source.write(f"\t.min_wisdom = {fish['min_wisdom']},\n");
	source.write(f"\t.max_wisdom = {fish['max_wisdom']},\n");
	source.write("\n");
	source.write(f"\t.mesh = &{fish['mesh']}_mesh2d,\n");
	source.write("};\n");
	source.write("\n");
source.write("\n");
source.write("const CAT_fish* fish_list[] =\n");
source.write("{\n");
for (idx, fish) in enumerate(json_entries):
	source.write(f"\t&{fish['name']}_fish,\n");
source.write("};\n");
source.close();






