#!/usr/bin/env python3

from html_writer import HTML_writer;
from mime_wrapper import MIME_wrapper;

def horizontal_selector(html: HTML_writer):
	html.horizontal_selector([("Home", "index.html"), ("Hardware", "hardware.html"), ("Items", "items.html"), ("Fish", "fish.html")]);

def vertical_selector(html: HTML_writer):
	html.vertical_selector([("Home", "index.html"), ("Hardware", "hardware.html"), ("Items", "items.html"), ("Fish", "fish.html")]);

def banner(html: HTML_writer, path, height):
	html.image(path, style=f"display: block; margin: auto; height: {height*100}vh;");