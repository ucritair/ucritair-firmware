#!/usr/bin/env python3

import os
import os.path
import subprocess
import sys
import pygame

pygame.init()

atlas = []

with open(os.path.dirname(__file__)+"/atlasdata.txt", 'r') as fd:
	for line in fd.readlines():
		line = line.strip()
		if not line: continue

		_, name, *p = line.split(',')
		atlas.append([name] + [int(x) for x in p])

image = os.path.dirname(__file__)+"/../game/sprites/atlas.png"
output = sys.argv[1]

def hex4(x):
	h = hex(x)[2:]
	h = (4-len(h))*'0' + h
	return '0x' + h

texture = pygame.image.load(image)
atlas_w, atlas_h = texture.get_size()
# atlas_w, atlas_h = [int(x) for x in subprocess.check_output(['identify', '-ping', '-format', "%w %h", f'{image}']).strip().split()]
# print(f"{image}: {atlas_w}x{atlas_h}")
# b = subprocess.check_output(['ffmpeg', '-vcodec', 'png', '-i', f'{image}', '-vcodec', 'rawvideo', '-f', 'rawvideo', '-pix_fmt', 'rgb565', '-'])
# data = b[:atlas_w*atlas_h*2]

def get_px(x, y):
	r, g, b, a = texture.get_at((x, y))
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

with open(f"{output}/images.c", 'w') as fd:
	fd.write('#include <stdint.h>\n')
	for name, x, y, w, h, idx in atlas:
		print(name)
		fd.write("const uint16_t image_data_"+str(idx)+"[] = { //"+name+"\n\t")

		written = 0
		for row in range(h):
			for col in range(w):
				fd.write(hex4(get_px(x + col, y + row)) + ", ")
				written += 1

				if (written%16) == 15:
					fd.write('\n\t')

		# for i in range(len(data)//2):
		# 	fd.write(hex4((data[(i*2)+0] << 8) | (data[(i*2)+1] << 0)) + ',')
		# 	if (i%16) == 15:
		# 		fd.write('\n\t')
		fd.write('\n};\n')

	fd.write('\n\n\nconst uint16_t* image_data_table[] = {\n')
	for name, x, y, w, h, idx in atlas:
		fd.write('\timage_data_'+str(idx)+',\n')
	fd.write('};\n')