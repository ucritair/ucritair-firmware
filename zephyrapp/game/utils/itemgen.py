#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;

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
	item['id'] = idx;
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
	if item['type'] == "tool":
		source.write("\t\t\t.type = CAT_ITEM_TYPE_TOOL,\n");
	elif item['type'] == "prop":
		source.write("\t\t\t.type = CAT_ITEM_TYPE_PROP,\n");
	else:
		source.write("\t\t\t.type = CAT_ITEM_TYPE_KEY,\n");
	source.write(f"\t\t\t.name = \"{item['display_name']}\",\n");
	source.write(f"\t\t\t.sprite = &{item['sprite']},\n");
	source.write(f"\t\t\t.price = {item['price']},\n");
	source.write(f"\t\t\t.text = \"{item['text']}\",\n");
	source.write(f"\t\t\t.icon = &{item['icon']},\n");
	if item['type'] == "tool":
		source.write("\t\t\t.data.tool_data =\n");
		source.write("\t\t\t{\n");
		if item['tool_data']['type'] == "food":
			source.write("\t\t\t\t.type = CAT_TOOL_TYPE_FOOD,\n");
		elif item['tool_data']['type'] == "book":
			source.write("\t\t\t\t.type = CAT_TOOL_TYPE_BOOK,\n");
		else:
			source.write("\t\t\t\t.type = CAT_TOOL_TYPE_TOY,\n");
		source.write(f"\t\t\t\t.cursor = &{item['tool_data']['cursor']},\n");
		source.write(f"\t\t\t\t.dv = {item['tool_data']['dv']},\n");
		source.write(f"\t\t\t\t.df = {item['tool_data']['df']},\n");
		source.write(f"\t\t\t\t.ds = {item['tool_data']['ds']},\n");
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






