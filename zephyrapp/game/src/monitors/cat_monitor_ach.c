#include "cat_monitor_ach.h"

#include <stdint.h>
#include "cat_core.h"
#include <math.h>
#include "cat_math.h"
#include "cat_monitor_graphics_utils.h"
#include "cat_monitor_graph.h"
#include "cat_input.h"
#include "cat_monitor_utils.h"
#include "cat_time.h"
#include "cat_monitor_calendar.h"

static int view;
static int16_t* values;
static uint64_t* timestamps;
static int32_t* indices;

static int first_idx;
static int last_idx;
static int start;
static int end;

void CAT_monitor_ACH_set_view(int _view)
{
	view = _view;
	if(view != CAT_MONITOR_GRAPH_VIEW_CO2 && view != CAT_MONITOR_GRAPH_VIEW_PN_10_0)
		view = CAT_MONITOR_GRAPH_VIEW_CO2;
}

void CAT_monitor_ACH_set_data(int16_t* _values, uint64_t* _timestamps, int32_t* _indices, int _first_idx, int _last_idx)
{
	values = _values;
	timestamps = _timestamps;
	indices = _indices;

	first_idx = _first_idx;
	last_idx = _last_idx;
	start = _first_idx;
	end = _last_idx;
}

void find_range(int* min_idx_out, int* max_idx_out)
{
	int min_value = INT16_MAX;
	int min_idx = first_idx;
	int max_value = INT16_MIN;
	int max_idx = last_idx;
	for(int i = first_idx; i < last_idx; i++)
	{
		if(indices[i] == -1)
			continue;

		int16_t value = values[i];
		if(value < min_value)
		{
			min_value = value;
			min_idx = i;
		}
		if(value > max_value)
		{
			max_value = value;
			max_idx = i;
		}
	}
	*min_idx_out = min_idx;
	*max_idx_out = max_idx;
}

float find_stddev()
{
	float mean = 0;
	for(int i = first_idx; i < last_idx; i++)
		mean += values[i];
	mean /= last_idx;

	float sum = 0;
	for(int i = 0; i < last_idx; i++)
		sum += (values[i] - mean) * (values[i] - mean);
	sum /= (last_idx-1);

	return sqrtf(sum);
}

int find_peak_before_trough(int trough_idx, unsigned int dip_threshold)
{
	int16_t trough_value = values[trough_idx];
	int16_t peak_idx = trough_idx;
	int16_t peak_value = values[peak_idx];
	
	for(int i = trough_idx-1; i >= first_idx; i--)
	{
		if(indices[i] == -1 || indices[i+1] == -1)
			continue;

		int16_t value = values[i];
		int diff = value - values[i+1];

		if(diff >= 0)
		{
			if(value > peak_value)
			{
				peak_value = value;
				peak_idx = i;
			}
		}
		else
		{
			if(abs(diff) > dip_threshold)
				break;
		}
	}

	return peak_idx;
}

int find_trough_after_peak(int peak_idx, float spike_threshold)
{
	int16_t peak_value = values[peak_idx];
	int16_t trough_idx = peak_idx;
	int16_t trough_value = values[trough_idx];

	for(int i = peak_idx+1; i < last_idx; i++)
	{
		if(indices[i] == -1 || indices[i+1] == -1)
			continue;

		int16_t value = values[i];
		int diff = values[i+1] - value;

		if(diff <= 0)
		{
			if(value < trough_value)
			{
				trough_value = value;
				trough_idx = i;
			}
		}
		else
		{
			if(abs(diff) > spike_threshold)
				break;
		}
	}

	return trough_idx;
}

void CAT_monitor_ACH_auto_cursors()
{
	int min_idx, max_idx;
	find_range(&min_idx, &max_idx);
	int16_t range = values[max_idx] - values[min_idx];
	float thresh = find_stddev();
	if(thresh > range/4)
		thresh = range/6;
	else if(thresh < range/8)
		thresh *= 2;

	if(view == CAT_MONITOR_GRAPH_VIEW_CO2)
	{	
		start = find_peak_before_trough(min_idx, thresh);
		end = min_idx;
	}
	else
	{
		start = max_idx;
		end = find_trough_after_peak(max_idx, thresh);
	}
}

void CAT_monitor_ACH_set_cursors(int _start, int _end)
{
	start = clamp(_start, first_idx, last_idx);
	end = clamp(_end, start, last_idx);
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

	int16_t max_ppm = values[start];
	int16_t min_ppm = values[end];
	if(min_ppm >= max_ppm)
		return -1;
	
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
	MODE_GATE,
	MODE_DATE_SELECT,
	MODE_INIT,
	MODE_START,
	MODE_END,
	MODE_INVALID,
	MODE_OUTCOME,
};
static int mode = MODE_OUTCOME;

static CAT_datetime target;
static int target_part = CAT_DATE_PART_DAY;

static float ach[2] = {-1, -1};

static bool should_reload;
static bool should_fast_forward = false;

void CAT_monitor_ACH_enter(CAT_datetime date)
{
	target = date;
	should_fast_forward = true;
	CAT_monitor_seek(CAT_MONITOR_PAGE_ACH);
}

void go_to_gate()
{
	CAT_monitor_gate_lock();
	mode = MODE_GATE;
}

void load_view(int _view)
{
	view = _view;
	should_reload = true;
	mode = MODE_INIT;
}

void quit()
{
	if(should_fast_forward)
		CAT_monitor_calendar_enter(target);	
	else
		go_to_gate();
}

void back()
{
	if(mode == MODE_DATE_SELECT)
		quit();
	else if(mode == MODE_START || mode == MODE_END)
	{
		if(view == CAT_MONITOR_GRAPH_VIEW_CO2)
			quit();
		else
			load_view(CAT_MONITOR_GRAPH_VIEW_CO2);
	}
	else if(mode == MODE_OUTCOME)
	{
		load_view(CAT_MONITOR_GRAPH_VIEW_PN_10_0);
	}
}

int move_cursor(int cursor, int l, int r)
{
	int dx = 0;
	if(CAT_input_held(CAT_BUTTON_LEFT, 0))
		dx -= 1;
	if(CAT_input_held(CAT_BUTTON_LEFT, 1))
		dx -= 3;
	if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
		dx += 1;
	if(CAT_input_held(CAT_BUTTON_RIGHT, 1))
		dx += 3;
	return clamp(cursor+dx, l, r);
}

void CAT_monitor_MS_ACH(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_monitor_gate_init("ACH Viewer");

			if(should_fast_forward)
			{
				load_view(CAT_MONITOR_GRAPH_VIEW_CO2);
			}
			else
			{
				CAT_get_datetime(&target);
				mode = MODE_GATE;
				view = CAT_MONITOR_GRAPH_VIEW_PN_10_0;
			}
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			CAT_monitor_graph_tick();

			switch(mode)
			{
				case MODE_GATE:
				{
					CAT_monitor_gate_logic();
					if(!CAT_monitor_gate_is_locked())
					{
						target_part = CAT_DATE_PART_DAY;
						mode = MODE_DATE_SELECT;
					}
				}
				break;

				case MODE_DATE_SELECT:
				{
					if(CAT_input_pressed(CAT_BUTTON_LEFT))
						target_part -= 1;
					if(CAT_input_pressed(CAT_BUTTON_RIGHT))
						target_part += 1;
					target_part = wrap(target_part, 3);

					if(CAT_input_pressed(CAT_BUTTON_UP))
						target.data[target_part] -= 1;
					if(CAT_input_pressed(CAT_BUTTON_DOWN))
						target.data[target_part] += 1;
					target.data[CAT_DATE_PART_YEAR] = CAT_clamp_date_part(CAT_DATE_PART_YEAR, target.year, target.month, target.day);
					target.data[CAT_DATE_PART_MONTH] = CAT_clamp_date_part(CAT_DATE_PART_MONTH, target.year, target.month, target.day);
					target.data[CAT_DATE_PART_DAY] = CAT_clamp_date_part(CAT_DATE_PART_DAY, target.year, target.month, target.day);

					if(CAT_input_pressed(CAT_BUTTON_B))
						back();
					if(CAT_input_pressed(CAT_BUTTON_A))
						load_view(CAT_MONITOR_GRAPH_VIEW_CO2);
				}
				break;

				case MODE_INIT:
				{
					if(should_reload)
					{	
						CAT_monitor_graph_set_view(view);
						CAT_monitor_graph_set_sample_count(WINDOW_W);
						CAT_monitor_graph_load_date(target);
						CAT_monitor_graph_set_scale(1);
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
							CAT_monitor_graph_get_first_idx(),
							CAT_monitor_graph_get_last_idx()
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
					start = move_cursor(start, first_idx, last_idx);
					CAT_monitor_ACH_set_cursors(start, end);

					if(CAT_input_pressed(CAT_BUTTON_A))
					{
						if(indices[start] != -1)
							mode = MODE_END;
					}
					if(CAT_input_pressed(CAT_BUTTON_B))
					{
						back();
					}
				}
				break;

				case MODE_END:
				{
					end = move_cursor(end, start+1, last_idx);
					CAT_monitor_ACH_set_cursors(start, end);

					if(CAT_input_pressed(CAT_BUTTON_A))
					{
						if(indices[end] != -1)
						{
							ach[view] = CAT_monitor_ACH_calculate();
							if(view == CAT_MONITOR_GRAPH_VIEW_CO2)
								load_view(CAT_MONITOR_GRAPH_VIEW_PN_10_0);
							else
								mode = MODE_OUTCOME;
						}
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
						quit();		
				}
				case MODE_OUTCOME:
				{
					if(CAT_input_pressed(CAT_BUTTON_B))
						back();
					if(CAT_input_pressed(CAT_BUTTON_A))
						quit();				
				}
				break;
			}
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		{
			should_fast_forward = false;
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
			snprintf(buf, sizeof(buf), "" CAT_FLOAT_FMT " #/cm\5\n", CAT_FMT_FLOAT(value / 100.0f));
		break;
	}
	return buf;
}

void CAT_monitor_render_ACH()
{	
	if(mode == MODE_GATE)
	{
		CAT_monitor_gate_render();
	}
	else if(mode == MODE_DATE_SELECT)
	{
		const char* format = "%.2d %.2d %.4d";
		int width = strlen(format) * CAT_GLYPH_WIDTH * 2;
		int cursor_y = center_textf(120, 80, 2, CAT_WHITE, " Select Date:") + 32;
		cursor_y = center_textf(120, cursor_y, 2, CAT_WHITE, format, target.month, target.day, target.year);
		
		int under_offset =
		target_part == CAT_DATE_PART_YEAR ? CAT_GLYPH_WIDTH*2*6 :
		target_part == CAT_DATE_PART_MONTH ? 0 :
		CAT_GLYPH_WIDTH*2*3;
		int under_x = 120-width/2 + CAT_GLYPH_WIDTH*2*2 + under_offset; 
		int under_width = 
		target_part == CAT_DATE_PART_YEAR ? CAT_GLYPH_WIDTH*2*4 :
		CAT_GLYPH_WIDTH*2*2;
		
		CAT_lineberry(under_x, cursor_y-4, under_x + under_width, cursor_y-4, CAT_WHITE);
	}
	else if(mode == MODE_OUTCOME)
	{
		int cursor_y = center_textf(120, 60, 2, CAT_WHITE, "ACH Viewer");
		cursor_y = underline(120, cursor_y, 2, CAT_WHITE, "ACH Viewer") + 20;

		if(ach[0] != -1)
		{
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf(120, cursor_y, "ACH: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(ach[0]));
		}
		if(ach[1] != -1)
		{
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf(120, cursor_y, "eACH: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(ach[1]));
		}
		if(ach[0] != -1 && ach[1] != -1)
		{
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf(120, cursor_y, "Total: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(ach[0] + ach[1]));
		}

		cursor_y = center_textf(120, cursor_y + 20, 1, CAT_WHITE, "Press [A] to return\n");
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
		CAT_draw_subpage_markers(32, 2, view, CAT_WHITE);

		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(12, WINDOW_Y0-14, "%s", get_title_string(view));
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(WINDOW_X1 - 10 * CAT_GLYPH_WIDTH, WINDOW_Y0-14, "%.2d/%.2d/%.4d", target.month, target.day, target.year);

		CAT_monitor_graph_draw(WINDOW_X0, WINDOW_Y0, WINDOW_Y1-WINDOW_Y0);
		if(CAT_monitor_graph_did_load_succeed())
		{
			int cursor = mode == MODE_START ? start : end;

			CAT_monitor_graph_draw_cursor(start, mode == MODE_START ? CAT_GREEN : CAT_GREY);
			CAT_monitor_graph_draw_cursor(end, mode == MODE_END ? CAT_RED : CAT_GREY);

			int32_t index = indices[cursor];
			int16_t value = values[cursor];
			CAT_datetime date;
			CAT_make_datetime(timestamps[cursor], &date);
			
			CAT_set_text_scale(2);
			CAT_set_text_colour(CAT_WHITE);
			int cursor_y = CAT_draw_textf(WINDOW_X0, WINDOW_Y1+4, index != -1 ? make_value_string(view, value) : "INVALID\n", value);
			CAT_set_text_colour(CAT_WHITE);
			cursor_y = CAT_draw_textf(WINDOW_X0, cursor_y, "at %.2d:%.2d\n", date.hour, date.minute) + 4;

			if(index != -1)
			{
				CAT_set_text_colour(CAT_WHITE);
				cursor_y = CAT_draw_textf(WINDOW_X0, cursor_y, "Press [A] to confirm %s\n", mode == MODE_START ? "start" : "end");
			}
			CAT_set_text_colour(CAT_WHITE);
			cursor_y = CAT_draw_textf(WINDOW_X0, cursor_y, "Press[B] to go back\n");
		}
	}
	else if(mode == MODE_OUTCOME)
	{
		int cursor_y = center_textf(120, 60, 2, CAT_WHITE, "ACH Viewer");
		cursor_y = underline(120, cursor_y, 2, CAT_WHITE, "ACH Viewer") + 20;

		if(ach[0] != -1)
		{
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf(120, cursor_y, "ACH: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(ach[0]));
		}
		if(ach[1] != -1)
		{
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf(120, cursor_y, "eACH: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(ach[1]));
		}
		if(ach[0] != -1 && ach[1] != -1)
		{
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf(120, cursor_y, "Total: " CAT_FLOAT_FMT "\n", CAT_FMT_FLOAT(ach[0] + ach[1]));
		}

		cursor_y = center_textf(120, cursor_y + 20, 1, CAT_WHITE, "Press [A] to calculate\n");
	}
}
