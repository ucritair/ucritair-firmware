#!/usr/bin/env python3

# PAGE(Playing)

from html_writer import HTMLWriter;
import common;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);

	html.start_text_block();
	html.end_text_block();