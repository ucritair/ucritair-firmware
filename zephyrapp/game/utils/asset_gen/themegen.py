#!/usr/bin/env python3

import assetgen;

triptych = assetgen.Triptych(
	"data/themes.json",
	"data/theme_assets.h",
	"data/theme_assets.c"
);
data = triptych.json_data;
data.ctype_name = "CAT_room_theme";
hdr = triptych.header_writer;
src = triptych.source_writer;

hdr.write_header(["\"cat_room.h\""]);
hdr.write_list();
src.write_header(["\"theme_assets.h\"", "\"sprite_assets.h\""]);

for instance in data.instances:
	src.start_body(instance["name"]);
	src.variable("name", f"\"{instance['display_name']}\"");

	src.variable("wall_tiles", f"&{instance['wall_tiles']}");
	src.variable("tile_wall", str(instance['tile_wall']).lower());
	src.start_block("wall_map", "uint8_t[]");
	if len(instance["wall_map"]) == 6*15:
		for y in range(6):
			for x in range(15):
				idx = y * 15 + x;
				src.literal(int(instance["wall_map"][idx]));
	else:
		src.literal(0);
	src.end_block();
	src.write();

	src.start_block("window_rect");
	src.variable("min", f"{{{instance["window_rect"][0]}, {instance["window_rect"][1]}}}");
	src.variable("max", f"{{{instance["window_rect"][0]+instance["window_rect"][2]}, {instance["window_rect"][1]+instance["window_rect"][3]}}}");
	src.end_block();
	src.write();

	src.variable("floor_tiles", f"&{instance['floor_tiles']}");
	src.variable("tile_floor", str(instance['tile_floor']).lower());
	src.start_block("floor_map", "uint8_t[]");
	if len(instance["floor_map"]) == 14*15:
		for y in range(14):
			for x in range(15):
				idx = y * 15 + x;
				src.literal(int(instance["floor_map"][idx]));
	else:
		src.literal(0);
	src.end_block();
	src.write();

	src.end_body();
src.write();

src.write_list();






