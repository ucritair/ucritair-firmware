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

def title(html: HTMLWriter, text):
	html.open_tag("div");
	html.heading(1, text, style="display:inline; padding-right:1rem");
	html.open_tag("a", href="/");
	html.image(
		"/images/thumbnails/icon_menu_sprite.png",		
		style="display:inline; vertical-align:bottom; margin-bottom:0.5rem;"
	);
	html.close_tag();
	html.close_tag();
	html.one_token("br");