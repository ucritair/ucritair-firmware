#!utils/catenv/bin/python

import sys;
import json;
import os;
from pathlib import Path;
import subprocess as sp;
from PIL import Image;

if(len(sys.argv) < 2):
	print("usage: spritegen.py [--all] <sprite directory>");
	exit();
sprites_dir = sys.argv[-1];

sprites_json_path = os.path.join(sprites_dir, "sprites.json");
sprites_json_file = open(sprites_json_path, "r");
sprites_json = json.load(sprites_json_file);
sprites_json_file.close();

sprite_header_path = os.path.join(sprites_dir, "sprite_assets.h");
sprite_header = open(sprite_header_path, "w");
sprite_source_path = os.path.join(sprites_dir, "sprite_assets.c");
sprite_source = open(sprite_source_path, "w");

sprite_header.write(f"// Generated from {sprites_json_path}\n");
sprite_header.write("\n");
sprite_header.write("#pragma once\n");
sprite_header.write("\n");
sprite_header.write("#include <stdint.h>\n");
sprite_header.write("#include <stdlib.h>\n");
sprite_header.write("\n");

sprite_source.write(f"// Generated from {sprites_json_path}\n");
sprite_source.write("\n");
sprite_source.write("#include \"sprite_assets.h\"\n");
sprite_source.write("\n");

sprite_header.write("typedef struct CAT_sprite\n");
sprite_header.write("{\n");
sprite_header.write("\tconst char* path;\n");
sprite_header.write("\n");
sprite_header.write(f"\tuint16_t* colour_table;\n");
sprite_header.write(f"\tuint8_t n_colours;\n");
sprite_header.write("\n");
sprite_header.write(f"\tuint8_t* runs;\n");
sprite_header.write("\tuint16_t n_runs;\n");
sprite_header.write("\n");
sprite_header.write("\tuint16_t width;\n");
sprite_header.write("\tuint16_t height;\n");
sprite_header.write("\tuint8_t frames;\n");
sprite_header.write("} CAT_sprite;\n");
sprite_header.write("\n");

def RGBA88882RGB565(c):
	if len(c) == 4 and c[3] < 128:
		return 0xdead;
	r = int((c[0] / 255) * 31);
	g = int((c[1] / 255) * 63);
	b = int((c[2] / 255) * 31);
	return (r << 11) | (g << 5) | b;

def build_colour_table(sprite):
	pal = sprite.palette.colors;
	rgb888s = pal.keys();
	rgb565s = [RGBA88882RGB565(c) for c in rgb888s];
	return rgb565s;

def rlencode(sprite):
	source = sprite.tobytes();
	runs = [[source[0], 1]];
	for i in range(1, len(source)):
		if(source[i] != runs[-1][0]):
			runs.append([source[i], 1]);
		else:
			runs[-1][1] += 1;	
	return runs;

def rldecode(runs):
	pixels = [];
	for r in runs:
		for i in range(r[1]):
			pixels.append(r[0]);
	return pixels;

total_size = 0;
def serialize_sprite(path, frames):
	name = Path(path).stem;
	sprite = Image.open(os.path.join(sprites_dir, path));
	sprite = sprite.convert("P");
	
	sprite_header.write(f"extern CAT_sprite {name}_sprite;\n");

	sprite_source.write(f"CAT_sprite {name}_sprite = \n");
	sprite_source.write("{\n");
	sprite_source.write(f"\t.path = \"{path}\",\n");
	sprite_source.write("\n");

	colour_table = build_colour_table(sprite);
	sprite_source.write("\t.colour_table = (uint16_t[])\n");
	sprite_source.write("\t{\n");
	for c in colour_table:
		sprite_source.write(f"\t\t{hex(c)},\n");
	sprite_source.write("\t},\n");
	sprite_source.write(f"\t.n_colours = {len(colour_table)},\n");
	sprite_source.write("\n");

	runs = rlencode(sprite);
	sprite_source.write("\t.runs = (uint8_t[])\n");
	sprite_source.write("\t{\n\t\t");
	for (idx, run) in enumerate(runs):
		if idx > 0 and idx % 8 == 0:
			sprite_source.write("\n\t\t");
		sprite_source.write(f"{run[0]},{run[1]},");
		if idx == len(runs)-1:
			sprite_source.write("\n");
	sprite_source.write("\t},\n");
	sprite_source.write(f"\t.n_runs = {len(runs)},\n");
	sprite_source.write("\n");
	sprite_source.write(f"\t.width = {sprite.size[0]},\n");
	sprite_source.write(f"\t.height = {sprite.size[1]},\n");
	sprite_source.write(f"\t.frames = {frames}\n");
	sprite_source.write("};\n\n");

	global total_size;
	total_size += len(colour_table) * 2 + len(runs) * 2;

if len(sys.argv) > 2 and sys.argv[-2] == "--all":
	pngs = [p for p in os.listdir(sprites_dir) if os.path.splitext(p)[1] == ".png"]
	for path in pngs:
		serialize_sprite(path, 1);
else:
	for sprite_obj in sprites_json:
		serialize_sprite(sprite_obj["path"], sprite_obj["frames"]);
print("Serialized", total_size, "bytes of colours and runs");

sprite_header.close();
sprite_source.close();
