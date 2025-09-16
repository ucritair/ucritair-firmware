
#!/usr/bin/env python3

# PAGE(Arcade)

from html_writer import HTMLWriter;
import common;
import pages;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);

	html.text("The arcade offers minigames to be played for enjoyment and the occasional reward." \
	"The current games are Snack, Sweep, and Foursquares");

	html.heading(3, "Snack");
	html.text("Maneuver Snackcat with the D-pad to snap up snacks and coins." \
	"Beware! If you collide with the walls or yourself, it's game over.");

	html.heading(3, "Sweep");
	html.text("Carefully unearth a minefield tile-by-tile." \
	"Tearing up a mine spells certain doom. The number on a tile indicates its distance in tiles from the nearest mine.");

	html.heading(3, "Foursquares");
	html.text("Stack up 4-squared geometry to form a continuous block." \
	"A fully-formed block will dissolve and score you points." \
	"If your geometry touches the ceiling, it's game over.");