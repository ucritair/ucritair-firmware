#!/usr/bin/env python3

# PAGE(Fish)

import json;
from html_writer import HTMLWriter;
import common;

def build(html: HTMLWriter):
	json_file = open("data/fish.json", "r");
	json_data = json.load(json_file);
	json_file.close();
	json_entries = json_data['entries'];
	json_entries.sort(key=lambda j: j['display_name']);

	html.heading(1, "Fish");
	html.start_text_block();
	html.text(f"There are {len(json_entries)} fish in-game.");
	html.text("Fish belong to tiers. Higher tier fish are more rare and harder to catch.");
	html.text("Once caught, fish can be sold.");
	html.end_text_block();
	html.newline();

	html.start_table(["Display Name", "Asset Name", "Tier", "Proverb"]);
	for item in json_entries:
		html.table_row([
			item["display_name"],
			item["name"],
			item["grade_constraint"]+1,
			item["proverb"]
		]);
	html.end_table();








