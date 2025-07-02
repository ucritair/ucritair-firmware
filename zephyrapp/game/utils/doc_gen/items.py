#!/usr/bin/env python3

import json;
from md_writer import MD_writer;

json_file = open("data/items.json", "r");
json_data = json.load(json_file);
json_file.close();
json_entries = json_data['entries'];
json_entries.sort(key=lambda j: j['display_name']);

md = MD_writer("docs/items.md");

md.header(3, "Items");
md.text(f"There are {len(json_entries)} items in-game.");
md.text("All items belong to one of 3 overarching types: Key, Tool, or Prop.");
md.text("Many items are available for purchase through the vending machine, but some can only be obtained through other gameplay avenues.");
md.text("A small subset of items can be sold.");
md.newline();

md.table_header(["Sprite", "Display Name", "Asset Name", "Item ID", "Type", "Purchase Price", "Sale Price"]);
for item in json_entries:
	md.table_row([
		f"![](thumbnails/{item["sprite"]}.png)",
		item["display_name"],
		item["name"],
		item["id"],
		item["type"].title(),
		item["price"] if item["can_buy"] else "N/A",
		item["price"] if item["can_sell"] else "N/A"
	]);

md.close();
