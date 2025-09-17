#!/usr/bin/env python3

# PAGE(Fishing)

from html_writer import HTMLWriter;
import common;

def build(here, html: HTMLWriter):
	common.index_banner(html);
	common.title(html, here.title);

	html.text("Fishing is a skilled leisure activity that will help your pet regain focus, in addition to providing it with fish. " \
	"To launch the fishing minigame, press the screen button with a fishing pole on it.");

	html.heading(3, "Casting A Line");
	html.text("The quality of your cast influences the quality of the fish you can catch. " \
	"A good cast lands close to the target, which is indicated by the red dot seen along the casting line. " \
	"Your reticle will slide along the line. Press A to cast when it is right over top the target.");

	html.heading(3, "Luring");
	html.text("Once your line is cast, you must attract a fish to your line. " \
	"You will start with a free-moving reticle. Attract the fish's attention by moving your reticle into its field of view. " \
	"Once the fish has locked on to your reticle, intercept the fish's mouth at the center of the screen to secure its interest. " \
	"If you are too slow and fail to intercept the fish, you will have to re-cast.");

	html.heading(3, "Biting");
	html.text("Once the fish and your bobber are locked into place, you must wait for the fish to bite. " \
	"A bite is indicated by the bobber sinking underwater, and is accompanied by a ripple on the water. " \
	"Not all ripples indicate a bite though -- you must pay attention to whether or not your bobber has sunk. " \
	"As soon as the fish bites and the bobber is sunk, press A as fast as you can.");
	
	html.heading(3, "Reeling");
	html.text("Pressing A on a successful bite will take you to the final stage of a catch -- reeling. " \
	"Higher quality fish are considerably harder to reel in than lower quality fish. " \
	"To reel a fish in, you must fill the reel meter seen above the reel line." \
	"The reel line resembles a horizontally oriented version of the casting line, and the reel meter is the red progress bar above it. " \
	"The reel meter fills as long as your reticle is within the target range, indicated by the red mark on the reel line. " \
	"Hold or tap A to accelerate your reticle from left to right along the reel line.");

	html.heading(3, "Inspection");
	html.text("After you've reeled a fish in, you will be taken to the inspection screen. " \
	"There, you can inspect the stats of the particular fish you caught as well as see how much XP you got from the whole affair.");