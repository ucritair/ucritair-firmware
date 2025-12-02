#include "menu_time.h"
#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include <stdio.h>
#include "cat_deco.h"
#include "cat_item.h"
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

void CAT_MS_time(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_time);
			break;
		}
		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_pushdown_pop();

			if (CAT_input_pulse(CAT_BUTTON_LEFT))
				time_edit_time_selector--;
			if (CAT_input_pulse(CAT_BUTTON_RIGHT))
				time_edit_time_selector++;
			time_edit_time_selector = CAT_clamp(time_edit_time_selector, 0, NUM_EDITS-1);

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
					sensor_wakeup_period = local_wakeup.hours*60*60 + local_wakeup.mins*60 + local_wakeup.secs;
					sensor_wakeup_period = CAT_clamp(sensor_wakeup_period, 15, 60*60*16);
					break;

				case NOX:
					if (nox_sample_period != local_nox_every)
						nox_sample_counter = 0;

					nox_sample_period = local_nox_every;
					break;

				case DIM:
					dim_after_seconds = local_dim_after.mins*60 + local_dim_after.secs;
					dim_after_seconds = CAT_clamp(dim_after_seconds, 10, 60*10);

					if (sleep_after_seconds < dim_after_seconds)
						sleep_after_seconds = dim_after_seconds + 1;

					break;

				case SLEEP:
					sleep_after_seconds = local_sleep_after.mins*60 + local_sleep_after.secs;
					sleep_after_seconds = CAT_clamp(sleep_after_seconds, 10, 60*10);

					if (sleep_after_seconds < dim_after_seconds)
						dim_after_seconds = sleep_after_seconds - 1;

					break;
				}
			}

			break;
		}
		case CAT_FSM_SIGNAL_EXIT:
			break;
	}
}

#define PAD 8
static int cursor_x = PAD;
static int cursor_y = PAD;

static void draw_page(const char* title)
{
	cursor_x = PAD;
	cursor_y = PAD;

	CAT_frameberry(CAT_WHITE);
	cursor_y = CAT_draw_textf(cursor_x, cursor_y, "%s\n", title);
	cursor_y += PAD;
	CAT_rowberry(cursor_y, cursor_y+1, CAT_BLACK);
	cursor_y += PAD;
}

void do_edit_clock_field(enum which which)
{
	for (int i = 0; i < NUM_EDITS; i++)
	{
		if (edits[i].which != which) continue;
		bool editing = i == time_edit_time_selector;
		CAT_draw_textf(cursor_x, cursor_y, "%.*s%*s", edits[i].len, editing?"^^^^":"    ", edits[i].pad, "");
		cursor_x += CAT_get_drawn_strlen() * CAT_GLYPH_WIDTH;
	}
	cursor_y += CAT_TEXT_LINE_HEIGHT;
	cursor_x = PAD;
}

void CAT_render_time()
{
	draw_page("SET TIME");
	
	time_t now = get_current_rtc_time();
	gmtime_r(&now, &local);

	// LOG_DBG("at gmtime_r now=%lld; year=%d", now, local.tm_year);
	
	cursor_y = CAT_draw_textf
	(
		cursor_x, cursor_y,
		"%s %2d %4d, %02d:%02d:%02d\n",
		month_names[local.tm_mon], local.tm_mday, local.tm_year, local.tm_hour, local.tm_min, local.tm_sec
	);
	do_edit_clock_field(RTC);

	local_wakeup.hours = 0;
	local_wakeup.mins = 0;
	local_wakeup.secs = sensor_wakeup_period;
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

	cursor_y = CAT_draw_textf
	(
		cursor_x, cursor_y,
		"Sample Rate:\n%02dh %02dm %02ds\n",
		local_wakeup.hours, local_wakeup.mins, local_wakeup.secs
	);
	do_edit_clock_field(WAKEUP);

	local_nox_every = nox_sample_period;
	bool nox_selected = edits[time_edit_time_selector].which == NOX;
	cursor_y = CAT_draw_textf
	(
		cursor_x, cursor_y,
		"Sample NOX+VOC every %s%d%s\nsamples. %s\n",
		nox_selected?">":"", local_nox_every, nox_selected?"<":"",
		local_nox_every!=0?"(Extra batt use)":"(Disabled)"
	);

	// +15 is approximate time to get a fix and log
	int hrs = get_hours_of_logging_at_rate(sensor_wakeup_period+15);
	int days = hrs/24;
	hrs %= 24;
	cursor_y = CAT_draw_textf(cursor_x, cursor_y, "Space for %dd %dh\nlogging left at this rate\n\n", days, hrs);

	local_dim_after.mins = 0;
	local_dim_after.secs = dim_after_seconds;
	while (local_dim_after.secs >= 60)
	{
		local_dim_after.mins++;
		local_dim_after.secs -= 60;
	}

	cursor_y = CAT_draw_textf(cursor_x, cursor_y, "Dim screen after:\n%02dm %02ds\n", local_dim_after.mins, local_dim_after.secs);
	do_edit_clock_field(DIM);

	local_sleep_after.mins = 0;
	local_sleep_after.secs = sleep_after_seconds;
	while (local_sleep_after.secs >= 60)
	{
		local_sleep_after.mins++;
		local_sleep_after.secs -= 60;
	}

	cursor_y = CAT_draw_textf(cursor_x, cursor_y, "Sleep after:\n%02dm %02ds\n", local_sleep_after.mins, local_sleep_after.secs);
	do_edit_clock_field(SLEEP);
}
