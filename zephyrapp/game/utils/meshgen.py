#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;

######################################################
# MESHES

json_path = os.path.join("meshes", "meshes.json");
json_file = open(json_path, "r+");
json_data = json.load(json_file)
json_entries = json_data['entries'];
json_file.close();

header_path = os.path.join("meshes", "mesh_assets.h");
header = open(header_path, "w");
header.write("#pragma once\n");
header.write("\n");
header.write("#include \"cat_render.h\"\n");
header.write("\n");
for mesh in json_entries:
	header.write(f"extern CAT_mesh {mesh['name']}_mesh;\n");
header.close();

source_path = os.path.join("meshes", "mesh_assets.c");
source = open(source_path, "w");
source.write("#include \"mesh_assets.h\"\n");
source.write("\n");
for mesh in json_entries:
	with open(os.path.join("meshes", mesh['path']), "r") as file:
		vs = [];
		fs = [];
		for line in iter(file.readline, ""):
			tokens = line.split();
			match tokens:
				case ['v', x, y, z]:
					vs.append([float(x), float(y), float(z)]);
				case ['f', a, b, c]:
					f = []
					for v in [a, b, c]:
						f.append(int(v.split("/")[0])-1);
					fs.append(f);
		
		source.write(f"CAT_mesh {mesh['name']}_mesh =\n");
		source.write("{\n");
		source.write("\t.verts = (float[])\n");
		source.write("\t{\n");
		for (idx, v) in enumerate(vs):
			source.write(f"\t\t{v[0]}f, {v[1]}f, {v[2]}f");
			if idx < len(vs)-1:
				source.write(",");
			source.write("\n");
		source.write("\t},\n");
		source.write(f"\t.n_verts = {len(vs)},\n");

		source.write("\t.faces = (uint8_t[])\n");
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


######################################################
# MESH2Ds

json_file = open("meshes/mesh2ds.json", "r");
json_data = json.load(json_file);
json_entries = json_data['entries'];
json_file.close();

header = open("meshes/mesh2d_assets.h", "w");
header.write("#pragma once\n");
header.write("\n");
header.write(f"#define MESH2D_COUNT {len(json_entries)}\n");
header.write("\n");
header.write("#include \"cat_render.h\"\n");
header.write("\n");
for (idx, mesh2d) in enumerate(json_entries):
	header.write(f"extern const CAT_mesh2d {mesh2d['name']}_mesh2d;\n");
header.write("\n");
header.write("extern const CAT_mesh2d* mesh2d_list[];\n");
header.close();

def write_verts(f, verts, count):
	f.write("\t.verts = (uint8_t[])\n");
	f.write("\t{\n");
	i = 0;
	while i < count:
		f.write(f"\t\t{verts[i*2+0]}, {verts[i*2+1]},\n");
		i += 1;
	f.write("\t},\n")
	f.write(f"\t.vert_count = {count},\n");

def write_edges(f, edges, count):
	f.write("\t.edges = (uint16_t[])\n");
	f.write("\t{\n");
	i = 0;
	while i < count:
		f.write(f"\t\t{edges[i*2+0]}, {edges[i*2+1]},\n");
		i += 1;
	f.write("\t},\n")
	f.write(f"\t.edge_count = {count},\n");

source = open("meshes/mesh2d_assets.c", "w");
source.write("#include \"mesh2d_assets.h\"\n");
source.write("\n");
for (idx, mesh2d) in enumerate(json_entries):
	source.write(f"const CAT_mesh2d {mesh2d['name']}_mesh2d =\n");
	source.write("{\n");
	write_verts(source, mesh2d['verts'], mesh2d['vert_count']);
	write_edges(source, mesh2d['edges'], mesh2d['edge_count']);
	source.write("};\n");
	source.write("\n");
source.write("\n");
source.write("const CAT_mesh2d* mesh2d_list[] =\n");
source.write("{\n");
for (idx, mesh2d) in enumerate(json_entries):
	source.write(f"\t&{mesh2d['name']}_mesh2d,\n");
source.write("};\n");
source.close();






