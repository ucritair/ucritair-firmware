#!/usr/bin/env python3

# PAGE(Items)

import json;
from html_writer import HTMLWriter, HTMLMode;
import common;

def build(html: HTMLWriter):
	json_file = open("data/items.json", "r");
	json_data = json.load(json_file);
	json_file.close();
	json_entries = json_data['instances'];
	json_entries.sort(key=lambda j: j['display_name']);

	common.title(html, "Items");
	html.start_text_block();
	html.text(f"There are {len(json_entries)} items in-game.");
	html.text("All items belong to one of 3 overarching types: Key, Tool, or Prop.");
	html.text("Many items are available for purchase through the vending machine, but some can only be obtained through other gameplay avenues.");
	html.text("A small subset of items can be sold.");
	html.end_text_block();
	html.newline();

	html.start_table(["Sprite", "Display Name", "Asset Name", "Item ID", "Type", "Purchase Price", "Sale Price", "Description"]);
	for item in json_entries:
		html.table_row([
			html.image(f"/images/thumbnails/{item["sprite"]}.png", mode=HTMLMode.INLINE),
			item["display_name"],
			item["name"],
			item["id"],
			item["type"].title(),
			item["price"] if item["can_buy"] else "N/A",
			item["price"] if item["can_sell"] else "N/A",
			item["text"] if len(item["text"]) > 0 else "N/A"
		]);
	html.end_table();
