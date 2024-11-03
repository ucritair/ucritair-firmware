#!/usr/bin/env python3

import os
import os.path
import subprocess
import sys
import pygame
from collections import namedtuple

pygame.init()

BakeData = namedtuple('BakeData', ('idx', 'name', 'path', 'frames', 'width', 'height'))

atlas = []

with open(os.path.dirname(__file__)+"/atlasdata.txt", 'r') as fd:
	for line in fd.readlines():
		line = line.strip()
		if not line.startswith("BAKE:"): continue

		tup = eval(line.replace('BAKE: ', '', 1))

		idx, name, path, frames, width, height = tup
		atlas.append(BakeData(int(idx), name, path, int(frames), int(width), int(height)))

assert len(set(x.path for x in atlas)) == len([x.path for x in atlas]), "Duplicated path"
assert len(set(x.name for x in atlas)) == len([x.name for x in atlas]), "Duplicated name"

atlas.sort(key=lambda x: x.idx)

output = sys.argv[1]

def hex4(x):
	h = hex(x)[2:]
	h = (4-len(h))*'0' + h
	return '0x' + h

textures = {}
for x in atlas:
	try:
		textures[x.path] = pygame.image.load(os.path.dirname(__file__)+"/../game/"+x.path)
	except FileNotFoundError:
		print("Falling back for ", x.path)
		textures[x.path] = pygame.image.load(os.path.dirname(__file__)+"/../game/sprites/none_32x32.png")

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

def rledecode(data, width):
	out = []

	while data:
		word = data.pop(0)
		# print(hex4(word))
		if word == 0xbeef:
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
	rleword = 0xbeef

	last = None
	run = 0

	in_len = len(data)
	in_bak = data[:]

	data.append(None)

	while data:
		item = data.pop(0)

		# print(item, last, run)

		if item == rleword:
			item = 0xbeee
			print("!!")

		if item == last:
			run += 1
		else:
			if run < 4:
				output.extend([last]*run)
			else:
				output.extend([rleword, last, run])

			last = item
			run = 1

	assert rledecode(output[:], width) == [x if x!=0xbeef else 0xbeee for x in in_bak]

	return output

with open(f"{output}/images.c", 'w') as fd:
	fd.write('#include <stdint.h>\n')
	fd.write('#include "cat_sprite.h"\n')
	fd.write('\n\n')
	for sprite in atlas:
		print(sprite.name)

		image = textures[sprite.path]
		for frame in range(sprite.frames):
			fd.write("const uint16_t image_data_"+sprite.name+"_frame"+str(frame)+"[] = {\n\t")

			lin_data = []
			for row in range(sprite.height):
				for col in range(sprite.width):
					lin_data.append(get_px(image, col, (frame * sprite.height) + row))

			lin_data = rleencode(lin_data, sprite.width)

			for idx, data in enumerate(lin_data):
				fd.write(hex4(data) + ", ")

				if (idx%16) == 0 and idx != 0:
					fd.write('\n\t')

			fd.write('\n};\n')

		fd.write("const uint16_t* image_data_"+sprite.name+"[] = {\n")
		for frame in range(sprite.frames):
			fd.write("\timage_data_"+sprite.name+"_frame"+str(frame)+",\n")
		fd.write("};\n\n");

	fd.write('\n\n\nconst uint16_t** image_data_table[] = {\n')
	for sprite in atlas:
		fd.write('\timage_data_'+sprite.name+',\n')
	fd.write('};\n')

	fd.write('\n\n\nCAT_atlas atlas = { .table = {\n')
	for sprite in atlas:
		fd.write('\t['+str(sprite.idx)+']={\n');
		fd.write('\t\t.width = '+str(sprite.width)+",\n")
		fd.write('\t\t.height = '+str(sprite.height)+",\n")
		fd.write('\t\t.frame_count = '+str(sprite.frames)+",\n")
		fd.write('\t\t.frame_idx = 0,\n')
		fd.write('\t\t.loop = true,\n');
		fd.write('\t\t.needs_update = false,\n')
		fd.write('\t},\n')
	fd.write('}, .length = '+str(len(atlas))+'};\n\n')

	fd.write('\n\nuint16_t rle_work_region['+str(max(x.width for x in atlas))+'];')
