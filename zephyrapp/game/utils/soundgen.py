#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;
import subprocess as sp;

json_path = os.path.join("sounds", "sounds.json");
json_file = open(json_path, "r");
json_data = json.load(json_file);
json_entries = json_data['entries'];
json_file.close();

header_path = os.path.join("sounds", "sound_assets.h");
header = open(header_path, "w");
header.write("#pragma once\n");
header.write("\n");
header.write("#include <stdint.h>\n");
header.write("#include <stdlib.h>\n");
header.write("\n");

source_path = os.path.join("sounds", "sound_assets.c");
source = open(source_path, "w");
source.write("#include \"sound_assets.h\"\n");
source.write("\n");

header.write("typedef struct CAT_sound\n");
header.write("{\n");
header.write(f"\tuint8_t* samples;\n");
header.write("\tsize_t size;\n");
header.write("} CAT_sound;\n");
header.write("\n");
for sound in json_entries:
	header.write(f"extern CAT_sound {sound['name']}_sound;\n");
header.close();

sp.call("mkdir sounds/temp", shell=True);
for sound in json_entries:
	in_wav_path = os.path.join("sounds", sound['path']);
	out_wav_path = os.path.join("sounds/temp", "out.wav");
	out_au_path = os.path.join("sounds/temp", "out.au");
	out_bin_path = os.path.join("sounds/temp", "out.bin");
	sp.call(f"ffmpeg -i {in_wav_path} -ac 1 -ar 32000 -c:a pcm_s16le -y {out_wav_path}", shell=True);
	sp.call(f"ffmpeg -i {out_wav_path} -y {out_au_path}", shell=True);
	sp.call(f"utils/encoder.out 23 < {out_au_path} > {out_bin_path}", shell=True);
	with open(out_bin_path, "rb") as file:
		data = file.read();
		source.write(f"CAT_sound {sound['name']}_sound = \n");
		source.write("{\n");
		source.write("\t.samples = (uint8_t[])\n");
		source.write("\t{\n\t\t");
		for (idx, b) in enumerate(data):
			if idx > 0 and idx % 128 == 0:
				source.write("\n\t\t");
			source.write(f"{hex(b)},");
			if idx == len(data)-1:
				source.write("\n");
		source.write("\t},\n");
		source.write(f"\t.size = {len(data)}\n");
		source.write("};\n\n");
sp.call(f"/bin/rm -r sounds/temp", shell=True);
source.close();


