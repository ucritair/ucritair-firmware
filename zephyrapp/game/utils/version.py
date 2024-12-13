#!/usr/bin/env python3

import sys;

if(len(sys.argv) != 2):
	print("usage: version.py {tell|major|minor|patch|push}");
	exit();
mode = sys.argv[1];

def is_version_def(tokens):
	if len(tokens) != 3:
		return False;
	if tokens[0] != "#define":
		return False;
	if not "CAT_VERSION" in tokens[1]:
		return False;
	return True;

file = open("src/cat_version.h", "r");
lines = file.read().split("\n");
file.close();
lines = [line.strip() for line in lines];
lines = [line.split(" ") for line in lines];
defines = filter(is_version_def, lines);
fields = {};
for tokens in defines:
	fields[tokens[1]] = int(tokens[2]);
major = fields["CAT_VERSION_MAJOR"];
minor = fields["CAT_VERSION_MINOR"];
patch = fields["CAT_VERSION_PATCH"];
push = fields["CAT_VERSION_PUSH"];

if mode == "tell":
	print(f"Version {major}.{minor}.{patch}.{push}");
	exit();
elif mode == "major":
	major += 1;
elif mode == "minor":
	minor += 1;
elif mode == "patch":
	patch += 1;
elif mode == "push":
	push += 1;
else:
	print(f"Argument {mode} not recognized.")
	print("usage: version.py {tell|major|minor|patch|push}");
	exit();

text = ("#pragma once\n"
		"\n"
		f"#define CAT_VERSION_MAJOR {major}\n"
		f"#define CAT_VERSION_MINOR {minor}\n"
		f"#define CAT_VERSION_PATCH {patch}\n"
		f"#define CAT_VERSION_PUSH {push}\n");

file = open("src/cat_version.h", "w");
file.write(text);
file.close();
	
		