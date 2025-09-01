
#!/usr/bin/env python3

# PAGE(Gameplay)

from html_writer import HTMLWriter;
import common;
import pages;

def build(html: HTMLWriter):
	common.index_banner(html);
	common.title(html, pages.here().title);
	common.navigator(html, pages.here());