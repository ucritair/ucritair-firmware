#!/usr/bin/env python3

import os
import os.path
import sys
from dataclasses import dataclass
import json
from PIL import Image;
from pathlib import Path;
import math;

json_file = open("sprites/sprites.json", "r+");
json_data = json.load(json_file);
sprite_objects = json_data['instances'];
for (i, sprite) in enumerate(sprite_objects):
	# Ensure correct JSON
	sprite['id'] = i;
	sprite['frames'] = max(sprite['frames'], 1);
	image = Image.open(os.path.join("sprites", sprite['path']));
	sprite['width'] = image.size[0];
	sprite['height'] = image.size[1] // sprite['frames'];
	image.close();
json_file.seek(0);
json_file.truncate();
json_file.write(json.dumps(json_data, indent=4));
json_file.close();

json_file = open("sprites/tinysprites.json", "r+");
json_data = json.load(json_file);
tinysprite_objects = json_data['instances'];
for (i, sprite) in enumerate(tinysprite_objects):
	# Ensure correct JSON
	image = Image.open(os.path.join("sprites", sprite['path']));
	sprite['width'] = image.size[0];
	sprite['height'] = image.size[1];
	sprite['stride'] = image.size[0] + (image.size[0] % 8);
	image.close();
json_file.seek(0);
json_file.truncate();
json_file.write(json.dumps(json_data, indent=4));
json_file.close();

with open("sprites/sprite_assets.h", "w") as fd:
	fd.write("#pragma once\n");
	fd.write("\n")
	fd.write("#include <stdint.h>\n");
	fd.write("#include \"cat_render.h\"\n");
	fd.write("#include \"cat_core.h\"\n");
	fd.write("\n");

	for (i, sprite) in enumerate(sprite_objects):
		fd.write(f"extern const CAT_sprite {sprite['name']};\n");
	fd.write("\n");
	fd.write(f"extern const CAT_sprite* sprite_list[];\n");
	fd.write(f"#define CAT_SPRITE_LIST_LENGTH {len(sprite_objects)}\n");
	fd.write("\n");

	for (i, sprite) in enumerate(tinysprite_objects):
		fd.write(f"extern const CAT_tinysprite tnyspr_{sprite['name']};\n");
	fd.write("\n");

def TOS_write_epaper_sprites(fd):
	for sprite in tinysprite_objects:
		texture = Image.open(os.path.join("sprites", sprite["path"])).convert("RGBA");
		width, height = texture.width, texture.height;
		padded_width = width + (width % 8);
		compatible = (padded_width*height)%8 == 0;
		if not compatible:
			print(f"[ERROR] {path} not compatible!");
			continue;

		fd.write(f"const CAT_tinysprite tnyspr_{sprite['name']} = {{\n");
		fd.write(f'\t.width = {width}, .height = {height}, .stride = {padded_width},\n');
		fd.write(f'\t.bytes = {{\n\t\t');

		texture = texture.load();
		pixels = [];
		for y in range(height):
			for x in range(padded_width):
				try:
					px = texture[x, y];
				except IndexError:
					px = (255, 255, 255, 0);
				lum = math.sqrt(px[0]*px[0] + px[1]*px[1] + px[2]*px[2]);
				alph = px[3];
				filled = lum < 128 and alph >= 128;
				pixels.append(int(filled));

		words = [];
		while pixels:
			w = 0;
			for i in range(8):
				w |= pixels.pop(0) << (7-i);
			words.append(w);

		assert len(words) == (padded_width*height)//8;

		for i, w in enumerate(words):
			fd.write(f'{hex(w)}, ');
			if (i%16 == 0) and (i != 0):
				fd.write('\n\t\t');

		fd.write('\n\t}\n};\n');
	fd.write("\n");

def TOS_RGBA88882RGB565(c):
	if len(c) == 4 and c[3] < 128:
		return 0xdead;
	r = int((c[0] / 255) * 31);
	g = int((c[1] / 255) * 63);
	b = int((c[2] / 255) * 31);
	return (r << 11) | (g << 5) | b;

def TOS_reverse_endianness(c):
	return int.from_bytes(c.to_bytes(2)[::-1]);

def TOS_tabulate_colour(image):
	pal = image.palette.colors;
	rgb888s = pal.keys();
	rgb565s = [TOS_RGBA88882RGB565(c) for c in rgb888s];
	return rgb565s;

class TOS_rle_packet:
	def __init__(self, colour, length):
		self.colour = colour;
		self.length = length;

	def tokenize(self):
		if self.length > 3:
			return [0xff, self.colour, self.length];
		else:
			return [self.colour] * self.length;

def TOS_rl_decode(stream):
	pixels = [];
	for packet in stream:
		pixels.extend([packet.colour] * packet.length);
	return pixels;

def TOS_rl_tokenize(stream):
	data = [];
	for packet in stream:
		data.extend(packet.tokenize());
	return data;

def TOS_rl_encode(image):
	pixels = list(image.tobytes());
	stream = [TOS_rle_packet(pixels[0], 1)];

	for i in range(1, len(pixels)):
		head = stream[-1];
		if pixels[i] != head.colour or head.length >= 255:
				stream.append(TOS_rle_packet(pixels[i], 1));
		else:
			head.length += 1;
	
	assert(TOS_rl_decode(stream) == pixels);
	return stream;

with open("sprites/sprite_assets.c", 'w') as fd:
	fd.write("#include \"sprite_assets.h\"\n");
	fd.write('\n');

	path_map = {};
	compression_ratio = 0;

	for (i, sprite) in enumerate(sprite_objects):
		path = sprite['path'];
		if path in path_map:
			continue;
		frame_count = sprite['frames'];

		image = Image.open(os.path.join("sprites", path)).convert("P");
		raw_size = image.size[0] * image.size[1] * 2;

		colour_table = TOS_tabulate_colour(image);
		fd.write(f"const uint16_t sprite_{i}_colour_table[] = \n");
		fd.write("{\n");
		for c in colour_table:
			if c != 0xdead:
				c = TOS_reverse_endianness(c);
			fd.write(f"\t{hex(c)},\n");
		fd.write("};\n");
		fd.write("\n");

		frame_width = image.size[0];
		frame_height = image.size[1] // frame_count;
		compressed_size = 0;
		for j in range(frame_count):
			l = 0;
			r = frame_width;
			t = frame_height * j;
			b = t + frame_height;
			frame = image.crop((l, t, r, b));
			stream = TOS_rl_encode(frame);
			tokens = TOS_rl_tokenize(stream);	

			fd.write(f"const uint8_t sprite_{i}_frame_{j}[] = \n");
			fd.write("{\n\t");
			for (k, token) in enumerate(tokens):
				if k > 0 and k % 32 == 0:
					fd.write("\n\t");
				fd.write(f"{hex(token)},");
				if k == len(tokens)-1:
					fd.write("\n");
			compressed_size += len(tokens);
			fd.write("};\n");
			fd.write("\n");

		fd.write(f"const uint8_t* sprite_{i}_frames[] = \n");
		fd.write("{\n");
		for j in range(frame_count):
			fd.write(f"\tsprite_{i}_frame_{j},\n");
		fd.write("};\n");
		fd.write("\n");

		path_map[path] = i;
		compression_ratio += raw_size / compressed_size;
	
	for (i, sprite) in enumerate(sprite_objects):
		fd.write(f"const CAT_sprite {sprite['name']} =\n");
		fd.write("{\n");
		fd.write(f"\t.id = {sprite['id']},\n");

		data_idx = path_map[sprite['path']];
		fd.write(f"\t.colour_table = sprite_{data_idx}_colour_table,\n");
		fd.write(f"\t.frames = sprite_{data_idx}_frames,\n");

		fd.write(f"\t.frame_count = {sprite['frames']},\n");
		fd.write(f"\t.width = {sprite['width']},\n");
		fd.write(f"\t.height = {sprite['height']},\n");
		fd.write(f"\t.loop = {str(sprite['loop']).lower()},\n");
		fd.write(f"\t.reverse = {str(sprite['reverse']).lower()},\n");
		fd.write("};\n");
		fd.write("\n");
	fd.write("\n");
	
	fd.write("const CAT_sprite* sprite_list[] =\n");
	fd.write("{\n");
	for (i, sprite) in enumerate(sprite_objects):
		fd.write(f"\t&{sprite['name']},\n");
	fd.write("};\n");
	fd.write("\n");

	OLV_write_epaper_sprites(fd);

	print(f"Mean compression ratio: {compression_ratio / len(path_map):.2f}");
