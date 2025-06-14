#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;

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

json_file = open("data/items.json", "r+");
json_data = json.load(json_file);
json_entries = json_data['entries'];

tool_types = ['food", "book", "toy'];
prop_types = ['prop", "bottom", "top'];

def ensure_key(obj, key, default):
	if not key in obj.keys():
		obj[key] = default;
	return obj[key];

for (idx, item) in enumerate(json_entries):
	ensure_key(item, "type", "key");
	ensure_key(item, "name", "item");
	ensure_key(item, "display_name", "Item");
	ensure_key(item, "sprite", "none_24x24.png");
	ensure_key(item, "icon", "item_icon_key_sprite");
	ensure_key(item, "text", "");
	ensure_key(item, "price", 0);
	if item['type'] == "tool":
		tool_data = ensure_key(item, "tool_data", {});
		ensure_key(tool_data, "type", "food");
		ensure_key(tool_data, "cursor", item['sprite']);
		ensure_key(tool_data, "dv", 0);
		ensure_key(tool_data, "df", 0);
		ensure_key(tool_data, "ds", 0);
		if tool_data["type"] == "food":
			food_data = ensure_key(tool_data, "food_data", {});
			ensure_key(food_data, "food_group", "veg");
			ensure_key(food_data, "food_role", "staple");
	elif item['type'] == "prop":
		prop_data = ensure_key(item, "prop_data", {});
		ensure_key(prop_data, "type", "default");
		ensure_key(prop_data, "shape", [1, 1]);
		ensure_key(prop_data, "animate", True);
		ensure_key(prop_data, "child_dy", 0);
	item['text'] = "";

json_file.seek(0);
json_file.truncate();
json_file.write(json.dumps(json_data, indent=4));
json_file.close();

json_entries.sort(key = lambda i: i["id"]);
for i in range(len(json_entries)-1):
	a = json_entries[i];
	b = json_entries[i+1];
	if b["id"]-a["id"] != 1:
		print(f"[WARNING] Gap between item IDs {a["id"]} and {b["id"]}!");
	
header = open("data/item_assets.h", "w");
header.write("#pragma once\n");
header.write("\n");
for (idx, item) in enumerate(json_entries):
	header.write(f"#define {item['name']}_item {idx}\n");
header.close();

source = open("data/item_assets.c", "w");
source.write("#include \"item_assets.h\"\n");
source.write("\n");
source.write("#include \"cat_item.h\"\n");
source.write("#include \"sprite_assets.h\"\n");
source.write("\n");
source.write("CAT_item_table item_table =\n");
source.write("{\n");
source.write("\t.data =\n");
source.write("\t{\n");
for (idx, item) in enumerate(json_entries):
	source.write("\t\t{\n");
	source.write(f"\t\t\t.type = {item_type_enum_map[item['type']]},\n");
	source.write(f"\t\t\t.name = \"{item['display_name']}\",\n");
	source.write(f"\t\t\t.sprite = &{item['sprite']},\n");
	source.write(f"\t\t\t.price = {item['price']},\n");
	source.write(f"\t\t\t.text = \"{item['text']}\",\n");
	source.write(f"\t\t\t.icon = &{item['icon']},\n");
	if item['type'] == "tool":
		source.write("\t\t\t.data.tool_data =\n");
		source.write("\t\t\t{\n");
		source.write(f"\t\t\t\t.type = {tool_type_enum_map[item['tool_data']['type']]},\n");
		source.write(f"\t\t\t\t.cursor = &{item['tool_data']['cursor'] if len(item['tool_data']['cursor']) else item['sprite']},\n");
		source.write(f"\t\t\t\t.dv = {item['tool_data']['dv']},\n");
		source.write(f"\t\t\t\t.df = {item['tool_data']['df']},\n");
		source.write(f"\t\t\t\t.ds = {item['tool_data']['ds']},\n");
		if(item['tool_data']['type'] == "food"):
			source.write("\n");
			source.write(f"\t\t\t\t.food_group = {food_group_enum_map[item['tool_data']['food_data']['food_group']]},\n");
			source.write(f"\t\t\t\t.food_role = {food_role_enum_map[item['tool_data']['food_data']['food_role']]},\n");
		source.write("\t\t\t}\n");
	elif item['type'] == "prop":
		source.write("\t\t\t.data.prop_data =\n");
		source.write("\t\t\t{\n");
		if item['prop_data']['type'] == "bottom":
			source.write("\t\t\t\t.type = CAT_PROP_TYPE_BOTTOM,\n");
		elif item['prop_data']['type'] == "top":
			source.write("\t\t\t\t.type = CAT_PROP_TYPE_TOP,\n");
		else:
			source.write("\t\t\t\t.type = CAT_PROP_TYPE_DEFAULT,\n");
		source.write(f"\t\t\t\t.shape = {{{item['prop_data']['shape'][0]}, {item['prop_data']['shape'][1]}}},\n");
		if item['prop_data']['animate']:
			source.write(f"\t\t\t\t.animate = true,\n");
		else:
			source.write(f"\t\t\t\t.animate = false,\n");
		source.write(f"\t\t\t\t.child_dy = {item['prop_data']['child_dy']},\n");
		source.write("\t\t\t}\n");
	source.write("\t\t},\n");
source.write("\t},\n");
source.write(f"\t.length = {len(json_entries)}\n");
source.write("};\n");
source.close();






