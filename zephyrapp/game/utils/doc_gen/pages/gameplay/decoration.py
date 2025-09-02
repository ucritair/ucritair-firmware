
#!/usr/bin/env python3

# PAGE(Decoration)

from html_writer import HTMLWriter;
import common;
import pages;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);