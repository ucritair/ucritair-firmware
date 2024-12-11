#!utils/catenv/bin/python

import sys;
import json;
import os;
from pathlib import Path;
import subprocess as sp;
from PIL import Image;

if(len(sys.argv) != 2):
	print("usage: spritegen.py <sprite directory>");
	exit();
sprites_dir = sys.argv[1];

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
sprite_header.write(f"\tuint16_t* pixels;\n");
sprite_header.write("\tint width;\n");
sprite_header.write("\tint height;\n");
sprite_header.write("\tint frames;\n");
sprite_header.write("} CAT_sprite;\n");
sprite_header.write("\n");

temp_dir = os.path.join(sprites_dir, "temp");
sp.call(f"mkdir {temp_dir}", shell=True);

def serialize_sprite(sprite_obj):
	path = sprite_obj["path"];
	name = Path(path).stem;

	sprite = Image.open(os.path.join(sprites_dir, path));
	sprite = sprite.convert("P");
	data = sprite.tobytes();
		
	sprite_header.write(f"extern CAT_sprite {name}_sprite;\n");

	sprite_source.write(f"CAT_sprite {name}_sprite = \n");
	sprite_source.write("{\n");
	sprite_source.write(f"\t.path = \"{path}\",\n");

	sprite_source.write("\t.pixels = (uint8_t[])\n");
	sprite_source.write("\t{\n");
	new_line = True;
	for (idx, b) in enumerate(data):
		if new_line:
			sprite_source.write("\t\t");
			new_line = False;
		sprite_source.write(str(b));
		if idx == len(data)-1:
			sprite_source.write('\n');
		else:
			sprite_source.write(',');
			if idx == len(data)-1 or (idx > 0 and (idx+1) % sprite.size[0] == 0):
				sprite_source.write('\n');
				new_line = True;
	sprite_source.write("\t},\n");
	sprite_source.write(f"\t.width = {sprite.size[0]},\n");
	sprite_source.write(f"\t.height = {sprite.size[1]},\n");
	sprite_source.write(f"\t.frames = {sprite_obj["frames"]}\n");
	sprite_source.write("};\n\n");

for sprite_obj in sprites_json:
	serialize_sprite(sprite_obj);

sp.call(f"trash {temp_dir}", shell=True);
sprite_header.close();
sprite_source.close();
