#!/usr/bin/env python3

import os
import os.path
import subprocess
import sys
import pygame
from dataclasses import dataclass
import json

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

atlas = []

with open("sprites/sprites.json", "r") as fd:
	sprites = json.load(fd);
	for sprite in sprites:
		if sprite["mode"] == "init":
			atlas.append(BakeData(
				sprite["id"],
				sprite["name"],
				sprite["path"],
				sprite["frames"],
				sprite["width"],
				sprite["height"]
			));
		else:
			source = atlas[sprite["source"]];
			atlas.append(BakeData(
				sprite["id"],
				sprite["name"],
				source.path,
				source.frames,
				source.width,
				source.height
			));

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
		textures[x.path] = pygame.image.load(x.path)
	except FileNotFoundError:
		print("Falling back for ", x.path)
		textures[x.path] = pygame.image.load("sprites/none_24x24.png")

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
	fd.write('#include <stdint.h>\n')
	fd.write('#include "cat_sprite.h"\n')
	fd.write('\n\n')
	for sprite in atlas:
		print(sprite.name)

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
			fd.write("};\n\n");

	fd.write('\n\n\nconst CAT_baked_sprite image_data_table[] = {\n')
	for sprite in atlas:
		name = texture_use_cache[sprite.path]
		fd.write('\t['+str(sprite.idx)+'] = {\n')
		if sprite.do_compress:
			fd.write('\t\t.color_table=image_data_'+name+'_colorkey,\n')
		fd.write('\t\t.frames=image_data_'+name+',\n')
		fd.write('\t},\n')
	fd.write('};\n')

	fd.write('\n\n\nCAT_atlas atlas = { .table = {\n')
	for sprite in atlas:
		fd.write('\t['+str(sprite.idx)+']={\n');
		fd.write('\t\t.width = '+str(sprite.width)+",\n")
		fd.write('\t\t.height = '+str(sprite.height)+",\n")
		fd.write('\t\t.frame_count = '+str(sprite.frames)+",\n")
		fd.write('\t},\n')
		
	fd.write('}, .length = '+str(len(atlas))+'};\n\n')
    
	fd.write('\n\nuint16_t rle_work_region['+str(max(x.width for x in atlas))+'];\n\n\n\n')

	einksize = 0

	if '--noswap' not in sys.argv:
		fd.write('#include "../../src/epaper_rendering.h"\n')
		eink_folder = "../assets/"
		for path in os.listdir(eink_folder):
			print('eink', path)
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

			fd.write('\n\t}\n};\n\n');

print('eink size', einksize)


# print(len(colors), "colors")

# print(max(len(x.colors) for x in atlas), "max colors per sprite")
# print(sum(x.size for x in atlas), "bytes pre-RLE")
# print(sum(x.rlesize for x in atlas), "bytes post-RLE")

# colorsizes = {}
# for sprite in atlas[:-5]:
# 	colorsizes.setdefault(len(sprite.colors), 0)
# 	colorsizes[len(sprite.colors)] += 1

# print(colorsizes)

# keying_savings = 0
# for sprite in atlas:
# 	tabsize = len(sprite.colors) + 1
# 	dsize = sprite.rlesize / 2
# 	keyedsize = int(tabsize + dsize)
# 	if keyedsize < sprite.size:
# 		keying_savings += sprite.size - keyedsize

# print(keying_savings, "keying savings")
