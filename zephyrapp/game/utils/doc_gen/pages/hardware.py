#!/usr/bin/env python3

# PAGE(Hardware)

from html_writer import HTMLWriter;
import common;

def build(here, html: HTMLWriter):
	common.title(html, "Hardware");
	
	common.gallery(html, ["/images/case.png", "/images/hardware.webp", ]);

	html.heading(1, "Top-to-Bottom");

	html.heading(2, "E-Ink Display");
	html.begin_text_block();
	html.text("The e-ink display is used to communicate device status, air quality conditions, and pet information.");
	html.text("Intervals between e-ink updates are relatively long, as updating the e-ink necessarily halts other device functions.");
	html.text("The e-ink display is non-interactive");
	html.end_text_block();

	html.heading(2, "LCD Display");
	html.begin_text_block();
	html.text("The LCD display is the device's primary display.");
	html.text("It updates many times per second and is used for all real-time gameplay.");
	html.text("The LCD display supports touch input, but with relatively low precision.");
	html.text("Some but not all gameplay takes advantage of touch support. There is typically a non-touch way to perform any action that can be performed with touches.");
	html.end_text_block();

	html.heading(3, "Display Orientation and the IMU");
	html.begin_text_block();
	html.text("The device has an on-board IMU for detecting orientation.");
	html.text("When the device's orientation is determined to be right-side-up or upside-down, the LCD and E-Ink are reoriented to match.");
	html.text("Device input logic is also reoriented to match the display.");
	html.end_text_block();

	html.heading(2, "Reset Button");
	html.begin_text_block();
	html.text("The device's reset button is situated at the back of a small aperture above the D-pad.");
	html.text("Pressing the reset button resets critical device state and prompts the device to restart itself.");
	html.text("As such, it is essential for recovering from crashes and other issues which may interfere with device logic.");
	html.text("More commonly, it is useful for waking from low-power sleep and DFU mode.");
	html.end_text_block();

	html.heading(2, "Face Buttons");
	html.begin_text_block();
	html.text("The device sports 4 face buttons: Start, Select, A, and B.");
	html.text("These buttons roughly adhere to their expected functions within video games.");
	html.text("Start and select are used for various modifications of the game context with select being more or less auxiliary to start.");
	html.text("A is used for positive action or confirmation, and B for retreat or cancellation.");
	html.end_text_block();
	
	html.heading(2, "D-Pad");
	html.begin_text_block();
	html.text("The D-pad is the main set of inputs for in-game navigation.");
	html.text("As such, most navigation is 4-directional.");
	html.end_text_block();

	html.heading(2, "USB-C Port");
	html.begin_text_block();
	html.text("Connecting the uCritAir to a power source via the USB-C port will refill its battery.");
	html.text("New builds of the system software are flashed to the uCritAir over the USB-C port.");
	html.end_text_block();

	html.heading(2, "Some Special Input Combinations");
	html.begin_text_block();
	html.line("Select + Start + Down while system is asleep: enter DFU mode.");
	html.line("Select + Start + Up: open debug menu.");
	html.end_text_block();








