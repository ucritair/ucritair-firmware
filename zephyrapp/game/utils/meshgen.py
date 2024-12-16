#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;

if(len(sys.argv) != 2):
	print("usage: meshgen.py");
	exit();
meshes_dir = "meshes/";

meshes_json_path = os.path.join(meshes_dir, "meshes.json");
meshes_json_file = open(meshes_json_path, "r");
meshes_json = json.load(meshes_json_file);
meshes_json_file.close();

mesh_header_path = os.path.join(meshes_dir, "mesh_assets.h");
mesh_header = open(mesh_header_path, "w");
mesh_source_path = os.path.join(meshes_dir, "mesh_assets.c");
mesh_source = open(mesh_source_path, "w");

mesh_header.write(f"// Generated from {meshes_json_path}\n");
mesh_header.write("\n");
mesh_header.write("#pragma once\n");
mesh_header.write("\n");

mesh_source.write(f"// Generated from {meshes_json_path}\n");
mesh_source.write("\n");
mesh_source.write("#include \"mesh_assets.h\"\n");
mesh_source.write("\n");

mesh_header.write("typedef struct CAT_mesh\n");
mesh_header.write("{\n");
mesh_header.write("\tconst char* path;\n");
mesh_header.write(f"\tfloat* verts;\n");
mesh_header.write("\tint n_verts;\n");
mesh_header.write(f"\tint* faces;\n");
mesh_header.write("\tint n_faces;\n");
mesh_header.write("} CAT_mesh;\n");
mesh_header.write("\n");

def serialize_mesh(path):
	with open(os.path.join(meshes_dir, path), "r") as file:
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
		
		mesh_header.write(f"extern CAT_mesh {name}_mesh;\n");

		mesh_source.write(f"CAT_mesh {name}_mesh =\n");
		mesh_source.write("{\n");
		mesh_source.write(f"\t.path = \"{path}\",\n");

		mesh_source.write("\t.verts = (float[])\n");
		mesh_source.write("\t{\n");
		for (idx, v) in enumerate(vs):
			mesh_source.write(f"\t\t{v[0]}f, {v[1]}f, {v[2]}f");
			if idx < len(vs)-1:
				mesh_source.write(",");
			mesh_source.write("\n");
		mesh_source.write("\t},\n");
		mesh_source.write(f"\t.n_verts = {len(vs)},\n");

		mesh_source.write("\t.faces = (int[])\n");
		mesh_source.write("\t{\n");
		for (idx, f) in enumerate(fs):
			mesh_source.write(f"\t\t{f[0]}, {f[1]}, {f[2]}");
			if idx < len(fs)-1:
				mesh_source.write(",");
			mesh_source.write("\n");
		mesh_source.write("\t},\n");
		mesh_source.write(f"\t.n_faces = {len(fs)},\n");

		mesh_source.write("};\n\n");

for mesh_obj in meshes_json:
	serialize_mesh(mesh_obj["path"]);
mesh_header.close();
mesh_source.close();