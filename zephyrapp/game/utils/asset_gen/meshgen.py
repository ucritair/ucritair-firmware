#!/usr/bin/env python3

import assetgen;
from pathlib import Path;

triptych = assetgen.Triptych(
	"meshes/meshes.json",
	"meshes/mesh_assets.h",
	"meshes/mesh_assets.c"
);
data = triptych.json_data;
hdr = triptych.header_writer;
src = triptych.source_writer;

hdr.write_header(["\"cat_poly.h\""]);
src.write_header(["\"mesh_assets.h\""]);

for instance in data.instances:
	path = Path("meshes")/instance["path"];
	vs = [];
	fs = [];
	with open(path, "r") as file:
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
	
	src.start_body(instance["name"]);

	src.start_block("verts", "float[]");
	for (idx, v) in enumerate(vs):
		src.literal(f"{v[0]}f, {v[1]}f, {v[2]}f");
	src.end_block();
	src.variable("n_verts", len(vs));

	src.start_block("faces", "uint16_t[]");
	for (idx, f) in enumerate(fs):
		src.literal(f"{f[0]}, {f[1]}, {f[2]}");
	src.end_block();
	src.variable("n_faces", len(fs));

	src.end_body();
