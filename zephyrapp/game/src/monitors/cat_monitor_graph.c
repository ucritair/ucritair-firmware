#include "cat_monitor_graph.h"

#include "cat_input.h"
#include "cat_math.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_monitor_graphics_utils.h"
#include "cat_monitor_ach.h"
#include <inttypes.h>
#include <time.h>
#include "cat_time.h"

#define MAX_SAMPLE_COUNT CAT_LCD_SCREEN_W

typedef enum
{
	MODE_LOADING,
	MODE_VIEWING
} graph_mode;
static int mode = MODE_VIEWING;

static int view = CAT_MONITOR_GRAPH_VIEW_CO2;

static int sample_count;
static int sample_period;

static int16_t values[MAX_SAMPLE_COUNT];
static uint64_t timestamps[MAX_SAMPLE_COUNT];
static int32_t indices[MAX_SAMPLE_COUNT];
static int16_t value_min;
static uint16_t value_min_idx;
static int16_t value_max;
static uint16_t value_max_idx;
static int first_valid_idx = -1;
static int last_valid_idx = -1;

static CAT_datetime start_date = {0};
static CAT_datetime start_date_last = {0};
static int seek_bookmark = -1;
static int seek_direction = -1;

static int focus_idx;

void CAT_monitor_graph_set_view(int _view)
{
	view = CAT_wrap(_view, CAT_MONITOR_GRAPH_VIEW_MAX);
}

void CAT_monitor_graph_set_sample_count(int _sample_count)
{
	sample_count = _sample_count;
	sample_period = CAT_DAY_SECONDS / sample_count;
}

static int16_t make_value(CAT_log_cell* cell, int view)
{
#define LOG_CELL_HAS_FLAG(flag) (cell->flags & (flag))
#define NEG1_IF_NOT_FLAG(x, flag) (LOG_CELL_HAS_FLAG(flag) ? (x) : -1)
#define NEG1_IF_ZERO(x) ((x) == 0 ? -1 : x)
	switch(view)
	{
		case CAT_MONITOR_GRAPH_VIEW_CO2:
			return NEG1_IF_NOT_FLAG(cell->co2_ppmx1, CAT_LOG_CELL_FLAG_HAS_CO2);
		case CAT_MONITOR_GRAPH_VIEW_PM_2_5:
			return NEG1_IF_NOT_FLAG(cell->pm_ugmx100[1], CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case CAT_MONITOR_GRAPH_VIEW_PN_10_0:
			return NEG1_IF_NOT_FLAG(cell->pn_ugmx100[4], CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case CAT_MONITOR_GRAPH_VIEW_TEMP:
			return NEG1_IF_NOT_FLAG(cell->temp_Cx1000/10, CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case CAT_MONITOR_GRAPH_VIEW_RH:
			return NEG1_IF_NOT_FLAG(cell->rh_pctx100, CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case CAT_MONITOR_GRAPH_VIEW_PRESS:
			return NEG1_IF_ZERO(cell->pressure_hPax10);
		case CAT_MONITOR_GRAPH_VIEW_VOC:
			return NEG1_IF_ZERO(cell->voc_index);
		case CAT_MONITOR_GRAPH_VIEW_NOX:
			return NEG1_IF_ZERO(cell->nox_index);
		default:
			return 0;
	}
}

static uint64_t diff;

static void load_graph_data()
{
	seek_direction = CAT_datecmp(&start_date, &start_date_last);
	start_date_last = start_date;

	struct tm start_tm = {0};
	start_tm.tm_year = start_date.year;
	start_tm.tm_mon = start_date.month-1;
	start_tm.tm_mday = start_date.day;
	uint64_t start_time = timegm(&start_tm) + SPOOKY_TIME_CONSTANT; // <- WHATEVER. I DON'T EVEN KNOW
	
	CAT_log_cell cell;
	if(seek_bookmark == -1)
	{
		seek_bookmark = CAT_read_log_cell_before_time(seek_bookmark, start_time, &cell);
	}
	else
	{
		if(seek_direction == -1)
			seek_bookmark = CAT_read_log_cell_before_time(seek_bookmark, start_time, &cell);
		else if(seek_direction == 1)
			seek_bookmark = CAT_read_log_cell_after_time(seek_bookmark, start_time, &cell);
	}

	uint64_t sample_time = start_time;
	int sample_bookmark = seek_bookmark;
	value_max = INT16_MIN;
	value_min = INT16_MAX;
	first_valid_idx = -1;
	last_valid_idx = -1;

	for(int i = 0; i < sample_count; i++)
	{
		values[i] = -1;
		indices[i] = -1;

		CAT_log_cell cell;
		int last_sample_bookmark = sample_bookmark;
		sample_bookmark = CAT_read_log_cell_after_time(sample_bookmark, sample_time, &cell);

		if(sample_bookmark > 0 && sample_bookmark != last_sample_bookmark)
		{
			int16_t value = make_value(&cell, view);
			if(value < 0 && view != CAT_MONITOR_GRAPH_VIEW_TEMP)
				value = -1;

			timestamps[i] = cell.timestamp;
			CAT_datetime sample_date;
			CAT_make_datetime(cell.timestamp, &sample_date);
			
			if(value != -1 && sample_date.day == start_date.day)
			{
				values[i] = value;
				indices[i] = sample_bookmark;
				
				if(first_valid_idx == -1)
					first_valid_idx = i;
				last_valid_idx = i;

				if(value > value_max)
				{
					value_max = values[i];
					value_max_idx = i;
				}
				if(value < value_min)
				{
					value_min = values[i];
					value_min_idx = i;
				}
			}
		}

		sample_time += sample_period;
	}

	focus_idx = value_max_idx;
}

void CAT_monitor_graph_load_date(CAT_datetime date)
{
	start_date = date;
	if(start_date_last.year == 0)
		start_date_last = date;
	mode = MODE_LOADING;
}

bool CAT_monitor_graph_is_loading()
{
	return mode == MODE_LOADING;
}

bool CAT_monitor_graph_did_load_succeed()
{
	return mode != MODE_LOADING && last_valid_idx > 0;
}

int16_t* CAT_monitor_graph_get_values()
{
	return values;
}

uint64_t* CAT_monitor_graph_get_timestamps()
{
	return timestamps;
}

int32_t* CAT_monitor_graph_get_indices()
{
	return indices;
}

int CAT_monitor_graph_get_first_idx()
{
	return first_valid_idx;
}

int CAT_monitor_graph_get_last_idx()
{
	return last_valid_idx;
}

void CAT_monitor_graph_tick()
{
	if(mode == MODE_LOADING)
	{
		load_graph_data();
		mode = MODE_VIEWING;
	}
}

static int window_x;
static int window_y;
static int window_h;

static uint8_t pps = 1;
static int center_x = 0;
static int center_y = 0;

void CAT_monitor_graph_set_focus(int idx)
{
	focus_idx = CAT_clamp(idx, first_valid_idx, last_valid_idx);
}

void CAT_monitor_graph_set_scale(int scale)
{
	pps = CAT_clamp(scale, 1, 16);
}

static int window_transform_x(int x)
{
	x -= center_x;
	x *= pps;

	x += window_x + sample_count / 2;
	return x;
}

static int window_transform_y(int y)
{
	float yf = y;
	yf -= center_y;
	float range = value_max-value_min;
	float margin = range * 0.25f;
	range += margin * 2;
	if(range == 0)
		return window_y + window_h / 2;
	yf = inv_lerp(yf, 0, range) * window_h;
	yf *= pps;
	
	yf += window_h / 2;
	yf = window_y + window_h - yf;
	return yf;
}

void CAT_monitor_graph_draw(int x, int y, int h)
{
	window_x = x;
	window_y = y;
	window_h = h;

	CAT_fillberry(window_x, window_y, sample_count, window_h, CAT_GRAPH_BG);

	if(mode == MODE_LOADING)
	{
		center_textf(window_x + sample_count/2, window_y + window_h/2, 3, CAT_GRAPH_FG, "Loading");
		return;
	}
	else if(last_valid_idx <= 0)
	{
		center_textf(window_x + sample_count/2, window_y + window_h/2, 3, CAT_RED, "N/A");
		return;
	}

	int scaled_halfwidth = (sample_count/2) / pps;
	center_x = CAT_clamp(focus_idx, scaled_halfwidth, sample_count-scaled_halfwidth);
	if(pps > 1)
		center_y = values[focus_idx];
	else
		center_y = (value_min + value_max) / 2;

	CAT_CSCLIP_set_rect(window_x, window_y, window_x+sample_count, window_y+window_h);
	for(int i = 0; i < last_valid_idx; i++)
	{
		if(values[i] == -1 || values[i+1] == -1)
			continue;

		int x0 = window_transform_x(i);
		int y0 = window_transform_y(values[i]);
		int x1 = window_transform_x(i+1);
		int y1 = window_transform_y(values[i+1]);
		if(CAT_CSCLIP(&x0, &y0, &x1, &y1))
			CAT_lineberry(x0, y0, x1, y1, CAT_GRAPH_FG);
	}
}

void CAT_monitor_graph_draw_cursor(int x, uint16_t c)
{
	CAT_lineberry
	(
		window_x+x, window_y,
		window_x+x, window_y+window_h,
		c
	);
}

#define MARGIN 12
#define WINDOW_X0 MARGIN
#define WINDOW_Y0 64
#define WINDOW_H 128
#define WINDOW_W (CAT_LCD_SCREEN_W - 2 * MARGIN)
#define WINDOW_X1 (WINDOW_X0 + WINDOW_W - 1)
#define WINDOW_Y1 (WINDOW_Y0 + WINDOW_H - 1)

static bool should_exit = false;

void CAT_monitor_graph_enter(CAT_datetime date)
{
	CAT_monitor_graph_set_view(CAT_MONITOR_GRAPH_VIEW_CO2);
	CAT_monitor_graph_set_sample_count(WINDOW_W);
	CAT_monitor_graph_load_date(date);
	CAT_monitor_graph_set_scale(1);
	should_exit = false;
}

void CAT_monitor_graph_logic()
{
	CAT_monitor_graph_tick();

	if(mode == MODE_VIEWING)
	{
		if(CAT_input_touch_rect(window_x, window_y, sample_count, window_h))
		{
			CAT_monitor_graph_set_view(view+1);
			CAT_monitor_graph_load_date(start_date);
		}

		if(CAT_monitor_graph_did_load_succeed())
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
			if(focus_idx+dx < first_valid_idx)
			{
				CAT_datetime yesterday = start_date;
				yesterday.day -= 1;
				CAT_monitor_graph_load_date(CAT_normalize_date(yesterday));
			}
			else if(focus_idx+dx > last_valid_idx)
			{
				CAT_datetime tomorrow = start_date;
				tomorrow.day += 1;
				CAT_monitor_graph_load_date(CAT_normalize_date(tomorrow));	
			}
			CAT_monitor_graph_set_focus(focus_idx+dx);

			if(CAT_input_pressed(CAT_BUTTON_A))
				CAT_monitor_graph_set_scale(pps+1);
			if(CAT_input_pressed(CAT_BUTTON_B))
			{
				if(pps == 1)
					should_exit = true;
				else
					CAT_monitor_graph_set_scale(pps-1);
			}
			
			if(view == CAT_MONITOR_GRAPH_VIEW_CO2 || view == CAT_MONITOR_GRAPH_VIEW_PN_10_0)
			{
				if(CAT_input_pressed(CAT_BUTTON_SELECT))
					CAT_monitor_ACH_enter(start_date);
			}
		}
		else
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				should_exit = true;
		}
	}
}

bool CAT_monitor_graph_should_exit()
{
	return should_exit;
}

static const char* get_title_string(int view)
{
	switch(view)
	{
		case CAT_MONITOR_GRAPH_VIEW_CO2: return "CO2";
		case CAT_MONITOR_GRAPH_VIEW_PN_10_0: return "PN 10";
		case CAT_MONITOR_GRAPH_VIEW_PM_2_5: return "PM 2.5";
		case CAT_MONITOR_GRAPH_VIEW_TEMP: return "TEMPERATURE";
		case CAT_MONITOR_GRAPH_VIEW_RH: return "REL. HUMIDITY";
		case CAT_MONITOR_GRAPH_VIEW_PRESS: return "PRESSURE";
		case CAT_MONITOR_GRAPH_VIEW_VOC: return "VOC";
		case CAT_MONITOR_GRAPH_VIEW_NOX: return "NOX";
		default: return "None";
	}
}

static char* make_value_string(int view, int16_t value)
{
	static char buf[32];
	switch(view)
	{
		case CAT_MONITOR_GRAPH_VIEW_CO2:
			snprintf(buf, sizeof(buf), "%d ppm", value);
		break;
		case CAT_MONITOR_GRAPH_VIEW_PM_2_5:
			snprintf(buf, sizeof(buf), "" CAT_FLOAT_FMT "\4 g/m\5", CAT_FMT_FLOAT(value / 100.0f));
		break;
		case CAT_MONITOR_GRAPH_VIEW_PN_10_0:
			snprintf(buf, sizeof(buf), "" CAT_FLOAT_FMT " #/cm\5", CAT_FMT_FLOAT(value / 100.0f));
		break;
		case CAT_MONITOR_GRAPH_VIEW_TEMP:
			snprintf(buf, sizeof(buf), "" CAT_FLOAT_FMT " %s", CAT_FMT_FLOAT(CAT_AQ_map_celsius((float) value / 100.0f)), CAT_AQ_get_temperature_unit_string());
		break;
		case CAT_MONITOR_GRAPH_VIEW_RH:
			snprintf(buf, sizeof(buf), "" CAT_FLOAT_FMT "%% RH", CAT_FMT_FLOAT(value / 100.0f));
		break;
		case CAT_MONITOR_GRAPH_VIEW_PRESS:
			snprintf(buf, sizeof(buf), "" CAT_FLOAT_FMT " hPa", CAT_FMT_FLOAT(value / 10.0f));
		break;
		case CAT_MONITOR_GRAPH_VIEW_VOC:
		case CAT_MONITOR_GRAPH_VIEW_NOX:
			snprintf(buf, sizeof(buf), "%d", value);
		break;
		default:
			snprintf(buf, sizeof(buf), "N/A");
		break;
	}
	return buf;
}

void CAT_monitor_graph_render()
{
	CAT_draw_subpage_markers(32, CAT_MONITOR_GRAPH_VIEW_MAX, view, CAT_WHITE);

	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(12, WINDOW_Y0-14, "%s", get_title_string(view));
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(WINDOW_X1 - 10 * CAT_GLYPH_WIDTH, WINDOW_Y0-14, "%.2d/%.2d/%.4d", start_date.month, start_date.day, start_date.year);

	CAT_monitor_graph_draw(WINDOW_X0, WINDOW_Y0, WINDOW_Y1-WINDOW_Y0);

	if(mode == MODE_VIEWING && CAT_monitor_graph_did_load_succeed())
	{
		if(indices[focus_idx] > 0)
		{
			CAT_circberry(window_transform_x(focus_idx), window_transform_y(values[focus_idx]), 2, CAT_RED);
			
			int16_t cursor_value = values[focus_idx];
			CAT_datetime cursor_date;
			CAT_make_datetime(timestamps[focus_idx], &cursor_date);

			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(WINDOW_X1-CAT_GLYPH_WIDTH*3, WINDOW_Y1 + 4, "%.2dx", pps);
			
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_scale(2);
			CAT_draw_textf(window_x, WINDOW_Y1+4, "%s", make_value_string(view, cursor_value));

			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(window_x, WINDOW_Y1+4+28, "at %.2d:%.2d on %.2d/%.2d/%.4d", cursor_date.hour, cursor_date.minute, cursor_date.month, cursor_date.day, cursor_date.year);
			
			if(view == CAT_MONITOR_GRAPH_VIEW_CO2 || view == CAT_MONITOR_GRAPH_VIEW_PN_10_0)
			{
				CAT_set_text_colour(CAT_WHITE);
				CAT_draw_textf(window_x, WINDOW_Y1+4+28+28, "[SELECT] to go to\nACH calculator\n");
			}
		}
		else
		{
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_scale(1);
			CAT_draw_textf(window_x, WINDOW_Y1+4, "INVALID\n");
		}

		CAT_set_text_colour(CAT_96_GREY);
		CAT_draw_textf(WINDOW_X0, CAT_LCD_SCREEN_H-32, "A/B to control zoom\nTap graph to change metric");
	}
}

