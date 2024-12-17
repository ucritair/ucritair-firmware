#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;

if(len(sys.argv) != 2):
	print("usage: meshgen.py");
	exit();
dir = "/";

json_path = os.path.join(dir, "meshes.json");
json_file = open(json_path, "r+");
json_data = json.load(json_file);

json_file.close();

header_path = os.path.join(dir, "mesh_assets.h");
header = open(header_path, "w");
source_path = os.path.join(dir, "mesh_assets.c");
source = open(source_path, "w");

header.write(f"// Generated from {json_path}\n");
header.write("\n");
header.write("#pragma once\n");
header.write("\n");

source.write(f"// Generated from {json_path}\n");
source.write("\n");
source.write("#include \"assets.h\"\n");
source.write("\n");

header.write("typedef struct CAT_mesh\n");
header.write("{\n");
header.write("\tconst char* path;\n");
header.write(f"\tfloat* verts;\n");
header.write("\tint n_verts;\n");
header.write(f"\tint* faces;\n");
header.write("\tint n_faces;\n");
header.write("} CAT_mesh;\n");
header.write("\n");

def serialize_mesh(path):
	with open(os.path.join(dir, path), "r") as file:
		name = pl.Path(path).stem;
		vs = [];
		fs = [];
		for line in iter(file.readline, ""):
			tokens = line.split();
			# ONLY v AND f TOKENS SUPPORTED
			match tokens:
				case ["v", x, y, z]:
					# v TOKENS ARE VEC3
					vs.append([float(x), float(y), float(z)]);
				case ["f", a, b, c]:
					f = []
					for v in [a, b, c]:
						f.append(int(v.split("/")[0])-1);
					# f TOKENS ARE JUST INT TRIPLETS
					fs.append(f);
		
		header.write(f"extern CAT_mesh {name}_mesh;\n");

		source.write(f"CAT_mesh {name}_mesh =\n");
		source.write("{\n");
		source.write(f"\t.path = \"{path}\",\n");

		source.write("\t.verts = (float[])\n");
		source.write("\t{\n");
		for (idx, v) in enumerate(vs):
			source.write(f"\t\t{v[0]}f, {v[1]}f, {v[2]}f");
			if idx < len(vs)-1:
				source.write(",");
			source.write("\n");
		source.write("\t},\n");
		source.write(f"\t.n_verts = {len(vs)},\n");

		source.write("\t.faces = (int[])\n");
		source.write("\t{\n");
		for (idx, f) in enumerate(fs):
			source.write(f"\t\t{f[0]}, {f[1]}, {f[2]}");
			if idx < len(fs)-1:
				source.write(",");
			source.write("\n");
		source.write("\t},\n");
		source.write(f"\t.n_faces = {len(fs)},\n");

		source.write("};\n\n");

for obj in json:
	serialize_mesh(obj["path"]);

header.close();
source.close();