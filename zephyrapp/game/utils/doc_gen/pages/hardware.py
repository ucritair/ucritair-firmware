#!/usr/bin/env python3

# PAGE(Hardware)

from html_writer import HTMLWriter;
import common;

def build(html: HTMLWriter):
	html.heading(1, "Hardware");
	
	common.banner(html, "/images/hardware.webp", 0.75);

	html.heading(2, "Power-On");
	html.text("The uCritAir ships in a protected power-off mode, to prevent draining the battery before it arrives. To turn it on for the first time, simply press the reset button with a small object such as a paperclip, and wait a moment.");

	html.heading(2, "Displays");
	html.heading(3, "E-Ink");
	html.text("The uCritAir's topmost display is a non-interacive e-ink display which is used to communicate device status, air quality, gameplay metrics, and more. The e-ink display can be manually updated from the system menu.");
	html.text("When the device is flipped upside-down, the e-ink display will flip its orientation to match.");
	html.heading(3, "LCD");
	html.text("The main display is a touchscreen LCD which is constantly updated in real time. It is where the action happens. All air quality and gameplay features are controlled through interfaces that appear on the LCD.");

	html.heading(2, "Face Buttons");
	html.text("The D-pad, A/B buttons, and start/select buttons are used to control the uCritAir's software in a manner that will be familiar to anyone who has used a gamepad. That is, in general: D-pad is for navigation, A/B are for confirm and cancel respectively, start is for entering or exiting some overarching context, and select is for changing modes within a context.");

	html.heading(2, "USB-C Port");
	html.text("Connecting the μCritAir to a power source via the USB-C port will refill its battery.");
	html.text("New builds of the system software are flashed to the μCritAir over the USB-C port.");








