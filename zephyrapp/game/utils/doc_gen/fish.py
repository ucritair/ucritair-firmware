#!/usr/bin/env python3

import json;
from md_writer import MD_writer;

json_file = open("data/fish.json", "r");
json_data = json.load(json_file);
json_file.close();
json_entries = json_data['entries'];
json_entries.sort(key=lambda j: j['display_name']);

md = MD_writer("docs/fish.md");

md.header(3, "Fish");
md.text(f"There are {len(json_entries)} fish in-game.");
md.text("Fish belong to tiers. Higher tier fish are more rare and harder to catch.");
md.text("Once caught, fish can be sold.");
md.newline();

md.table_header(["Display Name", "Asset Name", "Tier", "Proverb"]);
for item in json_entries:
	md.table_row([
		item["display_name"],
		item["name"],
		item["grade_constraint"]+1,
		item["proverb"]
	]);

md.close();








