#include "cat_monitors.h"

#include "cat_core.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_aqi.h"
#include "cat_input.h"
#include <math.h>
#include "cat_monitor_graphics_utils.h"
#include <stdio.h>
#include <time.h>
#include "cat_monitor_graph.h"

enum
{
	GATE,
	CALENDAR,
	GRAPH
};
static int page = GATE;
static float focus_progress = 0;

static enum
{
	CELLS,
	DATE
};
static int section = CELLS;

enum
{
	YEAR,
	MONTH,
	DAY
};

static CAT_datetime earliest;
static CAT_datetime today;
static CAT_datetime target;

#define DATE_Y 48
#define DATE_X 120

#define GRID_Y (DATE_Y + 48)
#define GRID_COLS 7
#define GRID_MARGIN 8
#define GRID_SPACING 4
#define GRID_X GRID_MARGIN
#define GRID_CELL_R (((CAT_LCD_SCREEN_W - (GRID_MARGIN * 2) - (GRID_SPACING * (GRID_COLS-1))) / GRID_COLS) / 2)

#define CELL_GREY RGB8882565(164, 164, 164)
#define CELL_PLACEHOLDER RGB8882565(64, 64, 64)    // outside this month (faint ring, no number)
#define CELL_DISABLED    RGB8882565(120, 120, 120) // in-month but outside [earliest..today]


bool is_leap_year(int year)
{
	return (year % 4) || ((year % 100 == 0) && (year % 400)) ? 0 : 1;
}

int days_in_month(int year, int month)
{
	return month == 2 ? (28 + is_leap_year(year)) : 31 - (month-1) % 7 % 2;
}

static int weekday_sun0(int year, int month, int day)
{
    // Sakamoto's algorithm (Gregorian)
    static const int t[] = {0,3,2,5,0,3,5,1,4,6,2,4};
    year -= month < 3; // make Jan/Feb part of previous year for leap-day handling
    int w = (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7;
    if (w < 0) w += 7;
    return w;
}

static int clamp_date_part(int phase, int year, int month, int day)
{
	switch(phase)
	{
		case YEAR:
		{
			return clamp(year, earliest.year, today.year);
		}

		case MONTH:
		{
			int min_month = 1;
			int max_month = 12;
			if(year == earliest.year)
				min_month = earliest.month;
			if(year == today.year)
				max_month = today.month;
			return clamp(month, min_month, max_month);
		}

		case DAY:
		{
			int days = days_in_month(year, month);
			int min_days = (year == earliest.year && month == earliest.month) ? earliest.day : 1;
			int max_days = (year == today.year && month == today.month) ? today.day : days;
			return clamp(day, min_days, max_days);
		}
	}
	
	return -1;
}

void calendar_logic()
{
	if(CAT_input_pressed(CAT_BUTTON_B))
		page = GATE;

	if(section == DATE)
	{
		if(CAT_input_pulse(CAT_BUTTON_LEFT))
			target.month -= 1;
		if(CAT_input_pulse(CAT_BUTTON_RIGHT))
			target.month += 1;
		if(target.month < 1)
		{
			target.year -= 1;
			target.month = 12;
		}
		if(target.month > 12)
		{
			target.year += 1;
			target.month = 1;
		}
		target.year = clamp_date_part(YEAR, target.year, target.month, target.day);
		target.month = clamp_date_part(MONTH, target.year, target.month, target.day);

		if(CAT_input_pressed(CAT_BUTTON_DOWN) || CAT_input_pressed(CAT_BUTTON_A))
			section = CELLS;
	}
	else
	{
		int was = target.day;
		int delta = 0;
		if(CAT_input_pulse(CAT_BUTTON_LEFT))
			delta += -1;
		if(CAT_input_pulse(CAT_BUTTON_RIGHT))
			delta += 1;
		if(CAT_input_pulse(CAT_BUTTON_UP))
		{
			if(was <= 7)
				section = DATE;
			else
				delta -= 7;
		}
		if(CAT_input_pulse(CAT_BUTTON_DOWN))
			delta += 7;
		target.day += delta;
		target.day = clamp_date_part(DAY, target.year, target.month, target.day);

		if(CAT_input_pressed(CAT_BUTTON_A))
		{
			if(target.day == was)
			{
				page = GRAPH;
				CAT_monitor_graph_enter(target);
			}
		}
	}
}

void render_calendar()
{
	CAT_set_text_scale(2);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
	CAT_draw_textf(DATE_X, DATE_Y, "%.2d/%.2d/%.4d", target.month, target.day, target.year);
	int date_width = strlen("##/##/####") * CAT_GLYPH_WIDTH * 2;
	CAT_draw_arrows(DATE_X, DATE_Y + CAT_GLYPH_HEIGHT, CAT_GLYPH_HEIGHT, date_width + 12, CAT_WHITE);
	if(section == DATE)
		CAT_draw_arrows(DATE_X, DATE_Y + CAT_GLYPH_HEIGHT, CAT_GLYPH_HEIGHT, date_width + 20, CAT_WHITE);

	int day = 1;
	for(int row = 0; row < 5; row++)
	{
		int y = GRID_Y + ((GRID_CELL_R * 2) + GRID_SPACING) * row;
		int cols = row == 4 ? 3 : 7;

		for(int col = 0; col < cols; col++)
		{
			int x = GRID_X + ((GRID_CELL_R * 2) + GRID_SPACING) * col;

			CAT_datetime date = target;
			date.day = day;
			if
			(
				CAT_datecmp(&date, &earliest) >= 0 &&
				CAT_datecmp(&date, &today) <= 0 &&
				day <= days_in_month(target.year, target.month)
			)
			{
				uint16_t colour = section == CELLS ? CAT_WHITE : CELL_GREY;
				if(day == target.day)
				{
					CAT_discberry(x + GRID_CELL_R, y + GRID_CELL_R, GRID_CELL_R, colour);
					CAT_circberry(x + GRID_CELL_R, y + GRID_CELL_R, GRID_CELL_R, colour);
					center_textf(x + GRID_CELL_R, y + GRID_CELL_R, 1, CAT_BLACK, "%d", day);
				}
				else
				{	
					CAT_circberry(x + GRID_CELL_R, y + GRID_CELL_R, GRID_CELL_R, colour);
					center_textf(x + GRID_CELL_R, y + GRID_CELL_R, 1, colour, "%d", day);
				}
			}
			else
			{
				CAT_circberry(x + GRID_CELL_R, y + GRID_CELL_R, GRID_CELL_R, CELL_GREY);
			}
			day += 1;
		}
	}
}

void gate_logic()
{
	if(CAT_input_dismissal())
		CAT_monitor_dismiss();
	if(CAT_input_pressed(CAT_BUTTON_LEFT))
		CAT_monitor_retreat();
	if(CAT_input_pressed(CAT_BUTTON_RIGHT))
		CAT_monitor_advance();

	if(!CAT_AQ_logs_initialized())
		return;

	if(CAT_input_held(CAT_BUTTON_A, 0))
		focus_progress += CAT_get_delta_time_s();
	if(CAT_input_released(CAT_BUTTON_A))
		focus_progress = 0;
	focus_progress = clamp(focus_progress, 0, 1);
	if(focus_progress >= 1)
	{
		focus_progress = 0;
		page = CALENDAR;
		section = CELLS;
	}
}

void render_gate()
{
	int cursor_y = center_textf(120, 60, 2, CAT_WHITE, "Calendar");
	cursor_y = underline(120, cursor_y, 2, CAT_WHITE, "Calendar");

	if(!CAT_AQ_logs_initialized())
	{
		center_textf(120, 160, CAT_input_held(CAT_BUTTON_A, 0) ? 3 : 2, CAT_WHITE, "No Logs");
	}
	else
	{
		CAT_annulusberry(120, 200, 64, 56, CAT_WHITE, CAT_ease_inout_sine(focus_progress), 0.25);
		CAT_circberry(120, 200, 56, CAT_WHITE);
		CAT_circberry(120, 200, 64, CAT_WHITE);

		CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
		CAT_set_text_colour(CAT_WHITE);
		CAT_set_text_scale(3);
		CAT_draw_text(120, 200-18, "A");
	}		
}

void CAT_monitor_MS_calendar(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_log_cell first;
			CAT_read_first_calendar_cell(&first);
			CAT_make_datetime(first.timestamp, &earliest);

			CAT_get_datetime(&today);
			target = today;

			page = GATE;
			section = CELLS;
		}
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			switch (page)
			{
				case GATE:
					gate_logic();
				break;

				case CALENDAR:
					calendar_logic();
				break;

				case GRAPH:
					CAT_monitor_graph_logic();
					if(CAT_monitor_graph_should_exit())
						page = CALENDAR;
				break;
			}
		}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
			focus_progress = 0;
		break;
	}
}
void CAT_monitor_render_calendar()
{	
	switch (page)
	{
		case GATE:
			render_gate();
		break;

		case CALENDAR:
			render_calendar();
		break;

		case GRAPH:
			CAT_monitor_graph_render();
		break;
	}
}