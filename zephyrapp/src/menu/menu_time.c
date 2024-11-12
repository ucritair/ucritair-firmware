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

#include "menu_system.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(menu_time, LOG_LEVEL_DBG);

struct tm local;

struct {
	int hours, mins, secs;
} local_wakeup;

const struct {const int len; const int pad; int* edit;} edits[] = {
	{3, 1, &local.tm_mon},
	{2, 1, &local.tm_mday},
	{4, 2, &local.tm_year},
	{2, 1, &local.tm_hour},
	{2, 1, &local.tm_min},
	{2, 0, &local.tm_sec},
	{2, 2, &local_wakeup.hours},
	{2, 2, &local_wakeup.mins},
	{2, 0, &local_wakeup.secs}
};

#define NUM_CLOCK_EDITS 6
#define NUM_RATE_EDITS 3
#define NUM_EDITS (sizeof(edits)/sizeof(edits[0]))

int time_edit_time_selector = 0;

void CAT_MS_time(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(CAT_MS_system_menu);

			if (CAT_input_pulse(CAT_BUTTON_LEFT))
				time_edit_time_selector--;
			if (CAT_input_pulse(CAT_BUTTON_RIGHT))
				time_edit_time_selector++;
			time_edit_time_selector = clamp(time_edit_time_selector, 0, NUM_EDITS-1);

			int up = CAT_input_pulse(CAT_BUTTON_UP);
			int dn = CAT_input_pulse(CAT_BUTTON_DOWN);
			if (up)
				(*edits[time_edit_time_selector].edit)++;
			if (dn)
				(*edits[time_edit_time_selector].edit)--;

			if (up||dn)
			{
				set_rtc_counter(&local);
				sensor_wakeup_rate = local_wakeup.hours*60*60 + local_wakeup.mins*60 + local_wakeup.secs;
				sensor_wakeup_rate = clamp(sensor_wakeup_rate, 15, 60*60*16);
			}

			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_time()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("SET TIME ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_line_break();

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
	
	time_t now = get_current_rtc_time();

	gmtime_r(&now, &local);

	char buf[64];
#define textf(...) snprintf(buf, sizeof(buf), __VA_ARGS__); CAT_gui_text(buf)

	// LOG_DBG("at gmtime_r now=%lld; year=%d", now, local.tm_year);
	
	textf("%s ", month_names[local.tm_mon]);
	textf("%2d ", local.tm_mday);
	textf("%4d, ", local.tm_year);
	textf("%2d:", local.tm_hour);
	textf("%02d:", local.tm_min);
	textf("%02d", local.tm_sec);

	CAT_gui_line_break();

	for (int i = 0; i < NUM_CLOCK_EDITS; i++)
	{
		bool editing = i == time_edit_time_selector;
		textf("%.*s%*s", edits[i].len, editing?"^^^^":"    ", edits[i].pad, "");
	}

	CAT_gui_line_break();
	CAT_gui_line_break();

	local_wakeup.hours = 0;
	local_wakeup.mins = 0;
	local_wakeup.secs = sensor_wakeup_rate;

	while (local_wakeup.secs >= (60*60))
	{
		local_wakeup.hours++;
		local_wakeup.secs -= 60*60;
	}

	while (local_wakeup.secs >= 60)
	{
		local_wakeup.mins++;
		local_wakeup.secs -= 60;
	}

	textf("Sample Rate: ");
	CAT_gui_line_break();

	textf("%2dh ", local_wakeup.hours);
	textf("%02dm ", local_wakeup.mins);
	textf("%02ds", local_wakeup.secs);

	CAT_gui_line_break();

	for (int i = NUM_CLOCK_EDITS; i < NUM_CLOCK_EDITS+NUM_RATE_EDITS; i++)
	{
		bool editing = i == time_edit_time_selector;
		textf("%.*s%*s", edits[i].len, editing?"^^^^":"    ", edits[i].pad, "");
	}

	CAT_gui_line_break();
	CAT_gui_line_break();

	if (sensor_wakeup_rate < MIN_WAKEUP_RATE_TO_DEEP_SLEEP)
	{
		textf("NOTE: high power drain");
		CAT_gui_line_break();
	}
}
