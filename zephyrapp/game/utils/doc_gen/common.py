#!/usr/bin/env python3

from html_writer import HTMLWriter;
import pages;

def navigator(html: HTMLWriter, node):
	html.begin_list();
	for child in node.children:
		if child.in_path.name == "index.py":
			continue;
		html.list_item(f"<a href=/{child.rel_path}>{child.title}</a>");
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

def gallery(html: HTMLWriter, images):
	html.open_tag("div", _class="image_row");
	for image in images:
		html.open_tag("div", _class="image_column");
		html.one_tag("img", src=image, style="height:100%; max-height:4in");
		html.close_tag();
	html.close_tag();

def index_banner(html: HTMLWriter):
	html.image("/images/banner_1.png", style=f"height: 35vh;");