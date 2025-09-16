
#!/usr/bin/env python3

# PAGE(Decoration)

from html_writer import HTMLWriter;
import common;
import pages;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);

	html.text("The majority of the apartment's floor space is implicitly devided into a grid of tiles. " \
	"The grid is 15 tiles wide and 10 tiles tall, reaching from the top of the floor to the area above the screen buttons. " \
	"Deocoration items can be placed in the grid. Some decorations also serve gameplay functions. " \
	"To place decorations in the grid, one must first enter decoration mode by clicking on the screen button that sports a chair icon.");

	html.heading(3, "Acquiring Decorations");
	html.text("Decorations are some of the most prevalent items in the game. " \
	"The majority of them can be purchased from the shop, accessed via the vending machine. " \
	"Some decorations, however, can only be obtained through special gameplay avenues.");

	html.heading(3, "Placing Decorations");
	html.text("By default, when decoration mode is entered, the cursor will be in placement mode but no item will be selected for placement. " \
	"To select an item for placement, press A while the cursor is in placement mode. " \
	"If the cursor is over an empty space, an item selection dialogue will appear. " \
	"If the cursor is over a placed item, you will pick up that item for placement elsewhere.");
	html.text("Some decorations can have a child decoration placed atop them. Clicking on a decoration that has a child will pick up the child item first. " \
	"Once an item is picked up, you may place it by pressing A again. " \
	"To place one item atop another, if the two support such a configuration, simply place the child item within the bounds of the parent item. " \
	"If two items cannot be stacked, their bounds may not overlap.");

	html.heading(3, "Removing Decorations");
	html.text("Pressing select will change the mode of the cursor. " \
	"The next mode after placement is removal. The removal cursor looks like a red X. " \
	"To remove an item from the apartment and return it to your inventory, hover over it and press A.");

	html.heading(3, "Configuring Decorations");
	html.text("The next mode after removal is configuration. The configuration cursor looks like a blue rotor. " \
	"Only some decorations are reconfigurable. To cycle through configurations, repeatedly press A while hovering over a decoration.");