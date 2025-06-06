#!/usr/bin/env python3

import json;

class MD_writer:
	def __init__(self, path):
		self.md = open(path, "w");

	def text(self, s):
		self.md.write(f"{s}\n");

	def header(self, tier, s):
		if tier <= 0:
			self.text(s);
		else:
			tier = min(max(tier, 1), 3);
			header = "#" * (4-tier); 
			self.md.write(f"{header} {s}\n");

	def list(self, title, l):
		self.header(1, title);
		for item in list:
			self.md.write(f"- {item}\n");

	def newline(self):
		self.md.write("\n");

	def repr_item(self, item):
		self.header(1, item['display_name']);
		self.text(f"Asset Name: {item['name']}");
		self.text(f"Item ID: {item['id']}");
		self.text(f"Type: {item['type'].title()}");
		if item['price'] > 0:
			self.text(f"Price: {item['price']}");
		else:
			self.text("_This item cannot be purchased._");

	def close(self):
		self.md.close();

json_file = open("data/items.json", "r");
json_data = json.load(json_file);
json_entries = json_data['entries'];
json_file.close();

md = MD_writer("docs/items.md");

md.header(3, "Items");
md.text(f"There are {len(json_entries)} items in-game.");
md.text("All items belong to one of 3 overarching types: Key, Tool, or Prop.");
md.text("Many items are available for purchase through the vending machine, but some can only be obtained through other gameplay avenues.");
md.newline();
md.header(2, "Item List");


for item in json_entries:
	md.repr_item(item);
	md.newline();

md.close();








