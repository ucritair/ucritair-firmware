
#!/usr/bin/env python3

# PAGE(Dashboard)

from html_writer import HTMLWriter;
import common;
import pages;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);

	html.heading(3, "Live Summary");
	html.begin_text_block();
	html.text("The largest element on this page is the player's aggregate air quality grade.");
	html.text("It reflects the overall air quality at the moment.");
	html.text("Beneath, smaller elements show grades for each of the main air quality metrics.");
	html.text("Real measurements are thumbnailed above the grades.");
	html.end_text_block();

	html.heading(3, "Live Details");
	html.begin_text_block();
	html.text("Here, real measurements with units can be viewed for all main air quality metrics.");
	html.text("Measurement text is coloured to represent relative badness.");
	html.text("To the right of each measurement is an arrow indicating if that metric is currently trending up or down.");
	html.end_text_block();

	html.heading(3, "Weekly Performance");
	html.begin_text_block();
	html.text("The sparklines on this page reach seven days into the past from present day.");
	html.text("The plotted values correspond to grades, with grade goodness increasing along the Y axis.");
	html.text("Below each sparkline is a readout of its maximum, mean, and minimum values.");
	html.text("To change which metric is being dislayed, tap the sparkline viewport.");
	html.end_text_block();

	html.heading(3, "Calendar");
	html.begin_text_block();
	html.text("The calendar page, once opened, begins with date selection.");
	html.text("When a date is selected, a graph of that date's logged air quality data is made available.");
	html.text("The graph's cursor is moved with the D-pad. Zoom with A/B. Beneath the graph viewport, a readout of the hovered data is avaiable.");
	html.text("To change which metric is being displayed, tap the graph viewport.");
	html.end_text_block();

	html.heading(3, "ACH Calculator");
	html.begin_text_block();
	html.text("The ACH calculator, once opened, begins with date selection.");
	html.text("When a date is selected, graphs of that date's logged CO2 and PN 10.0 data are made available in sequence.");
	html.text("On each graph, you will be prompted to select a time range to be submitted for analysis.");
	html.text("Once the time ranges are selected, e/ACH for each range will be calculated and combined into a Total ACH value.");
	html.end_text_block();

	html.heading(3, "Clock");
	html.begin_text_block();
	html.text("Nothing more and nothing less than a clock.");
	html.end_text_block();

	html.heading(3, "Crisis Dashboard");
	html.begin_text_block();
	html.text("This page must be passed through to get to the apartment or, in the case of a crisis, the crisis report.");
	html.text("In the case that a crisis occurs, this page will be highlighted and will give an overview of the crisis' nature.");
	html.text("If no crisis is underway and all is well, the highlight will disappear and the page will indicate nominal status.");
	html.end_text_block();