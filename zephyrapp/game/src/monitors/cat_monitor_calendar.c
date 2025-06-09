#include "cat_monitors.h"

#include "cat_core.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_aqi.h"
#include "cat_input.h"

#define SELECTOR_X 8
#define SELECTOR_Y 148

bool focused = false;

enum
{
	YEAR,
	MONTH,
	DAY,
	FINISHED
};

static int date_parts[3];
static int selection_phase = YEAR;

static void draw_selector()
{
	int cursor_y = SELECTOR_Y;
	int cursor_x = SELECTOR_X;

	for(int i = 0; i < FINISHED; i++)
	{
		bool delim = i < DAY;
		const char* fmt = delim ? "%.4i/" : "%.4i";
		int len = delim ? 5 : 4;
		int width = len * CAT_GLYPH_WIDTH * 2;

		CAT_set_text_colour(CAT_WHITE);
		CAT_set_text_scale(2);
		CAT_draw_textf(cursor_x, cursor_y, fmt, date_parts[i]);

		if(focused && selection_phase == i)
		{
			int sub_x = cursor_x + width / 2 - (5 * CAT_GLYPH_WIDTH) / 2;
			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(sub_x, cursor_y-16, "%.4i ", date_parts[i]-1);
			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(sub_x, cursor_y+28, "%.4i ", date_parts[i]+1);
		}

		cursor_x += len * CAT_GLYPH_WIDTH * 2;
	}
}

void CAT_monitor_render_calendar()
{
	if(selection_phase < FINISHED)
		draw_selector();
}

static int clamp_date_part(int phase, int part)
{
	switch(phase)
	{
		case YEAR:
			return clamp(part, 2020, 2030);

		case MONTH:
			return clamp(part, 1, 12);

		case DAY:
		{
			bool is_leap_year = (date_parts[YEAR] % 4) || ((date_parts[YEAR] % 100 == 0) && (date_parts[YEAR] % 400)) ? 0 : 1;
			int days = date_parts[MONTH] == 2 ? (28 + is_leap_year) : 31 - (date_parts[MONTH]-1) % 7 % 2;
			return clamp(part, 1, days);
		}

		default:
			return 0;
	}
}

void CAT_monitor_MS_calendar(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			focused = false;

			date_parts[0] = 2025;
			date_parts[1] = 6;
			date_parts[2] = 9;

			selection_phase = YEAR;
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(!focused)
			{
				if(CAT_input_released(CAT_BUTTON_START))
					CAT_monitor_exit();
				if(CAT_input_pressed(CAT_BUTTON_LEFT))
					CAT_monitor_retreat();
				if(CAT_input_pressed(CAT_BUTTON_RIGHT))
					CAT_monitor_advance();

				if(CAT_input_pressed(CAT_BUTTON_A))
					focused = true;
			}
			else
			{
				if(selection_phase != FINISHED)
				{
					if(CAT_input_pressed(CAT_BUTTON_B))
						focused = false;

					if(CAT_input_pressed(CAT_BUTTON_LEFT))
						selection_phase -= 1;
					if(CAT_input_pressed(CAT_BUTTON_RIGHT) || CAT_input_pressed(CAT_BUTTON_A))
					{
						selection_phase += 1;
						if(CAT_input_pressed(CAT_BUTTON_A) && selection_phase == FINISHED)
							break;
					}
					selection_phase = (selection_phase + FINISHED) % FINISHED;
				
					if(CAT_input_pulse(CAT_BUTTON_UP))
						date_parts[selection_phase] -= 1;
					if(CAT_input_pulse(CAT_BUTTON_DOWN))
						date_parts[selection_phase] += 1;
					for(int i = 0; i < FINISHED; i++)
						date_parts[i] = clamp_date_part(i, date_parts[i]);
				}
				else
				{
					if(CAT_input_pressed(CAT_BUTTON_B))
						selection_phase = DAY;
				}
			}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}