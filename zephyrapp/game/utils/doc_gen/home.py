#!/usr/bin/env python3

import json;
from html_writer import HTML_writer;
import common;

html = HTML_writer("docs/index.html");
html.start(title="Home", stylesheet="sakura.css");

common.banner(html, "images/banner_2.png", 0.5);

html.heading(1, "Home");
html.start_text_block();
html.text("Welcome to the uCritAir documentation.");
html.text("From here, you can explore a variety of documents relating to the uCritAir hardware, firmware, and software.");
html.end_text_block();
html.newline();

common.vertical_selector(html);

html.end();