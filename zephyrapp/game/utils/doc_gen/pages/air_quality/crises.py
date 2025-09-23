
#!/usr/bin/env python3

# PAGE(Crises)

from html_writer import HTMLWriter;
import common;
import pages;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);

	common.gallery(html, ["/images/dashboard/crisis.png", "/images/apartment/crisis_report.png"]);

	html.begin_text_block();
	html.text("Crises occur when the uCritAir detects that air quality has reached critical badness in some way.");
	html.text("There are various crisis types and severity levels that correspond to different air quality metrics and the points at which they become worrisome.");
	html.text("In-game avenues for alleviating crises exist, but crises are fundamentally intended to inform you of real-life air quality and prompt real-life mitigation efforts.");
	html.end_text_block();

	html.heading(3, "Type and Severity");
	html.text("With some exceptions, for each crisis type, a crisis of each severity level is possible.");
	html.text("There are 4 crisis types:");
	html.begin_list();
	html.list_item("Carbon Dioxide (CO2)");
	html.list_item("Particulate Matter (PM 2.5)");
	html.list_item("Volatile Compounds (VOx/NOC)");
	html.list_item("Wet Bulb (Temp./RH)");
	html.end_list();
	html.text("And 3 severity levels:");
	html.begin_list();
	html.list_item("Mild");
	html.list_item("Moderate");
	html.list_item("Severe");
	html.end_list();
	html.text("If multiple metrics are bad enough to cause a crisis, the crisis type for which the most severe conditions are present is made active.");

	html.heading(3, "Mitigation");
	html.begin_text_block();
	html.text("Crises can be mitigated either naturally or via in-game intervention.");
	html.text("Gameplay interventions work via the sacrifice of mitigation items that you may keep in your inventory or place in your apartment.");
	html.text("Carbon Dioxide crises can be mitigated by consuming a UV lamp placed in the apartment.");
	html.text("Mild Particulate Matter or Volatile Compounds crises can be mitigated by consuming a mask from the inventory.");
	html.text("More severe crises of those types be mitigated by consuming an air purifier placed in the apartment.");
	html.end_text_block();
	html.newline();
	html.begin_text_block();
	html.text("It's important to keep in mind that in-game mitigations only dismiss in-game crises.");
	html.text("The real-life air quality that spawned the crisis will continue to affect you and, after the crisis cooldown has passed, will even continue to affect your critter.");
	html.text("The best way to deal with any air quality crisis is to improve the quality of the air around you.");
	html.end_text_block();

	html.heading(3, "Effects");
	html.begin_text_block();
	html.text("Any crisis that is left unmitigated for too long will affect your critter's lifespan.");
	html.text("Lifespan damage is only dealt once per crisis, and scales according to the crisis' severity as well as the degree of neglect.");
	html.text("The crisis dashboard and crisis report will both clearly indicate the amount of time left until a crisis turns disastrous and deals lifespan damage.");
	html.end_text_block();