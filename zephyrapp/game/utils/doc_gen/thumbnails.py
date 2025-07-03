#!/usr/bin/env python3

import json;
from PIL import Image;

json_file = open("sprites/sprites.json", "r");
json_data = json.load(json_file);
json_file.close();
json_entries = json_data["entries"];

for sprite in json_entries:
	in_path = f"sprites/{sprite["path"]}";
	out_path = f"docs/images/thumbnails/{sprite["name"]}.png";

	img = Image.open(in_path);
	img = img.crop((0, 0, sprite["width"], sprite["height"]));
	img.thumbnail((128, 128));
	img.save(out_path, "PNG");
