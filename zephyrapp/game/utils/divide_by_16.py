#!/usr/bin/env python3

import json;

file = open("data/props.json", "r+");
data = json.load(file);
instances = data["instances"];

for prop in instances:
	for blocker in prop["blockers"]:
		for i in range(4):
			blocker[i] = int(blocker[i]);
	for trigger in prop["triggers"]:
		for i in range(4):
			trigger["aabb"][i] = int(trigger["aabb"][i]);

file.seek(0);
file.truncate();
file.write(json.dumps(data, indent=4));
file.close();

file = open("data/scenes.json", "r+");
data = json.load(file);
instances = data["instances"];

for scene in instances:
	for layer in scene["layers"]:
		for prop in layer:
			for i in range(2):
				prop["position"][i] = int(prop["position"][i]//16);

file.seek(0);
file.truncate();
file.write(json.dumps(data, indent=4));
file.close();