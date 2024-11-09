#include "menu_time.h"
#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include <stdio.h>
#include "cat_deco.h"
#include "cat_actions.h"
#include "cat_bag.h"
#include "rtc.h"
#include "airquality.h"

void CAT_MS_aqi(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			bag_state.base = 0;
			bag_state.idx = 0;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, CAT_MS_menu);

			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			bag_state.destination = CAT_MS_menu;
			break;
	}
}

void CAT_render_aqi()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("AIR QUALITY ");
	CAT_gui_image(fbut_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_line_break();

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
	
	char buf[64];
#define textf(...) snprintf(buf, sizeof(buf), __VA_ARGS__); CAT_gui_text(buf)
	
	if (current_readings.sunrise.uptime_last_updated)
	{
		textf("CO2: %4dppm", (int)current_readings.sunrise.ppm_filtered_compensated);
	}
	else
	{
		textf("CO2 sensor starting...");
	}
	CAT_gui_line_break();
	if (current_readings.sunrise.uptime_last_updated)
	{
		textf("PM2.5: %2.1f  |  PM10: %2.1f", (double)current_readings.sen5x.pm2_5, (double)current_readings.sen5x.pm10_0);
	}
	else
	{
		textf("PM2.5/PM10 sensor starting...");
	}
	CAT_gui_line_break();
	if (current_readings.sen5x.voc_index && current_readings.sen5x.nox_index)
	{
		textf("VOC %3.0f  |  NOX %3.0f", (double)current_readings.sen5x.voc_index, (double)current_readings.sen5x.nox_index);
	}
	else
	{
		textf("VOC/NOX sensor starting...");
	}
	CAT_gui_line_break();
	if (current_readings.sen5x.temp_degC)
	{
		textf("%2.1fC  |  %2.0f%%RH", (double)current_readings.sen5x.temp_degC, (double)current_readings.sen5x.humidity_rhpct);
	}
	else
	{
		textf("Temp/RH sensor starting...");
	}
}
