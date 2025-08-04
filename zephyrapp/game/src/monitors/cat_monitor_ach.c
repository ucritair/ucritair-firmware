#include "cat_monitor_ach.h"

#include <stdint.h>
#include "cat_core.h"
#include <math.h>
#include "cat_math.h"
#include "cat_monitor_graphics_utils.h"
#include "cat_monitor_graph.h"
#include "cat_input.h"

static int view;
static int16_t* values;
static uint64_t* timestamps;
static int32_t* indices;

static int start;
static int end;
static int extent;

void CAT_monitor_ACH_set_view(int _view)
{
	view = _view;
	if(view != CAT_MONITOR_GRAPH_VIEW_CO2 && view != CAT_MONITOR_GRAPH_VIEW_PN_10_0)
		view = CAT_MONITOR_GRAPH_VIEW_CO2;
}

void CAT_monitor_ACH_set_data(int16_t* _values, uint64_t* _timestamps, int32_t* _indices, int _extent)
{
	values = _values;
	timestamps = _timestamps;
	indices = _indices;

	start = 0;
	end = _extent;
	extent = _extent;
}

void CAT_monitor_ACH_auto_cursors()
{
	int16_t peak = INT16_MIN;
	int peak_idx = -1;
	for(int i = 0; i < extent; i++)
	{
		if(indices[i] == -1)
			continue;

		int16_t value = values[i];
		if(value > peak)
		{
			peak = value;
			peak_idx = i;
		}
	}
	start = peak_idx;

	int trough_idx = peak_idx;
	int16_t trough = values[trough_idx];
	for(int i = trough_idx; i < extent; i++)
	{
		if(indices[i] == -1)
			continue;

		int16_t value = values[i];
		if(value < trough)
		{
			trough = value;
			trough_idx = i;
		}
	}
	end = trough_idx;
}

void CAT_monitor_ACH_set_cursors(int _start, int _end)
{
	start = clamp(_start, 0, extent);
	end = clamp(_end, start, extent);
}

void CAT_monitor_ACH_get_cursors(int* _start, int* _end)
{
	*_start = start;
	*_end = end;
}

float CAT_monitor_ACH_calculate()
{
	if(start == end)
		return -1;
	if(values[start] == values[end])
		return -1;

	int16_t max_ppm = values[start];
	int16_t min_ppm = values[end];
	float decay_concentration = min_ppm + (float) (max_ppm - min_ppm) / (float) M_E;
	
    // Decay time (Assume for simplicity it's between max and baseline times)
   // <A FUNCTION THAT GIVES ME THE TIME that the concentration dropped below decay_concentration and returns decay_time)

	uint64_t decay_time = 0;
	for (int cursor = start; cursor <= end; cursor++)
	{
		if (indices[cursor] == -1)
			continue;

		if(values[cursor] < decay_concentration)
		{
			decay_time = timestamps[cursor];
			break;
		}
	}
	if(decay_time == 0)
		return -1;

    // Calculate ACH
    float ach = 1.0 / ((float) (decay_time - timestamps[start]) / 3600.0);  // Convert to hours
	return ach;
}

#define MARGIN 12
#define WINDOW_X0 MARGIN
#define WINDOW_Y0 64
#define WINDOW_H 128
#define WINDOW_W (CAT_LCD_SCREEN_W - 2 * MARGIN)
#define WINDOW_X1 (WINDOW_X0 + WINDOW_W - 1)
#define WINDOW_Y1 (WINDOW_Y0 + WINDOW_H - 1)

enum mode
{
	MODE_INIT,
	MODE_START,
	MODE_END,
	MODE_INVALID,
	MODE_OUTCOME,
};
static int mode = MODE_OUTCOME;
static CAT_datetime today;
static float ach[2] = {-1, -1};
static bool should_reload;

void switch_and_reload()
{
	view = view == CAT_MONITOR_GRAPH_VIEW_CO2 ? CAT_MONITOR_GRAPH_VIEW_PN_10_0 : CAT_MONITOR_GRAPH_VIEW_CO2;
	should_reload = true;
	mode = MODE_INIT;
}

void CAT_monitor_MS_ACH(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			view = CAT_MONITOR_GRAPH_VIEW_PN_10_0;
			CAT_get_datetime(&today);
		}
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_monitor_graph_tick();

			switch(mode)
			{
				case MODE_INIT:
				{
					if(should_reload)
					{	
						CAT_monitor_graph_set_view(view);
						CAT_monitor_graph_set_sample_count(WINDOW_W);
						CAT_monitor_graph_load_date(today);
						should_reload = false;
						break;
					}

					if(CAT_monitor_graph_did_load_succeed())
					{
						CAT_monitor_ACH_set_data
						(
							CAT_monitor_graph_get_values(),
							CAT_monitor_graph_get_timestamps(),
							CAT_monitor_graph_get_indices(),
							CAT_monitor_graph_get_extent()
						);
						CAT_monitor_ACH_auto_cursors();
						mode = MODE_START;
					}
					else
						mode = MODE_INVALID;
				}
				break;

				case MODE_START:
				{
					if(CAT_input_pulse(CAT_BUTTON_LEFT))
						start -= 1;
					if(CAT_input_pulse(CAT_BUTTON_RIGHT))
						start += 1;
					CAT_monitor_ACH_set_cursors(start, end);

					if(CAT_input_pressed(CAT_BUTTON_A))
						mode = MODE_END;
					if(CAT_input_pressed(CAT_BUTTON_B))
					{
						if(view == CAT_MONITOR_GRAPH_VIEW_PN_10_0)
							switch_and_reload();
						else
							mode = MODE_OUTCOME;
					}
				}
				break;

				case MODE_END:
				{
					if(CAT_input_pulse(CAT_BUTTON_LEFT))
						end -= 1;
					if(CAT_input_pulse(CAT_BUTTON_RIGHT))
						end += 1;
					CAT_monitor_ACH_set_cursors(start, end);

					if(CAT_input_pressed(CAT_BUTTON_A))
					{
						ach[view] = CAT_monitor_ACH_calculate();
						if(view == CAT_MONITOR_GRAPH_VIEW_CO2)
							switch_and_reload();
						else
							mode = MODE_OUTCOME;
					}
					if(CAT_input_pressed(CAT_BUTTON_B))
					{
						mode = MODE_START;
					}
				}
				break;

				case MODE_INVALID:
				{
					if(CAT_input_pressed(CAT_BUTTON_A) || CAT_input_pressed(CAT_BUTTON_B))
						mode = MODE_OUTCOME;
				}
				break;

				case MODE_OUTCOME:
				{
					if(CAT_input_dismissal())
						CAT_monitor_soft_exit();
					if(CAT_input_pressed(CAT_BUTTON_LEFT))
						CAT_monitor_retreat();
					if(CAT_input_pressed(CAT_BUTTON_RIGHT))
						CAT_monitor_advance();	

					if(CAT_input_pressed(CAT_BUTTON_A))
						switch_and_reload();
				}
				break;
			}
		}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		{

		}
		break;
	}
}

static const char* get_title_string(int view)
{
	if(view == CAT_MONITOR_GRAPH_VIEW_CO2)
		return "ACH (CO2)";
	if(view == CAT_MONITOR_GRAPH_VIEW_PN_10_0)
		return "eACH (PN10.0)";
	return "N/A";
}

static char* make_value_string(int view, int16_t value)
{
	static char buf[32];
	switch(view)
	{
		case CAT_MONITOR_GRAPH_VIEW_CO2:
			snprintf(buf, sizeof(buf), "%d ppm\n", value);
		break;
		case CAT_MONITOR_GRAPH_VIEW_PN_10_0:
			snprintf(buf, sizeof(buf), "%.1f #/cm\5\n", (float) value / 100.0f);
		break;
	}
	return buf;
}

void CAT_monitor_render_ACH()
{
	if(mode == MODE_OUTCOME)
	{
		int cursor_y = center_textf(120, 60, 2, CAT_WHITE, "ACH Viewer");
		cursor_y = underline(120, cursor_y, 2, CAT_WHITE, "ACH Viewer") + 20;

		if(ach[0] != -1)
		{
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf(120, cursor_y, "ACH: %.2f\n", ach[0]);
		}
		if(ach[1] != -1)
		{
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf(120, cursor_y, "eACH: %.2f\n", ach[1]);
		}
		if(ach[0] != -1 && ach[1] != -1)
		{
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf(120, cursor_y, "Total: %.2f\n", ach[0] + ach[1]);
		}

		cursor_y = center_textf(120, cursor_y + 20, 1, CAT_WHITE, "Press [A] to calculate\n");
	}
	else if(mode == MODE_INVALID)
	{
		CAT_set_text_colour(CAT_WHITE);
		CAT_set_text_scale(2);
		int cursor_y = CAT_draw_text(WINDOW_X0, 48, "Error\n");

		CAT_set_text_colour(CAT_WHITE);
		CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
		CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
		cursor_y = CAT_draw_text(WINDOW_X0, cursor_y, "\nInsufficient data available for this date.\nPress [A] or [B] to dismiss this page.");
	}
	else if(!(mode == MODE_INIT && should_reload))
	{
		draw_subpage_markers(32, 2, view);

		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(12, WINDOW_Y0-14, "%s", get_title_string(view));
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(WINDOW_X1 - 10 * CAT_GLYPH_WIDTH, WINDOW_Y0-14, "%.2d/%.2d/%.4d", today.month, today.day, today.year);

		CAT_monitor_graph_draw(WINDOW_X0, WINDOW_Y0, WINDOW_Y1-WINDOW_Y0);
		if(CAT_monitor_graph_did_load_succeed())
		{
			int cursor = mode == MODE_START ? start : end;

			CAT_monitor_graph_draw_cursor(start, mode == MODE_START ? CAT_GREEN : CAT_GREY);
			CAT_monitor_graph_draw_cursor(end, mode == MODE_END ? CAT_RED : CAT_GREY);

			int16_t value = values[cursor];
			CAT_datetime date;
			CAT_make_datetime(timestamps[cursor], &date);
			
			CAT_set_text_scale(2);
			CAT_set_text_colour(CAT_WHITE);
			int cursor_y = CAT_draw_textf(WINDOW_X0, WINDOW_Y1+4, value != -1 ? make_value_string(view, value) : "INVALID", value);
			CAT_set_text_colour(CAT_WHITE);
			cursor_y = CAT_draw_textf(WINDOW_X0, cursor_y, "at %.2d:%.2d\n", date.hour, date.minute) + 4;

			CAT_set_text_colour(CAT_WHITE);
			cursor_y = CAT_draw_textf(WINDOW_X0, cursor_y, "Press [A] to confirm %s\nPress[B] to go back", mode == MODE_START ? "start" : "end");
		}
	}
}
