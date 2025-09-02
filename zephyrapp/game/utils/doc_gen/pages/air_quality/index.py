
#!/usr/bin/env python3

# PAGE(Air Quality)

from html_writer import HTMLWriter;
import common;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);
	common.navigator(html, here.parent);