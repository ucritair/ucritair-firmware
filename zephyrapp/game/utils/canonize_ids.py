#!/usr/bin/env python3

import sys;
import json;
import os;
import pathlib as pl;

if(len(sys.argv) != 3):
	print("usage: canonize_ids.py {OPERAND} {CANON}");
	exit();
operand_path = sys.argv[1];
canon_path = sys.argv[2];

canon_file = open(canon_path, "r");
canon_data = json.load(canon_file);
canon = canon_data['entries'];
canon_file.close();

operand_file = open(operand_path, "r+");
operand_data = json.load(operand_file);
operands = operand_data['entries'];

canon_id_map = {};
for entry in canon:
	canon_id_map[entry["name"]] = entry["id"];

canon_id_set = set(canon_id_map.values());
def take_free_id():
	M = max(canon_id_set);
	i = 0;
	while i <= M:
		if not i in canon_id_set:
			break;
		i += 1;
	canon_id_set.add(i);
	return i;

for entry in operands:
	if entry["name"] in canon_id_map:
		entry["id"] = canon_id_map[entry["name"]];
		print(f"[CANON] {entry["name"]} : {entry["id"]}");
	else:
		old_id = entry["id"];
		print(f"[NEW] {entry["name"]} : {entry["id"]}");

operand_file.seek(0);
operand_file.truncate();
operand_file.write(json.dumps(operand_data, indent=4));
