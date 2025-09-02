
#!/usr/bin/env python3

# PAGE(Apartment)

from html_writer import HTMLWriter;
import common;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);
	
	html.start_text_block();
	html.text("The apartment is where your critter lives.");
	html.text("The critter can be interacted with in the apartment, and it can be decorated with a variety of items.");
	html.text("Additionally, it contains diegetic access points for the dashboard, the vending machine, and the arcade.");
	html.text("The GUI overlaid on the apartment screen includes buttons to access the 3 care mechanics, decoration mode, and the menu.");
	html.end_text_block();

	html.newline();
	
	html.start_text_block();
	html.text("Generally speaking, the apartment is the \"bottom\" of the game's state machine. All roads lead back to the apartment.");
	html.text("Exiting the dashboard will return you to the apartment, as will exiting the overworld.");
	html.end_text_block();
	