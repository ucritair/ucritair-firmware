#!/usr/bin/env python3

import os
import os.path
import subprocess
import sys
import pygame
from dataclasses import dataclass
import json
from PIL import Image;
import copy;

pygame.init()

@dataclass
class BakeData:
	idx: int
	name: str
	path: str
	frames: str
	width: str
	height: str
	rlesize: int = 0
	colors: set = None

	@property
	def size(self):
		return self.width * self.height * 2

	@property
	def do_compress(self):
		return True
		return not (self.name.startswith('base_wall') or self.name.startswith('base_floor'))

def RGBA88882RGB565(c):
	if len(c) == 4 and c[3] < 128:
		return 0xdead;
	r = int((c[0] / 255) * 31);
	g = int((c[1] / 255) * 63);
	b = int((c[2] / 255) * 31);
	return (r << 11) | (g << 5) | b;

json_file = open("sprites/sprites.json", "r+");
json_data = json.load(json_file);
json_entries = json_data["entries"];
atlas = [];

for (idx, sprite) in enumerate(json_entries):
	# Ensure correct JSON
	sprite["id"] = idx;
	image = Image.open(os.path.join("sprites", sprite["path"]));
	sprite["width"] = image.size[0];
	sprite["height"] = image.size[1] // sprite["frames"];
	image.close();

	# If so, atlasify
	atlas.append(BakeData(
		sprite["id"],
		sprite["name"],
		sprite["path"],
		sprite["frames"],
		sprite["width"],
		sprite["height"]
	));

json_file.seek(0);
json_file.truncate();
json_file.write(json.dumps(json_data, indent=4));
json_file.close();

with open("sprites/sprite_assets.h", "w") as fd:
	fd.write("#pragma once\n");
	fd.write("\n")
	fd.write("#include <stdint.h>\n");
	fd.write("\n");
	fd.write("#ifdef CAT_EMBEDDED\n");
	fd.write("typedef struct\n");
	fd.write("{\n");
	fd.write("\tint id;\n");
	fd.write("\tconst uint16_t* color_table;\n");
	fd.write("\tconst uint8_t** frames;\n");
	fd.write("\tint frame_count;\n");
	fd.write("\tint width;\n");
	fd.write("\tint height;\n");
	fd.write("} CAT_sprite;\n");
	fd.write("#else\n");
	fd.write("typedef struct\n");
	fd.write("{\n");
	fd.write("\tint id;\n");
	fd.write("\tconst uint16_t* colour_table;\n");
	fd.write("\tconst uint8_t** frames;\n");
	fd.write("\tint frame_count;\n");
	fd.write("\tint width;\n");
	fd.write("\tint height;\n");
	fd.write("} CAT_sprite;\n");
	fd.write("#endif\n");
	fd.write("\n");
	for (idx, sprite) in enumerate(atlas):
		fd.write(f"extern const CAT_sprite {sprite.name};\n");

# assert len(set(x.path for x in atlas)) == len([x.path for x in atlas]), "Duplicated path"
assert len(set(x.name for x in atlas)) == len([x.name for x in atlas]), "Duplicated name"

atlas.sort(key=lambda x: x.idx)

def hex4(x):
	h = hex(x)[2:]
	h = (4-len(h))*'0' + h
	return '0x' + h

def hex2(x):
	h = hex(x)[2:]
	h = (2-len(h))*'0' + h
	return '0x' + h

textures = {}
for x in atlas:
	try:
		textures[x.path] = pygame.image.load(os.path.join("sprites", x.path))
	except FileNotFoundError:
		print("Falling back for ", x.path)
		textures[x.path] = pygame.image.load(os.path.join("sprites", "none_24x24.png"))

def get_px(image, x, y):
	r, g, b, a = image.get_at((x, y))
	r = int(r * (1<<5) / (1<<8))
	g = int(g * (1<<6) / (1<<8))
	b = int(b * (1<<5) / (1<<8))

	assert r < (1<<5)
	assert g < (1<<6)
	assert b < (1<<5)

	word = r << (5+6) | g << 5 | b

	assert word < (1<<16)

	if "--noswap" not in sys.argv:
		hi = word >> 8
		lo = word & 0xff
		word = (lo << 8) | hi

	assert word != 0xdead

	if a != 255:
		word = 0xdead

	return word

rleword = 0xff

def rledecode(data, width):
	out = []

	while data:
		word = data.pop(0)
		# print(hex4(word))
		if word == rleword:
			repeated = data.pop(0)
			count = data.pop(0)
			while count:
				out.append(repeated)
				count -= 1
		else:
			out.append(word)

	return out

def rleencode(data, width):
	output = []
	rleword = 0xff

	last = None
	run = 0

	in_len = len(data)
	in_bak = data[:]

	data.append(None)

	while data:
		item = data.pop(0)

		# print(item, last, run)

		if item == rleword:
			item = 0x00
			print("!!")

		if item == last:
			run += 1

			if run == 255:
				output.extend([rleword, last, run])
				run = 0
		else:
			if run < 4:
				output.extend([last]*run)
			else:
				output.extend([rleword, last, run])

			last = item
			run = 1

	assert rledecode(output[:], width) == in_bak

	return output

texture_use_cache = {}

with open("sprites/sprite_assets.c", 'w') as fd:
	fd.write("#include \"sprite_assets.h\"\n")
	fd.write('\n')
	fd.write("#ifdef CAT_EMBEDDED\n");
	for sprite in atlas:
		if sprite.path not in texture_use_cache:
			texture_use_cache[sprite.path] = sprite.name
			image = textures[sprite.path]

			colors = {}
			# go through the whole 'image'
			for row in range(sprite.height * sprite.frames):
				for col in range(sprite.width):
					px = get_px(image, col, row)
					colors[px] = colors.get(px, 0) + 1

			color_freq = list(colors.items())
			color_freq.sort(key=lambda i: i[1], reverse=True)
			while len(color_freq) > 255:
				print("Dropped color")
				color = color_freq.pop(-1)

			key = list(x[0] for x in color_freq)

			if sprite.do_compress:
				fd.write("const uint16_t image_data_"+sprite.name+"_colorkey[] = {\n\t")
				for idx, data in enumerate(key):
					fd.write(hex4(data) + ", ")

					if (idx%16) == 0 and idx != 0:
						fd.write('\n\t')
				fd.write("\n};\n")

			for frame in range(sprite.frames):
				fd.write("const uint8_t image_data_"+sprite.name+"_frame"+str(frame)+"[] = {\n\t")

				lin_data = []
				for row in range(sprite.height):
					for col in range(sprite.width):
						px = get_px(image, col, (frame * sprite.height) + row)
						lin_data.append(px)

				if sprite.do_compress:
					sprite.colors = set(lin_data)
					keyed_data = []
					for word in lin_data:
						try:
							pos = key.index(word)
						except ValueError:
							pos = 0
						keyed_data.append(pos)

					data = rleencode(keyed_data, sprite.width)
					# sprite.rlesize = len(lin_data)

					for idx, data in enumerate(data):
						fd.write(hex2(data) + ", ")

						if (idx%16) == 0 and idx != 0:
							fd.write('\n\t')
				else:
					for idx, data in enumerate(lin_data):
						fd.write(hex2(data&0xff) + ", " + hex2(data>>8) + ", ")

						if (idx%16) == 0 and idx != 0:
							fd.write('\n\t')

				fd.write('\n};\n')

			fd.write("const uint8_t* image_data_"+sprite.name+"[] = {\n")
			for frame in range(sprite.frames):
				fd.write("\timage_data_"+sprite.name+"_frame"+str(frame)+",\n")
			fd.write("};\n")
	fd.write("\n")

	for sprite in atlas:
		fd.write(f"const CAT_sprite {sprite.name} =\n");
		fd.write("{\n");
		name = texture_use_cache[sprite.path];
		fd.write(f"\t.id = {sprite.idx},\n");
		if sprite.do_compress:
			fd.write(f"\t.color_table = image_data_{name}_colorkey,\n");
		fd.write(f"\t.frames = image_data_{name},\n");
		fd.write(f"\t.frame_count = {sprite.frames},\n");
		fd.write(f"\t.width = {sprite.width},\n");
		fd.write(f"\t.height = {sprite.height},\n");
		fd.write("};\n");
		fd.write("\n");

	einksize = 0
	if '--noswap' not in sys.argv:
		fd.write('#include "../../src/epaper_rendering.h"\n')
		fd.write("\n")
		eink_folder = "../assets/"
		for path in os.listdir(eink_folder):
			texture = pygame.image.load(eink_folder+path)
			width, height = texture.get_size()
			e_width = width + (width % 8)
			fd.write('struct epaper_image_asset epaper_image_'+path.split('.')[0]+" = {\n");
			fd.write(f'\t.w = {width}, .h = {height}, .stride = {e_width},\n')
			fd.write(f'\t.bytes = {{\n\t\t')

			pixels = []
			for y in range(height):
				for x in range(e_width):
					try:
						px = texture.get_at((x, y))
					except IndexError:
						px = (255, 255, 255, 0)
					# assert px[:3] in ((0, 0, 0), (255, 255, 255)), str(px)
					px = int(px[:3] == (0, 0, 0) and px[3] != 0)
					pixels.append(px)

			words = []
			while pixels:
				w = 0
				for i in range(8):
					w |= pixels.pop(0) << (7-i)
				words.append(w)

			assert len(words) == (e_width*height)//8

			einksize += len(words)+2

			for i, w in enumerate(words):
				fd.write(f'{hex2(w)}, ')
				if (i%16 == 0) and (i != 0):
					fd.write('\n\t\t')

			fd.write('\n\t}\n};\n');


def RGBA88882RGB565(c):
	if len(c) == 4 and c[3] < 128:
		return 0xdead;
	r = int((c[0] / 255) * 31);
	g = int((c[1] / 255) * 63);
	b = int((c[2] / 255) * 31);
	return (r << 11) | (g << 5) | b;

def TOS_tabulate_colour(image):
	pal = image.palette.colors;
	rgb888s = pal.keys();
	rgb565s = [RGBA88882RGB565(c) for c in rgb888s];
	return rgb565s;

def TOS_rl_encode(image):
	pixels = image.tobytes();
	runs = [[pixels[0], 1]];
	for i in range(1, len(pixels)):
		if(pixels[i] != runs[-1][0] or runs[-1][1] >= 255):
			runs.append([pixels[i], 1]);
		else:
			runs[-1][1] += 1;
	return runs;

def TOS_rl_decode(runs):
	pixels = [];
	for r in runs:
		for i in range(r[1]):
			pixels.append(r[0]);
	return pixels;

with open("sprites/sprite_assets.c", 'a') as fd:
	fd.write("#else\n");

	path_map = {};
	compression_ratio = 0;
	for (idx, sprite) in enumerate(json_entries):
		path = sprite["path"];
		if path in path_map:
			continue;
		frame_count = sprite["frames"];

		image = Image.open(os.path.join("sprites", path)).convert("P");
		raw_size = image.size[0] * image.size[1] * 2;

		colour_table = TOS_tabulate_colour(image);
		fd.write(f"const uint16_t sprite_{idx}_colour_table[] = \n");
		fd.write("{\n");
		for c in colour_table:
			fd.write(f"\t{hex(c)},\n");
		fd.write("};\n");
		fd.write("\n");

		frame_width = image.size[0];
		frame_height = image.size[1] // frame_count;
		compressed_size = 0;
		fd.write(f"const uint8_t* sprite_{idx}_frames[] = \n");
		fd.write("{\n");
		for i in range(frame_count):
			l = 0;
			r = frame_width;
			t = frame_height * i;
			b = t + frame_height;
			frame = image.crop((l, t, r, b));
			runs = TOS_rl_encode(frame);
			compressed_size += len(runs) * 2;

			fd.write(f"\t[{i}] = (uint8_t[])\n");
			fd.write("\t{\n\t\t");
			for (j, run) in enumerate(runs):
				if j > 0 and j % 16 == 0:
					fd.write("\n\t\t");
				fd.write(f"{hex(run[0])},{hex(run[1])},");
				if j == len(runs)-1:
					fd.write("\n");
			fd.write("\t},\n");
		fd.write("};\n");
		fd.write("\n");

		path_map[path] = idx;
		compression_ratio += raw_size / compressed_size;

	print(f"Mean compression ratio: {compression_ratio / len(path_map):.2f}");
		
	for (idx, sprite) in enumerate(json_entries):
		fd.write(f"const CAT_sprite {sprite["name"]} =\n");
		fd.write("{\n");
		fd.write(f"\t.id = {sprite["id"]},\n");

		data_idx = path_map[sprite["path"]];
		fd.write(f"\t.colour_table = sprite_{data_idx}_colour_table,\n");
		fd.write(f"\t.frames = sprite_{data_idx}_frames,\n");

		fd.write(f"\t.frame_count = {sprite["frames"]},\n");
		fd.write(f"\t.width = {sprite["width"]},\n");
		fd.write(f"\t.height = {sprite["height"]},\n");
		fd.write("};\n");
		fd.write("\n");

	fd.write("#endif\n");
	fd.write("\n");
