#!/usr/bin/env python3

# PAGE(Feeding)

from html_writer import HTMLWriter;
import common;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);

	html.text("Feeding your pet nutritious and delicious meals is the main way to restore its vigour and one way to gain XP. " \
	"To launch the feeding minigame, press the screen button with a table setting on it. " \
	"Once the minigame is launched, tapping around the feeding area will show you some visual hints.");

	html.text("At a basic level, feeding consists of selecting food items from your inventory and arranging them into a meal on your critter's meal tray. " \
	"Meals are judged by nutritional completeness and diversity, neatness of arrangement, and also style. " \
	"Once a meal is submitted for your critter to eat, Inspector Reed will deliver some feedback on the meal. " \
	"Use this feedback to inform the way you play the minigame.");

	html.heading(3, "Selecting Food");
	html.text("To open the inventory for selection, press the screen button with a rotor icon on it. " \
	"Selected items will have a golden highlight around them. You may have up to 5 items selected at a time. " \
	"Once you have selected the items you wish to work with, exit the inventory to see them on the counter above the critter's table. " \
	"You may change which items you have selected at any time prior to meal submission.");

	html.heading(3, "Arranging Food");
	html.text("To arrange selected foods, drag them from the counter to the critter's meal tray. " \
	"Make sure that any foods which you want included in the meal are within the tray's bounds. " \
	"For optimal results, lay your foods out neatly with even spacing.");

	html.heading(3, "Submission and Feedback");
	html.text("To submit the meal you have constructed, press the screen button with an upload icon on it. " \
	"Once your meal is submitted, you will see its overall grade as well as a breakdown of its grade in specific areas. " \
	"You will also see feedback from Inspector Reed and the amount of vigour and XP earned from the meal." \
	"Inspector Reed's feedback is essential to understanding what kind of meals will get you and your critter the best results.");