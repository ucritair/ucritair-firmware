#!/usr/bin/env python3

from html_writer import HTMLWriter;
import pages;

def navigator(html: HTMLWriter, node):
	html.start_list();
	for child in node.children:
		html.list_item(f"<a href=/{child.rel_path}>{child.title}</a>");
		if isinstance(child, pages.NodePage):
			navigator(html, child);
	html.end_list();

def banner(html: HTMLWriter, path, height):
	html.image(path, style=f"display: block; margin: auto; height: {height*100}vh;");

def thumbnail(path):
	return f"<img src=/images/thumbnails/{path}>";