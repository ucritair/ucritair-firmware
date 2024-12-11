#!utils/catenv/bin/python

import sys;
import json;
import os;
import pathlib as pl;
import subprocess as sp;

if(len(sys.argv) != 2):
	print("usage: soundgen.py <sound directory>");
	exit();
sounds_dir = sys.argv[1];

sounds_json_path = os.path.join(sounds_dir, "sounds.json");
sounds_json_file = open(sounds_json_path, "r");
sounds_json = json.load(sounds_json_file);
sounds_json_file.close();

sound_header_path = os.path.join(sounds_dir, "sound_assets.h");
sound_header = open(sound_header_path, "w");
sound_source_path = os.path.join(sounds_dir, "sound_assets.c");
sound_source = open(sound_source_path, "w");

sound_header.write(f"// Generated from {sounds_json_path}\n");
sound_header.write("\n");
sound_header.write("#pragma once\n");
sound_header.write("\n");
sound_header.write("#include <stdint.h>\n");
sound_header.write("#include <stdlib.h>\n");
sound_header.write("\n");

sound_source.write(f"// Generated from {sounds_json_path}\n");
sound_source.write("\n");
sound_source.write("#include \"sound_assets.h\"\n");
sound_source.write("\n");

sound_header.write("typedef struct CAT_sound\n");
sound_header.write("{\n");
sound_header.write("\tconst char* path;\n");
sound_header.write(f"\tuint8_t* samples;\n");
sound_header.write("\tsize_t size;\n");
sound_header.write("} CAT_sound;\n");
sound_header.write("\n");

temp_dir = os.path.join(sounds_dir, "temp");
sp.call(f"mkdir {temp_dir}", shell=True);

def serialize_sound(path):
	in_wav_path = os.path.join(sounds_dir, path);
	out_wav_path = os.path.join(temp_dir, "out.wav");
	out_au_path = os.path.join(temp_dir, "out.au");
	out_bin_path = os.path.join(temp_dir, "out.bin");
	sp.call(f"ffmpeg -i {in_wav_path} -ac 1 -ar 32000 -c:a pcm_s16le -y {out_wav_path}", shell=True);
	sp.call(f"ffmpeg -i {out_wav_path} -y {out_au_path}", shell=True);
	sp.call(f"utils/encoder.out 23 < {out_au_path} > {out_bin_path}", shell=True);

	with open(out_bin_path, "rb") as file:
		name = pl.Path(path).stem;
		data = file.read();
		
		sound_header.write(f"extern CAT_sound {name}_sound;\n");

		sound_source.write(f"CAT_sound {name}_sound = \n");
		sound_source.write("{\n");
		sound_source.write(f"\t.path = \"{path}\",\n");

		sound_source.write("\t.samples = (uint8_t[])\n");
		sound_source.write("\t{\n");
		new_line = True;
		for (idx, b) in enumerate(data):
			if new_line:
				sound_source.write("\t\t");
				new_line = False;
			sound_source.write(str(b));
			if idx < len(data)-1:
				sound_source.write(',');
			if idx == len(data)-1 or (idx > 0 and idx % 128 == 0):
				sound_source.write('\n');
				new_line = True;
		sound_source.write("\t},\n");

		sound_source.write(f"\t.size = {len(data)}\n");
		sound_source.write("};\n\n");

for sound_obj in sounds_json:
	serialize_sound(sound_obj["path"]);

sp.call(f"trash {temp_dir}", shell=True);
sound_header.close();
sound_source.close();
