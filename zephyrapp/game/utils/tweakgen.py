#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;

json_file = open("data/tweaking.json", "r");
json_data = json.load(json_file);
json_file.close();

header = open("data/stats.h", "w");
header.write("#pragma once\n");
header.write("\n");
header.write(f"#define CAT_MAX_LEVEL {json_data["max_level"]}\n");
header.write("\n");
header.write("extern const int level_cutoffs[];\n");
header.close();

source = open("data/stats.c", "w");
source.write("#include \"stats.h\"\n");
source.write("\n");

source.write("const int level_cutoffs[] =\n");
source.write("{\n\t");
coeffs = json_data["base_level_curve"];
for x in range(25):
	curve = int(coeffs[0] + coeffs[1] * x);
	source.write(f"{curve}, ");
source.write("\n\t");
coeffs = json_data["advanced_level_curve"];
for x in range(25, json_data["max_level"]):
	curve = int(coeffs[0] + coeffs[1] * x + coeffs[2] * x**2 + coeffs[3] * x**3);
	source.write(f"{curve}, ");
source.write("\n};\n");
source.write("\n");

source.close();






