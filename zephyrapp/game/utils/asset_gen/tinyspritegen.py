#!/usr/bin/env python3

import assetgen;
from pathlib import Path;
from PIL import Image;
import math;

triptych = assetgen.Triptych(
	"sprites/tinysprites.json",
	"sprites/tinysprite_assets.h",
	"sprites/tinysprite_assets.c"
);
data = triptych.json_data;
hdr = triptych.header_writer;
src = triptych.source_writer;

for (i, sprite) in enumerate(data.instances):
	image = Image.open(Path("sprites")/sprite['path']);
	sprite['width'] = image.size[0];
	sprite['height'] = image.size[1];
	sprite['stride'] = image.size[0] + (image.size[0] % 8);
	image.close();

hdr.write_header(["<stdint.h>", "\"cat_render.h\"", "\"cat_core.h\""]);
src.write_header(["\"tinysprite_assets.h\""]);

for sprite in data.instances:
	texture = Image.open(Path("sprites")/sprite["path"]).convert("RGBA");
	width, height = texture.width, texture.height;
	padded_width = width + (width % 8);
	compatible = (padded_width*height)%8 == 0;
	if not compatible:
		print(f"[ERROR] {sprite["path"]} not compatible!");
		continue;
	
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

	src.write(f"const CAT_tinysprite tnyspr_{sprite['name']} =");
	src.start_body();

	src.variable("width", width);
	src.variable("height", height);
	src.variable("stride", padded_width);

	src.start_block("bytes");
	src.write(end="");
	for i, w in enumerate(words):
		src.write(str(hex(w)), end=", ", ignore_indent=True);
		if i % 16 == 0 and i > 0:
			src.write();
			src.write(end="");
	src.end_block();

	src.end_body();