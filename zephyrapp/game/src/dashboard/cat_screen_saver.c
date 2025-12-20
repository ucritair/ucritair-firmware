#include "cat_screen_saver.h"

#include "cat_gui.h"
#include "cat_render.h"
#include "cat_input.h"
#include "cat_save.h"
#include "cat_room.h"
#include "cat_colours.h"
#include "cat_persist.h"

static CAT_button spell[] =
{
	CAT_BUTTON_UP,
	CAT_BUTTON_UP,
	CAT_BUTTON_DOWN,
	CAT_BUTTON_DOWN,
	CAT_BUTTON_LEFT,
	CAT_BUTTON_RIGHT,
	CAT_BUTTON_LEFT,
	CAT_BUTTON_RIGHT,
	CAT_BUTTON_B,
	CAT_BUTTON_A,
};

#define MARGIN 12

static CAT_datetime twelve_hour_time(CAT_datetime datetime, char* ampm)
{
	if(datetime.hour >= 12)
	{
		if(datetime.hour > 12)
			datetime.hour -= 12;
		strncpy(ampm, "PM", 3);
	}
	else
	{
		if(datetime.hour == 0)
			datetime.hour = 12;
		strncpy(ampm, "AM", 3);	
	}
	return datetime;
}

static void draw_screen_saver()
{
	CAT_timestamp now = CAT_get_RTC_now();
	CAT_datetime today;
	CAT_make_datetime(now, &today);

	char ampm[3];
	CAT_datetime twelve_hour = twelve_hour_time(today, ampm);

	CAT_frameberry(CAT_GRAPH_BG);
	int cursor_y = MARGIN;

	CAT_set_text_colour(CAT_GRAPH_FG);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_textf
	(
		MARGIN, cursor_y, "%.2d/%.2d/%.4d\n%.2d:%.2d %s\n",
		today.month, today.day, today.year,
		twelve_hour.hour, twelve_hour.minute,
		ampm
	);

	if(last_sensor_timestamp > 0)
	{
		uint32_t since_sample = now - last_sensor_timestamp;
		CAT_set_text_colour(CAT_GRAPH_FG);
		cursor_y = CAT_draw_textf
		(
			MARGIN, cursor_y, "\nLast sample taken\n%d minutes, %d seconds ago.\n",
			since_sample / CAT_MINUTE_SECONDS, since_sample % CAT_MINUTE_SECONDS
		);
	}
	if(last_log_timestamp > 0)
	{
		uint32_t since_log = now - last_log_timestamp;
		CAT_set_text_colour(CAT_GRAPH_FG);
		cursor_y = CAT_draw_textf
		(
			MARGIN, cursor_y, "\nLast log made\n%d minutes, %d seconds ago.\n",
			since_log / CAT_MINUTE_SECONDS, since_log % CAT_MINUTE_SECONDS
		);
	}
	else if(is_persist_fresh)
	{
		CAT_set_text_colour(CAT_GRAPH_FG);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, "\nWaiting for data...\n");
	}

	CAT_set_text_colour(CAT_GRAPH_FG);
	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	cursor_y = CAT_draw_textf
	(
		MARGIN, cursor_y, "\nResearch instrument -- please leave plugged in and do not touch.\n"
	);

	CAT_set_text_colour(CAT_GRAPH_FG);
	cursor_y = CAT_draw_textf(MARGIN, CAT_LCD_SCREEN_H-MARGIN-CAT_TEXT_LINE_HEIGHT*2, "For more information,\ncontact tretyakova@wisc.edu");
}

void CAT_MS_screen_saver(CAT_FSM_signal signal)
{
	static int exit_level = 0;
	
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(draw_screen_saver);
			exit_level = 0;
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_input_spell(spell))
			{
				exit_level++;
				CAT_input_buffer_clear();
				if(exit_level == 2)
					CAT_pushdown_rebase(CAT_MS_room);
			}
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		{
			
		}
		break;
	}
}