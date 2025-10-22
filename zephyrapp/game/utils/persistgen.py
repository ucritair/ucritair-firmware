#!/usr/bin/env python3

import re;

def parse_signatures(text):
	raw = re.findall(r"extern volatile\s*([A-z0-9_]+)\s*([a-zA-Z_0-9]+)(\[[0-9]+\])?;", text);
	table = [];
	for t, n, q in raw:
		row = {};
		row["type"] = t;
		row["name"] = n;
		if len(q) > 0:
			row["quant"] = q;
		table.append(row);
	return table;

def generate_header():
	text = "#pragma once\n" \
	"#ifdef CAT_DESKTOP\n" \
	"\n" \
	"#include \"cat_persist.h\"\n" \
	"#include <unistd.h>\n" \
	"\n";
	return text;

def generate_struct(table):
	text = "typedef struct\n" \
	"{\n";
	for row in table:
		text += f"\t{row["type"]} {row["name"]}{row["quant"] if "quant" in row else ""};\n";
	text += "} CAT_persist_archive;\n\n"
	return text;

def generate_write(table):
	text = "void CAT_write_persist_archive(int fd)\n" \
	"{\n" \
	"\tCAT_persist_archive archive = {};\n";
	for row in table:
		if "quant" in row:
			text += f"\tmemcpy(archive.{row["name"]}, {row["name"]}, sizeof(archive.{row["name"]}));\n";
		else:
			text += f"\tarchive.{row["name"]} = {row["name"]};\n";
	text += "\twrite(fd, &archive, sizeof(archive));\n"
	text += "}\n\n"
	return text;

def generate_read(table):
	text = "void CAT_read_persist_archive(int fd)\n" \
	"{\n" \
	"\tCAT_persist_archive archive = {};\n" \
	"\tread(fd, &archive, sizeof(archive));\n";
	for row in table:
		if "quant" in row:
			text += f"\tmemcpy({row["name"]}, archive.{row["name"]}, sizeof({row["name"]}));\n";
		else:
			text += f"\t{row["name"]} = archive.{row["name"]};\n";
	text += "}\n\n"
	return text;

def generate_footer():
	text = "#endif\n" \
	"\n";
	return text;

src = "";

with open("src/cat_persist.h") as file:
	table = parse_signatures(file.read());
	src += generate_header();
	src += generate_struct(table);
	src += generate_write(table);
	src += generate_read(table);
	src += generate_footer();

with open("src/cat_persist_archive.h", "w") as file:
	file.write(src);