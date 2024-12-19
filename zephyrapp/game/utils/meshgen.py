#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;

json_path = os.path.join("meshes", "meshes.json");
json_file = open(json_path, "r+");
json_data = json.load(json_file)
json_entries = json_data["entries"];
json_file.close();

header_path = os.path.join("meshes", "mesh_assets.h");
header = open(header_path, "w");
header.write("#pragma once\n");
header.write("\n");
header.write("typedef struct CAT_mesh\n");
header.write("{\n");
header.write("\tconst char* path;\n");
header.write(f"\tfloat* verts;\n");
header.write("\tint n_verts;\n");
header.write(f"\tint* faces;\n");
header.write("\tint n_faces;\n");
header.write("} CAT_mesh;\n");
header.write("\n");
for mesh in json_entries:
	header.write(f"extern CAT_mesh {mesh["name"]}_mesh;\n");
header.close();

source_path = os.path.join("meshes", "mesh_assets.c");
source = open(source_path, "w");
source.write("#include \"mesh_assets.h\"\n");
source.write("\n");
for mesh in json_entries:
	with open(os.path.join("meshes", mesh["path"]), "r") as file:
		vs = [];
		fs = [];
		for line in iter(file.readline, ""):
			tokens = line.split();
			match tokens:
				case ["v", x, y, z]:
					vs.append([float(x), float(y), float(z)]);
				case ["f", a, b, c]:
					f = []
					for v in [a, b, c]:
						f.append(int(v.split("/")[0])-1);
					fs.append(f);
		
		source.write(f"CAT_mesh {mesh["name"]}_mesh =\n");
		source.write("{\n");
		source.write(f"\t.path = \"{mesh["path"]}\",\n");

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
source.close();