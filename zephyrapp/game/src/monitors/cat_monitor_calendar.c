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

#define DATE_Y 60
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

static int get_min_year() { return earliest.year; }
static int get_max_year() { return today.year; }
static int get_min_month(int year) { return year == earliest.year ? earliest.month : 1; }
static int get_max_month(int year) { return year == today.year ? today.month : 1; }
static int get_min_day(int year, int month) { return (year == earliest.year && month == earliest.month) ? earliest.day : 1; }
static int get_max_day(int year, int month) { return (year == today.year && month == today.month) ? today.day : days_in_month(year, month); }

static int clamp_date_part(int phase, int year, int month, int day)
{
	switch(phase)
	{
		case YEAR: return clamp(year, get_min_year(), get_max_year());
		case MONTH: return clamp(month, get_min_month(year), get_max_month(year));
		case DAY: return clamp(day, get_min_day(year, month), get_max_day(year, month));
	}
	return -1;
}

void calendar_logic()
{
    if(section == DATE)
    {
		if
		(
			(CAT_input_pressed(CAT_BUTTON_LEFT) && target.month == get_min_month(target.year)) ||
			(CAT_input_pressed(CAT_BUTTON_RIGHT) && target.month == get_max_month(target.year)) ||
			CAT_input_pressed(CAT_BUTTON_A) || CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_DOWN)
		)
		{
			section = CELLS;
			return;
		}

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

        // keep all three parts valid after month/year changes
        target.year  = clamp_date_part(YEAR,  target.year, target.month, target.day);
        target.month = clamp_date_part(MONTH, target.year, target.month, target.day);
        target.day   = clamp_date_part(DAY,   target.year, target.month, target.day);    
    }
    else // --- CELLS ---
    {
		if(CAT_input_pressed(CAT_BUTTON_B))
        	page = GATE;

        int was = target.day;
        int delta = 0;

        int dim       = days_in_month(target.year, target.month);
        int first_dow = weekday_sun0(target.year, target.month, 1); // 0=Sun..6=Sat

        // compute selectable range for this month
        int min_day = (target.year == earliest.year && target.month == earliest.month) ? earliest.day : 1;
        int max_day = (target.year == today.year    && target.month == today.month)    ? today.day    : dim;

        // horizontal moves (will be clamped after)
        if(CAT_input_pulse(CAT_BUTTON_LEFT))  delta -= 1;
        if(CAT_input_pulse(CAT_BUTTON_RIGHT)) delta += 1;
		if(CAT_input_pulse(CAT_BUTTON_UP)) delta -= 7;
		if(CAT_input_pulse(CAT_BUTTON_DOWN)) delta += 7;

		int min_delta = min_day - target.day;
		int max_delta = max_day - target.day;
		
		if(delta < min_delta || delta > max_delta)
		{
			if((target.day == min_day && target.month != get_min_month(target.year)) || (target.day == max_day && target.month != get_max_month(target.year)))
			{
				section = DATE;
				return;
			}
			delta = clamp(delta, min_delta, max_delta);
		}

        // apply movement and clamp to the valid [min_day..max_day] for this month
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
    /* ---- declarations up front (C89‑friendly) ---- */
    int date_width;
    static const char* DOW[7] = {"S","M","T","W","T","F","S"};
    int header_y;
    int first_dow, dim, total, rows;
    int row, col;
    int x, y, cell, day;
    CAT_datetime date;
    uint16_t colour;
    bool in_logs_window;

    /* ---- date line + arrows ---- */
    CAT_set_text_scale(2);
    CAT_set_text_colour(CAT_WHITE);
    CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
    CAT_draw_textf(DATE_X, DATE_Y, "%.2d/%.2d/%.4d", target.month, target.day, target.year);

    date_width = (int) strlen("##/##/####") * CAT_GLYPH_WIDTH * 2;
    CAT_draw_arrows(DATE_X, DATE_Y + CAT_GLYPH_HEIGHT, CAT_GLYPH_HEIGHT, date_width + 12, CAT_WHITE);
    if (section == DATE)
	{
        CAT_draw_arrows(DATE_X, DATE_Y + CAT_GLYPH_HEIGHT, CAT_GLYPH_HEIGHT, date_width + 20, CAT_WHITE);
		center_textf(120, DATE_Y - CAT_TEXT_LINE_HEIGHT, 1, CAT_WHITE, "<< CHANGE MONTH >>\n");
	}

    /* ---- day-of-week header (S M T W T F S) ---- */
    header_y = GRID_Y - (CAT_GLYPH_HEIGHT / 2 ) - 4;
    for (col = 0; col < GRID_COLS; ++col)
    {
        int hx = GRID_X + ((GRID_CELL_R * 2) + GRID_SPACING) * col + GRID_CELL_R;
        center_textf(hx, header_y, 1, CAT_WHITE, "%s", DOW[col]);
    }

    /* ---- calendar math (Sun=0..Sat=6) ---- */
    first_dow = weekday_sun0(target.year, target.month, 1);
    dim       = days_in_month(target.year, target.month);
    total     = first_dow + dim;
    rows      = (total + GRID_COLS - 1) / GRID_COLS; /* 4..6 rows */

    /* ---- draw cells ---- */
    for (row = 0; row < rows; ++row)
    {
        y = GRID_Y + ((GRID_CELL_R * 2) + GRID_SPACING) * row;

        for (col = 0; col < GRID_COLS; ++col)
        {
            x    = GRID_X + ((GRID_CELL_R * 2) + GRID_SPACING) * col;
            cell = row * GRID_COLS + col;
            day  = cell - first_dow + 1; /* 1..dim for in-month days */

            if (day >= 1 && day <= dim)
            {
                date = target;
                date.day = day;

                in_logs_window =
                    (CAT_datecmp(&date, &earliest) >= 0) &&
                    (CAT_datecmp(&date, &today)    <= 0);

                if (in_logs_window)
                {
                    colour = (section == CELLS) ? CAT_WHITE : CELL_GREY;

                    if (day == target.day)
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
                    /* in this month, but outside [earliest..today] */
                    CAT_circberry(x + GRID_CELL_R, y + GRID_CELL_R, GRID_CELL_R, CELL_DISABLED);
                    center_textf(x + GRID_CELL_R, y + GRID_CELL_R, 1, CELL_DISABLED, "%d", day);
                }
            }
            else
            {
                /* leading/trailing placeholders (outside this month) */
                CAT_circberry(x + GRID_CELL_R, y + GRID_CELL_R, GRID_CELL_R, CELL_PLACEHOLDER);
            }
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