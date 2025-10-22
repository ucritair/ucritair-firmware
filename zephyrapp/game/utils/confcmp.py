#!/usr/bin/env python3

from pathlib import Path;
import configparser;
import sys;

if len(sys.argv) < 3:
	print("usage: confcmp.py <conf file A> <conf file B>");
paths = [Path(p) for p in sys.argv[1:]];
confs = [configparser.ConfigParser(allow_unnamed_section=True, strict=False) for p in paths];
for (i, c) in enumerate(confs):
	c.read(paths[i]);
confs = [c[configparser.UNNAMED_SECTION] for c in confs];

key_sets = [list(set(c.keys())) for c in confs];
key_superset = list(set(key_sets[0] + key_sets[1]));

for key in key_superset:
	if key in confs[0] and key in confs[1]:
		if confs[0][key] != confs[1][key]:
			print(f"{key}: {confs[0][key]} | {confs[1][key]}");
	elif key not in confs[0]:
		print(f"{key}: N/A | {confs[1][key]}");
	elif key not in confs[1]:
		print(f"{key}: {confs[0][key]} | N/A");