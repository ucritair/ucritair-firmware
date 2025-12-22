#include "cat_research.h"

#include "cat_gui.h"
#include "cat_render.h"
#include "cat_input.h"
#include "cat_save.h"
#include "cat_room.h"
#include "cat_colours.h"
#include "cat_persist.h"
#include "sprite_assets.h"
#include "cat_gui.h"

static CAT_button config_spell[] =
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
static CAT_button dev_spell[] =
{
	CAT_BUTTON_A,
	CAT_BUTTON_B,
	CAT_BUTTON_RIGHT,
	CAT_BUTTON_LEFT,
	CAT_BUTTON_RIGHT,
	CAT_BUTTON_LEFT,
	CAT_BUTTON_DOWN,
	CAT_BUTTON_DOWN,
	CAT_BUTTON_UP,
	CAT_BUTTON_UP,
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

static void draw_research_screen()
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

	CAT_draw_sprite
	(
		&ui_battery_12px,
		quantize(CAT_get_battery_pct(), 100, 5),
		CAT_LCD_SCREEN_W-6-ui_battery_12px.width,
		6
	);
}

void CAT_MS_research_screen(CAT_FSM_signal signal)
{	
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(draw_research_screen);
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_input_spell(dev_spell))
				CAT_toggle_save_flags(CAT_SAVE_CONFIG_FLAG_DEVELOPER);
			if(CAT_input_spell(config_spell))
				CAT_pushdown_push(CAT_MS_research_config);
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		{
			
		}
		break;
	}
}

void CAT_MS_research_config(CAT_FSM_signal signal)
{
	CAT_datetime dt_real;
	CAT_datetime dt_cached;

	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_gui_begin_menu("DEVELOPER MENU"))
			{
				if(CAT_gui_begin_menu("SET TIME"))
				{
					CAT_get_datetime(&dt_cached);
					dt_real = dt_cached;

					dt_real.year = CAT_gui_menu_ticker("YEAR", dt_real.year, 0, 3000);
					dt_real.month = CAT_gui_menu_ticker("MONTH", dt_real.month, 1, 12);
					dt_real.day = CAT_gui_menu_ticker("DAY", dt_real.day, 1, 31);
					dt_real.hour = CAT_gui_menu_ticker("HOUR", dt_real.hour, 0, 23);
					dt_real.minute = CAT_gui_menu_ticker("MINUTE", dt_real.minute, 0, 59);
					dt_real.second = CAT_gui_menu_ticker("SECOND", dt_real.second, 0, 59);

					if(CAT_timecmp(&dt_real, &dt_cached) != 0)
						CAT_set_datetime(dt_real);
					
					CAT_gui_end_menu();
				}

				if(CAT_gui_menu_item("WRITE LOGS TO SD"))
				{
					CAT_SD_write_result result = CAT_write_logs_to_SD();
					if(result == CAT_SD_WRITE_OKAY)
						CAT_gui_open_popup("Write succeeded!", CAT_POPUP_STYLE_OK);
					else
						CAT_gui_open_popup("Write failed!", CAT_POPUP_STYLE_OK);
				}

				if(CAT_gui_menu_item("GO TO GAME"))
					CAT_pushdown_rebase(CAT_MS_room);
				if(CAT_gui_menu_item("GO TO SLEEP"))
					CAT_sleep();

				if(CAT_check_save_flags(CAT_SAVE_CONFIG_FLAG_DEVELOPER))
				{
					if(CAT_gui_begin_menu("DANGER ZONE"))
					{
						CAT_gui_menu_text("[WARNING]");
						CAT_gui_menu_text("This page is dangerous.");
						CAT_gui_menu_text("Go back!");
						CAT_gui_menu_text("##dz_newline");

						if(CAT_gui_begin_menu("ERASE LOGS"))
						{
							CAT_gui_menu_text("Deleting your logs is an");
							CAT_gui_menu_text("irreversible action.");
							CAT_gui_menu_text("Be very careful!");
							CAT_gui_menu_text("##el_newline");

							if(CAT_gui_menu_item("REALLY ERASE LOGS"))
								CAT_gui_open_popup("This will permanently delete all your logged data. Are you sure you want to proceed?\n", CAT_POPUP_STYLE_YES_NO);
							if(CAT_gui_consume_popup())
							{
								CAT_erase_logs();
								CAT_gui_open_popup("All logs erased!\n", CAT_POPUP_STYLE_OK);
							}

							CAT_gui_end_menu();
						}

						CAT_gui_end_menu();
					}
				}
				
				CAT_gui_end_menu();
			}
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		{
		}
		break;
	}
}