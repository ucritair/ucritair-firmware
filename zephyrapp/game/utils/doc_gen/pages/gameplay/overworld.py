
#!/usr/bin/env python3

# PAGE(Overworld)

from html_writer import HTMLWriter;
import common;
import pages;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);

	html.text("The overworld is the game's other main zone besides the apartment. " \
	"It is accessbile from the EXPLORE option of the menu. " \
	"In the overworld, the player may travel around and interact with whatever they find." \
	"What is present in the overworld changes from one update to the next. Always be on the lookout for new content there. ");