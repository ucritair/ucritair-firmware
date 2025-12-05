#!/usr/bin/env python3

import assetgen;

item_type_enum_map = {
	"key" : "CAT_ITEM_TYPE_KEY",
	"tool" : "CAT_ITEM_TYPE_TOOL",
	"prop" : "CAT_ITEM_TYPE_PROP",
};

tool_type_enum_map = {
	"food" : "CAT_TOOL_TYPE_FOOD",
	"book" : "CAT_TOOL_TYPE_BOOK",
	"toy" : "CAT_TOOL_TYPE_TOY",
};

food_group_enum_map = {
	"veg" : "CAT_FOOD_GROUP_VEG",
	"starch" : "CAT_FOOD_GROUP_STARCH",
	"meat" : "CAT_FOOD_GROUP_MEAT",
	"dairy" : "CAT_FOOD_GROUP_DAIRY",
	"misc" : "CAT_FOOD_GROUP_MISC",
};

food_role_enum_map = {
	"staple" : "CAT_FOOD_ROLE_STAPLE",
	"main" : "CAT_FOOD_ROLE_MAIN",
	"side" : "CAT_FOOD_ROLE_SIDE",
	"soup" : "CAT_FOOD_ROLE_SOUP",
	"drink" : "CAT_FOOD_ROLE_DRINK",
	"treat" : "CAT_FOOD_ROLE_TREAT",
	"vice" : "CAT_FOOD_ROLE_VICE",
};

prop_type_enum_map = {
	"default" : "CAT_PROP_TYPE_DEFAULT",
	"bottom" : "CAT_PROP_TYPE_BOTTOM",
	"top" : "CAT_PROP_TYPE_TOP"
};


tool_types = ['food", "book", "toy'];
prop_types = ['prop", "bottom", "top'];

triptych = assetgen.Triptych(
	"data/items.json",
	"data/item_assets.h",
	"data/item_assets.c"
);
data = triptych.json_data;
hdr = triptych.header_writer;
src = triptych.source_writer;

item_ids = [x["id"] for x in data.instances];
max_id = max(item_ids);
if(max_id > 255):
	print("[ERROR] Max ID exceeds 255. Aborting");
	exit();

null_item = {
	"type" : "key",
	"display_name" : "Ketsuban",
	"sprite" : "null_sprite",
	"price" : 0,
	"text" : "",
	"can_buy" : False,
	"can_sell" : False
};
item_table = [null_item for i in range(max_id+1)];
for item in data.instances:
	item_table[item["id"]] = item;

missing_ids = [i for i in range(max_id+1) if i not in item_ids];
for i in missing_ids:
	print(f"[WARNING] Missing ID {i}");

hdr.write("#pragma once");
hdr.write();
for instance in item_table:
	hdr.write(f"#define {instance['name']}_item {instance["id"]}");

src.write_header(["\"cat_item.h\"", "\"item_assets.h\"", "\"sprite_assets.h\""]);
src.write("CAT_item_table item_table =");
src.write("{");

src.start_block("data");
for id in item_ids:
	item = item_table[id];

	src.write(f"[{id}] = (const CAT_item)");
	src.start_block();

	src.variable("type", item_type_enum_map[item['type']]);
	src.variable("name", f"\"{item['display_name']}\"");
	src.variable("sprite", f"&{item['sprite']}");
	src.variable("price", item['price']);
	src.variable("text", f"\"{item['text']}\"");
	src.variable("can_buy", str(item['can_buy']).lower());
	src.variable("can_sell", str(item['can_sell']).lower());

	if item['type'] == "tool":
		src.write();
		src.variable("tool_type", tool_type_enum_map[item['tool_data']['type']]);
		src.variable("tool_cursor", f"&{item['tool_data']['cursor'] if len(item['tool_data']['cursor']) else item['sprite']}");
		src.variable("tool_dv", item['tool_data']['dv']);
		src.variable("tool_df", item['tool_data']['df']);
		src.variable("tool_ds", item['tool_data']['ds']);
		if(item['tool_data']['type'] == "food"):
			src.write();
			src.variable("food_group", food_group_enum_map[item['tool_data']['food_data']['food_group']]);
			src.variable("food_role", food_role_enum_map[item['tool_data']['food_data']['food_role']]);

	elif item['type'] == "prop":
		src.write();
		src.variable("prop_type", prop_type_enum_map[item['prop_data']['type']]);
		src.variable("prop_shape", f"{{{item['prop_data']['shape'][0]}, {item['prop_data']['shape'][1]}}}");
		src.variable("prop_animated", str(item['prop_data']['animate']).lower());
		src.variable("prop_child_dy", item['prop_data']['child_dy']);

	src.end_block();
src.end_block();
src.variable("length", len(data.instances));

src.write("};");






