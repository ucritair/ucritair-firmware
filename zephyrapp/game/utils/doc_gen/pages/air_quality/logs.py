
#!/usr/bin/env python3

# PAGE(Logs)

from html_writer import HTMLWriter;
import common;
import pages;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);

	html.begin_text_block();
	html.text("Air quality data is logged at a user-configurable rate.");
	html.text("A higher logging rate will fill the uCritAir's storage space more quickly and also consume more battery.");
	html.text("Logs are intended to persist indefinitely until cleared.");
	html.text("If logs are cleared, dashboard features that rely on logs will be unavailable until a handful of new logs are made.");
	html.newline();
	html.newline();
	html.text("The logged metrics are:");
	html.begin_list();
	html.list_item("CO2");
	html.list_item("PM 2.5");
	html.list_item("PN 10.0");
	html.list_item("NOx Index");
	html.list_item("VOC Index");
	html.list_item("Temperature");
	html.list_item("Relative Humidity");
	html.list_item("Pressure");
	html.end_list();
	html.end_text_block();