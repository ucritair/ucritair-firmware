#!/usr/bin/env python3

import json;

def write_header(strings, tags):
	file = open("assets/notice_assets.h", "w+");
	def _print(str=""):
		print(str, file=file);
	
	_print("#pragma once");
	_print();
	_print("#include \"cat_notices.h\"");
	_print();
	_print("typedef enum\n{");
	_print(f"\tCAT_CONTENT_TAG_NONE = 0,");
	for idx, tag in enumerate(tags):
		_print(f"\tCAT_CONTENT_TAG_{tag} = (1 << {idx}),");
	_print("} CAT_content_tag;");
	_print();
	_print("extern const CAT_notice CAT_notice_list[];");
	_print(f"#define CAT_NOTICE_COUNT {len(strings)}");

	file.close();

def write_source(strings, tags):
	file = open("assets/notice_assets.c", "w+");
	def _print(str=""):
		print(str, file=file);
	
	_print("#include \"notice_assets.h\"");
	_print();
	#_print("static const char* const string_bank[] =");
	#_print("{");
	#for string in strings:
	#	_print(f"\t\"{string["string"]}\",");
	#_print("};");
	#_print();
	_print("const CAT_notice CAT_notice_list[] =");
	_print("{");
	for idx, string in enumerate(strings):
		_print("\t(const CAT_notice) {");
		#_print(f"\t\t.string = string_bank[{idx}],");
		_print(f"\t\t.string = \"{string["string"]}\",");
		if len(string["tags"]) > 0:
			tag_str = "";
			tag_queue = string["tags"].copy();
			while len(tag_queue) > 0:
				tag = f"CAT_CONTENT_TAG_{tag_queue[0]}";
				tag_queue.pop(0);
				tag_str += tag;
				if len(tag_queue) > 0:
					tag_str += " | ";
			_print(f"\t\t.tags = {tag_str},");
		else:
			_print("\t\t.tags = CAT_CONTENT_TAG_NONE,");
		_print("\t},");
	_print("};");
	
	file.close();
	pass;

file = open("data/notices.json", "r+");
json_data = json.load(file);
file.close();

tags = [];
strings = json_data;
for string in strings:
	for tag in string["tags"]:
		if not tag in tags:
			tags.append(tag);

write_header(strings, tags);
write_source(strings, tags);