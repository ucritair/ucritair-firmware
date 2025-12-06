#!/usr/bin/env python3

import assetgen;
from pathlib import Path;
from PIL import Image;

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

triptych = assetgen.Triptych(
	"sprites/sprites.json",
	"assets/sprite_assets.h",
	"assets/sprite_assets.c"
);
data = triptych.json_data;
hdr = triptych.header_writer;
src = triptych.source_writer;

for (i, sprite) in enumerate(data.instances):
	sprite['id'] = i;
	sprite['frames'] = max(sprite['frames'], 1);
	image = Image.open(Path("sprites")/sprite['path']);
	sprite['width'] = image.size[0];
	sprite['height'] = image.size[1] // sprite['frames'];
	image.close();

hdr.write_header(["<stdint.h>", "\"cat_render.h\"", "\"cat_core.h\""]);
hdr.write_list();
src.write_header(["\"sprite_assets.h\""]);

path_map = {};
compression_ratio = 0;

for (i, sprite) in enumerate(data.instances):
	path = sprite['path'];
	if path in path_map:
		continue;

	image = Image.open(Path("sprites")/path).convert("P");
	raw_size = image.size[0] * image.size[1] * 2;
	colour_table = TOS_tabulate_colour(image);

	src.write(f"const uint16_t sprite_{i}_colour_table[] =");
	src.start_body();
	for c in colour_table:
		if c != 0xdead:
			c = TOS_reverse_endianness(c);
		src.literal(c);
	src.end_body();

	frame_count = sprite['frames'];
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

		src.write(f"const uint8_t sprite_{i}_frame_{j}[] =");
		src.start_body();
		src.write(end="");
		for (k, token) in enumerate(tokens):
			if k > 0 and k % 32 == 0:
				src.write();
				src.write(end="");
			src.write(str(hex(token)), end=", ", ignore_indent=True);
			if k == len(tokens)-1:
				src.write();
		compressed_size += len(tokens);
		src.end_body();

	src.write(f"const uint8_t* sprite_{i}_frames[] =");
	src.start_body();
	for j in range(frame_count):
		src.literal(f"sprite_{i}_frame_{j}");
	src.end_body();

	path_map[path] = i;
	compression_ratio += raw_size / compressed_size;

for (i, sprite) in enumerate(data.instances):
	src.start_body(sprite["name"]);
	src.variable("id", sprite["id"]);

	data_idx = path_map[sprite['path']];
	src.variable("colour_table", f"sprite_{data_idx}_colour_table");
	src.variable("frames", f"sprite_{data_idx}_frames");

	src.variable("frame_count", sprite["frames"]);
	src.variable("width", sprite["width"]);
	src.variable("height", sprite["height"]);
	src.variable("loop", str(sprite["loop"]).lower());
	src.variable("reverse", str(sprite["reverse"]).lower());

	src.end_body();
src.write();

src.write_list();

print(f"Mean compression ratio: {compression_ratio / len(path_map):.2f}");
