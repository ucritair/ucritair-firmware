#!/usr/bin/env python3

# PAGE(Home)

from html_writer import HTMLWriter, HTMLMode;
import common;
import pages;

def build(html: HTMLWriter):
	common.banner(html, "/images/banner_2.png", 0.5);

	common.title(html, "Home");
	html.start_text_block();
	html.line("Welcome to the uCritAir documentation.");
	html.line("The uCritAir is an air quality sensor and tracker and a richly interactive virtual pet in one device.");
	html.line("Every aspect of the uCritAir is actively in development, with firmware updates releasing frequently.");
	html.line("This website serves as documentation for all things uCritAir.");
	html.line(f"Other uCritAir related sites are the {html.link("main page", "https://www.ucritter.com/", mode=HTMLMode.INLINE)} and the {html.link("source code", "https://github.com/ucritair/ucritair-firmware", mode=HTMLMode.INLINE)}.");
	html.end_text_block();
	html.newline();

	common.navigator(html, pages.here());