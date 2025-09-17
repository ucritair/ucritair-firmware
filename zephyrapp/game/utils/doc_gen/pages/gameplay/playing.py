#!/usr/bin/env python3

# PAGE(Playing)

from html_writer import HTMLWriter;
import common;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);

	html.text("To play with your pet, press the screen button with a pile of toy blocks on it. " \
	"When you do, you will be prompted to select a toy from your inventory. " \
	"Different toys enable different types of play.");

	html.heading(3, "Laser Pointer")
	html.text("The laser pointer is the toy you start out with. " \
	"Guide the laser pointer around the apartment. The pet will try to pounce on it. " \
	"If your pet catches the laser pointer, it will play with it. Enough playing and pouncing will restore your pet's spirit.");