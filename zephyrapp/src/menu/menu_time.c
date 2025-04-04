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
#include "sprite_assets.h"

#include "menu_system.h"

#include "flash.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(menu_time, LOG_LEVEL_DBG);

struct tm local;

struct {
	int hours, mins, secs;
} local_wakeup;

int local_nox_every;

struct {
	int mins, secs;
} local_dim_after;

struct {
	int mins, secs;
} local_sleep_after;

enum which {RTC, WAKEUP, NOX, DIM, SLEEP};

const struct {const int len; const int pad; int* edit; enum which which;} edits[] = {
	{3, 1, &local.tm_mon, RTC},
	{2, 1, &local.tm_mday, RTC},
	{4, 2, &local.tm_year, RTC},
	{2, 1, &local.tm_hour, RTC},
	{2, 1, &local.tm_min, RTC},
	{2, 0, &local.tm_sec, RTC},
	{2, 2, &local_wakeup.hours, WAKEUP},
	{2, 2, &local_wakeup.mins, WAKEUP},
	{2, 0, &local_wakeup.secs, WAKEUP},
	{0, 0, &local_nox_every, NOX},
	{2, 2, &local_dim_after.mins, DIM},
	{2, 0, &local_dim_after.secs, DIM},
	{2, 2, &local_sleep_after.mins, SLEEP},
	{2, 0, &local_sleep_after.secs, SLEEP}
};

#define NUM_EDITS (sizeof(edits)/sizeof(edits[0]))

int time_edit_time_selector = 0;

void CAT_MS_time(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_time);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();

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
				switch (edits[time_edit_time_selector].which)
				{
				case RTC:
					set_rtc_counter(&local);
					break;

				case WAKEUP:
					sensor_wakeup_rate = local_wakeup.hours*60*60 + local_wakeup.mins*60 + local_wakeup.secs;
					sensor_wakeup_rate = clamp(sensor_wakeup_rate, 15, 60*60*16);
					break;

				case NOX:
					if (nox_every_n_samples != local_nox_every)
						nox_every_n_samples_counter = 0;

					nox_every_n_samples = local_nox_every;
					break;

				case DIM:
					dim_after_seconds = local_dim_after.mins*60 + local_dim_after.secs;
					dim_after_seconds = clamp(dim_after_seconds, 10, 60*10);

					if (sleep_after_seconds < dim_after_seconds)
						sleep_after_seconds = dim_after_seconds + 1;

					break;

				case SLEEP:
					sleep_after_seconds = local_sleep_after.mins*60 + local_sleep_after.secs;
					sleep_after_seconds = clamp(sleep_after_seconds, 10, 60*10);

					if (sleep_after_seconds < dim_after_seconds)
						dim_after_seconds = sleep_after_seconds - 1;

					break;
				}
			}

			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void do_edit_clock_field(enum which which)
{
	for (int i = 0; i < NUM_EDITS; i++)
	{
		if (edits[i].which != which) continue;

		bool editing = i == time_edit_time_selector;
		CAT_gui_textf("%.*s%*s", edits[i].len, editing?"^^^^":"    ", edits[i].pad, "");
	}
}

void CAT_render_time()
{
	CAT_gui_title(false, NULL, &icon_exit_sprite, "SET TIME");
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
	
	time_t now = get_current_rtc_time();

	gmtime_r(&now, &local);

	// LOG_DBG("at gmtime_r now=%lld; year=%d", now, local.tm_year);
	
	CAT_gui_textf("%s ", month_names[local.tm_mon]);
	CAT_gui_textf("%2d ", local.tm_mday);
	CAT_gui_textf("%4d, ", local.tm_year);
	CAT_gui_textf("%2d:", local.tm_hour);
	CAT_gui_textf("%02d:", local.tm_min);
	CAT_gui_textf("%02d", local.tm_sec);
	CAT_gui_line_break();

	do_edit_clock_field(RTC);

	CAT_gui_line_break();
	// CAT_gui_line_break();

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

	CAT_gui_textf("Sample Rate:\n");
	CAT_gui_textf("%2dh ", local_wakeup.hours);
	CAT_gui_textf("%02dm ", local_wakeup.mins);
	CAT_gui_textf("%02ds", local_wakeup.secs);
	CAT_gui_line_break();

	do_edit_clock_field(WAKEUP);

	CAT_gui_line_break();
	// CAT_gui_line_break();

	local_nox_every = nox_every_n_samples;

	bool nox_selected = edits[time_edit_time_selector].which == NOX;
	CAT_gui_textf("Sample NOX+VOC every %s%d%s\n", nox_selected?">":"", local_nox_every, nox_selected?"<":"");
	CAT_gui_textf("samples. %s", local_nox_every!=0?"(Extra batt use)":"(Disabled)");

	CAT_gui_line_break();
	CAT_gui_line_break();


	// +15 is approximate time to get a fix and log
	int hrs = get_hours_of_logging_at_rate(sensor_wakeup_rate+15);
	int days = hrs/24;
	hrs %= 24;
	CAT_gui_textf("Space for %dd %dh\nlogging left at this rate", days, hrs);

	CAT_gui_line_break();
	CAT_gui_line_break();

	local_dim_after.mins = 0;
	local_dim_after.secs = dim_after_seconds;

	while (local_dim_after.secs >= 60)
	{
		local_dim_after.mins++;
		local_dim_after.secs -= 60;
	}

	CAT_gui_textf("Dim Screen After:\n");
	CAT_gui_textf("%02dm ", local_dim_after.mins);
	CAT_gui_textf("%02ds", local_dim_after.secs);
	CAT_gui_line_break();

	do_edit_clock_field(DIM);

	CAT_gui_line_break();
	// CAT_gui_line_break();

	local_sleep_after.mins = 0;
	local_sleep_after.secs = sleep_after_seconds;

	while (local_sleep_after.secs >= 60)
	{
		local_sleep_after.mins++;
		local_sleep_after.secs -= 60;
	}

	CAT_gui_textf("Sleep After:\n");
	CAT_gui_textf("%02dm ", local_sleep_after.mins);
	CAT_gui_textf("%02ds", local_sleep_after.secs);
	CAT_gui_line_break();

	do_edit_clock_field(SLEEP);

	CAT_gui_line_break();
	// CAT_gui_line_break();
}
