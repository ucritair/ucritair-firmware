
#!/usr/bin/env python3

# PAGE(Air Quality)

from html_writer import HTMLWriter;
import common;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);

	html.heading(2, "Metrics");
	html.start_text_block();
	html.text("The uCritAir is aware of a wide range of air quality metrics:");
	html.text("CO2, PM 1.0-10.0, PN 0.5-10.0, NOx index, VOC index, temperature, relative humidity, and pressure.");
	html.text("All metrics are tracked live, and many of them are permanently logged.");
	html.text("The most critical of them are presented on dashboard screens for easy viewing.");
	html.end_text_block();

	html.heading(2, "Air Quality and Gameplay");
	html.start_text_block();
	html.text("Changes in real-life air quality impact the uCritAir's gameplay.");
	html.text("Both good and bad quality passively influences the game's world, and can have active effects on your critter.");
	html.text("If air quality reaches a critical low point, a corresponding crisis will occur in-game.");
	html.text("Crises can reduce your critter's lifespan if not addressed properly.");
	html.end_text_block();
	
	html.heading(3, "Navigator");
	common.navigator(html, here.parent);