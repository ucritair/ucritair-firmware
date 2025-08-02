#include "cat_monitor_graph.h"

#include <time.h>
#include "cat_input.h"
#include "cat_math.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_monitor_graphics_utils.h"
#include "cat_monitor_ach.h"

typedef enum
{
	MODE_LOADING,
	MODE_VIEWING,
	MODE_ACH_START,
	MODE_ACH_END,
	MODE_ACH_CALC,
	MODE_EXIT
} graph_mode;
static int mode = MODE_LOADING;

static CAT_datetime start_date = {0};
static CAT_datetime start_date_last = {0};
static int seek_bookmark = -1;
static int seek_direction = -1;

#define MARGIN 12
#define WINDOW_X0 MARGIN
#define WINDOW_Y0 64
#define WINDOW_W (CAT_LCD_SCREEN_W - 2 * MARGIN)
#define WINDOW_H 128
#define WINDOW_X1 (WINDOW_X0 + WINDOW_W - 1)
#define WINDOW_Y1 (WINDOW_Y0 + WINDOW_H - 1)

#define SAMPLE_COUNT WINDOW_W
#define SAMPLE_PERIOD (CAT_DAY_SECONDS / SAMPLE_COUNT)

static int16_t values[SAMPLE_COUNT];
static uint64_t timestamps[SAMPLE_COUNT];
static int32_t indices[SAMPLE_COUNT];
static int16_t value_min;
static uint16_t value_min_idx;
static int16_t value_max;
static uint16_t value_max_idx;
static uint16_t last_valid_idx;

static int center_x;
static int center_y;
static uint8_t pps;
static int reticle_value_idx;

typedef enum
{
	VIEW_CO2,
	VIEW_PN_10_0,
	VIEW_PM_2_5,
	VIEW_TEMP,
	VIEW_RH,
	VIEW_PRESS,
	VIEW_VOC,
	VIEW_NOX,
	VIEW_MAX
} graph_view;
static int view = VIEW_CO2;

static float ach_cache[VIEW_MAX];
static int ach_start_idx;
static int ach_end_idx;

static int16_t make_value(CAT_log_cell* cell, int view)
{
#define LOG_CELL_HAS_FLAG(flag) (cell->flags & (flag))
#define NEG1_IF_NOT_FLAG(x, flag) (LOG_CELL_HAS_FLAG(flag) ? (x) : -1)
#define NEG1_IF_ZERO(x) ((x) == 0 ? -1 : x)
	switch(view)
	{
		case VIEW_CO2:
			return NEG1_IF_NOT_FLAG(cell->co2_ppmx1, CAT_LOG_CELL_FLAG_HAS_CO2);
		case VIEW_PM_2_5:
			return NEG1_IF_NOT_FLAG(cell->pm_ugmx100[1], CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case VIEW_PN_10_0:
			return NEG1_IF_NOT_FLAG(cell->pn_ugmx100[4], CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case VIEW_TEMP:
			return NEG1_IF_NOT_FLAG(cell->temp_Cx1000/10, CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case VIEW_RH:
			return NEG1_IF_NOT_FLAG(cell->rh_pctx100, CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case VIEW_PRESS:
			return NEG1_IF_ZERO(cell->pressure_hPax10);
		case VIEW_VOC:
			return NEG1_IF_ZERO(cell->voc_index);
		case VIEW_NOX:
			return NEG1_IF_ZERO(cell->nox_index);
		default:
			return 0;
	}
}

static void load_graph_data()
{
	seek_direction = CAT_datecmp(&start_date, &start_date_last);
	start_date_last = start_date;

	struct tm start_tm = {0};
	start_tm.tm_year = start_date.year;
	start_tm.tm_mon = start_date.month-1;
	start_tm.tm_mday = start_date.day;
	uint64_t start_time = mktime(&start_tm);
	
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
	last_valid_idx = 0;

	for(int i = 0; i < SAMPLE_COUNT; i++)
	{
		CAT_log_cell cell;
		sample_bookmark = CAT_read_log_cell_after_time(sample_bookmark, sample_time, &cell);

		if(sample_bookmark > 0)
		{
			values[i] = make_value(&cell, view);
			timestamps[i] = cell.timestamp;

			CAT_datetime sample_date;
			CAT_make_datetime(cell.timestamp, &sample_date);
			if(values[i] != -1 && sample_date.day == start_date.day)
			{
				indices[i] = sample_bookmark;
				last_valid_idx = i;
			}

			if(values[i] > value_max)
			{
				value_max = values[i];
				value_max_idx = i;
			}
			if(values[i] < value_min)
			{
				value_min = values[i];
				value_min_idx = i;
			}
		}
		else
		{
			values[i] = -1;
			indices[i] = -1;
		}

		sample_time += SAMPLE_PERIOD;
	}

	pps = 1;
	center_x = WINDOW_W/2;
	center_y = (value_min + value_max) / 2;
	reticle_value_idx = value_max_idx;

	for(int i = 0; i < VIEW_MAX; i++)
		ach_cache[i] = -1;
}

bool is_graph_valid()
{
	return last_valid_idx > 0;
}

bool is_ach_possible()
{
	return is_graph_valid() && view == VIEW_CO2 || view == VIEW_PN_10_0;
}

void CAT_monitor_graph_load_date(CAT_datetime date)
{
	start_date = date;
	mode = MODE_LOADING;
	if(start_date_last.year == 0)
		start_date_last = date;
}

int move_cursor(int cursor, int left, int right)
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
	cursor += dx;
	return clamp(cursor, left, right);
}

void CAT_monitor_graph_logic()
{
	if(mode == MODE_LOADING)
	{
		load_graph_data();
		mode = MODE_VIEWING;
	}
	else if(mode == MODE_VIEWING)
	{
		if(CAT_input_touch_rect(WINDOW_X0, WINDOW_Y0, WINDOW_W, WINDOW_H))
		{
			view = (view+1) % VIEW_MAX;
			mode = MODE_LOADING;
		}

		if(CAT_input_pressed(CAT_BUTTON_A))
			pps += 1;
		if(CAT_input_pressed(CAT_BUTTON_B))
		{
			pps -= 1;
			if(pps == 0)
			{
				mode = MODE_EXIT;
			}
		}
		pps = clamp(pps, 1, 16);

		if(CAT_input_pressed(CAT_BUTTON_SELECT) && is_ach_possible() && pps == 1)
		{
			CAT_monitor_graph_set_ACH_data(view, values, timestamps, indices, SAMPLE_COUNT);
			CAT_monitor_graph_auto_ACH_cursors(&ach_start_idx, &ach_end_idx);
			mode = MODE_ACH_START;
			ach_cache[view] = CAT_monitor_graph_get_ACH(ach_start_idx, ach_end_idx);
		}

		reticle_value_idx = move_cursor(reticle_value_idx, 0, SAMPLE_COUNT-1);
		int scaled_halfwidth = (WINDOW_W/2) / pps;
		center_x = clamp(reticle_value_idx, scaled_halfwidth, WINDOW_W-scaled_halfwidth);
		if(pps > 1)
			center_y = values[reticle_value_idx];
		else
			center_y = (value_min + value_max) / 2;
	}
	else if(mode == MODE_ACH_START)
	{
		ach_start_idx = move_cursor(ach_start_idx, 0, SAMPLE_COUNT-1);

		if(CAT_input_pressed(CAT_BUTTON_A) && indices[ach_start_idx] != -1)
			mode = MODE_ACH_END;
	}
	else if(mode == MODE_ACH_END)
	{
		ach_end_idx = move_cursor(ach_end_idx, ach_start_idx, SAMPLE_COUNT-1);

		if(CAT_input_pressed(CAT_BUTTON_A) && indices[ach_end_idx] != -1)
			mode = MODE_ACH_CALC;
	}
	if(mode == MODE_ACH_CALC)
	{
		ach_cache[view] = CAT_monitor_graph_get_ACH(ach_start_idx+1, ach_end_idx);
		mode = MODE_VIEWING;
	}
}

bool CAT_monitor_graph_is_loading()
{
	return mode == MODE_LOADING;
}

bool CAT_monitor_graph_should_exit()
{
	return mode == MODE_EXIT;
}

static int window_x(int x)
{
	x -= center_x;
	x *= pps;

	x += WINDOW_X0 + WINDOW_W / 2;
	return x;
}

static int window_y(int y)
{
	float yf = y;
	yf -= center_y;
	float range = value_max-value_min;
	float margin = range * 0.25f;
	range += margin * 2;
	if(range == 0)
		return WINDOW_Y1 - WINDOW_H / 2;
	yf = inv_lerp(yf, 0, range) * WINDOW_H;
	yf *= pps;
	
	yf += WINDOW_H / 2;
	yf = WINDOW_Y1 - yf;
	return yf;
}

#define BG_COLOUR 0xe7bf
#define FG_COLOUR 0x6b11
#define RETICLE_COLOUR CAT_RED

static const char* get_title_string(int view)
{
	switch(view)
	{
		case VIEW_CO2: return "CO2";
		case VIEW_PN_10_0: return "PN 10";
		case VIEW_PM_2_5: return "PM 2.5";
		case VIEW_TEMP: return "TEMPERATURE";
		case VIEW_RH: return "REL. HUMIDITY";
		case VIEW_PRESS: return "PRESSURE";
		case VIEW_VOC: return "VOC";
		case VIEW_NOX: return "NOX";
		default: return "None";
	}
}

static char* make_value_string(int view, int16_t value)
{
	static char buf[32];
	switch(view)
	{
		case VIEW_CO2:
			snprintf(buf, sizeof(buf), "%d ppm", value);
		break;
		case VIEW_PM_2_5:
			snprintf(buf, sizeof(buf), "%.1f\4 g/m\5", (float) value / 100.0f);
		break;
		case VIEW_PN_10_0:
			snprintf(buf, sizeof(buf), "%.1f #/cm\5", (float) value / 100.0f);
		break;
		case VIEW_TEMP:
			snprintf(buf, sizeof(buf), "%.1f %s", CAT_AQ_map_celsius((float) value / 100.0f), CAT_AQ_get_temperature_unit_string());
		break;
		case VIEW_RH:
			snprintf(buf, sizeof(buf), "%.1f%% RH", (float) value / 100.0f);
		break;
		case VIEW_PRESS:
			snprintf(buf, sizeof(buf), "%.1f hPa", (float) value / 10.0f);
		break;
		case VIEW_VOC:
		case VIEW_NOX:
			snprintf(buf, sizeof(buf), "%d", value);
		break;
		default:
			snprintf(buf, sizeof(buf), "N/A");
		break;
	}
	return buf;
}

#define ACH_Y (WINDOW_Y1+4+28+14+8)

void CAT_monitor_graph_render()
{
	draw_subpage_markers(32, VIEW_MAX, view);

	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(12, WINDOW_Y0-14, "%s", get_title_string(view));
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(WINDOW_X1 - 10 * CAT_GLYPH_WIDTH, WINDOW_Y0-14, "%.2d/%.2d/%.4d", start_date.month, start_date.day, start_date.year);

	CAT_fillberry(WINDOW_X0, WINDOW_Y0, WINDOW_W, WINDOW_H, BG_COLOUR);
	CAT_strokeberry(WINDOW_X0, WINDOW_Y0, WINDOW_W, WINDOW_H, CAT_WHITE);

	if(mode == MODE_LOADING)
	{
		center_textf(WINDOW_X0 + WINDOW_W/2, WINDOW_Y0 + WINDOW_H/2, 3, CAT_MONITOR_BLUE, "Loading");
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(WINDOW_X0, WINDOW_Y1+4, "Please wait...");
		return;
	}
	if(last_valid_idx == 0)
	{
		center_textf(WINDOW_X0 + WINDOW_W/2, WINDOW_Y0 + WINDOW_H/2, 3, CAT_RED, "N/A");
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(WINDOW_X0, WINDOW_Y1+4, "Insufficient data found\nfor this date!");
		return;
	}

	CAT_CSCLIP_set_rect(WINDOW_X0, WINDOW_Y0, WINDOW_X1, WINDOW_Y1);
	for(int i = 0; i < SAMPLE_COUNT-1; i++)
	{
		if(values[i] == -1 || values[i+1] == -1)
			continue;

		int x0 = window_x(i);
		int y0 = window_y(values[i]);
		int x1 = window_x(i+1);
		int y1 = window_y(values[i+1]);
		if(CAT_CSCLIP(&x0, &y0, &x1, &y1))
			CAT_lineberry(x0, y0, x1, y1, FG_COLOUR);
	}

	if(mode == MODE_VIEWING)
	{
		CAT_circberry(window_x(reticle_value_idx), window_y(values[reticle_value_idx]), 3, CAT_RED);
	}
	else if(mode == MODE_ACH_START || mode == MODE_ACH_END)
	{
		CAT_lineberry(WINDOW_X0+ach_start_idx, WINDOW_Y0, WINDOW_X0+ach_start_idx, WINDOW_Y1, CAT_GREEN);
		CAT_lineberry(WINDOW_X0+ach_end_idx, WINDOW_Y0, WINDOW_X0+ach_end_idx, WINDOW_Y1, CAT_RED);
	}

	int cursor = reticle_value_idx;
	if(mode == MODE_ACH_START)
		cursor = ach_start_idx;
	if(mode == MODE_ACH_END || mode == MODE_ACH_CALC)
		cursor = ach_end_idx;
	if(indices[cursor] > 0)
	{
		int16_t cursor_value = values[cursor];
		CAT_datetime cursor_date;
		CAT_make_datetime(timestamps[cursor], &cursor_date);

		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(WINDOW_X1-CAT_GLYPH_WIDTH*3, WINDOW_Y1 + 4, "%.2dx", pps);
		
		CAT_set_text_colour(CAT_WHITE);
		CAT_set_text_scale(2);
		CAT_draw_textf(WINDOW_X0, WINDOW_Y1+4, "%s", make_value_string(view, cursor_value));

		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(WINDOW_X0, WINDOW_Y1+4+28, "at %.2d:%.2d on %.2d/%.2d/%.4d", cursor_date.hour, cursor_date.minute, cursor_date.month, cursor_date.day, cursor_date.year);

		switch(mode)
		{
			case MODE_ACH_START:
				CAT_set_text_colour(CAT_WHITE);
				CAT_draw_textf(WINDOW_X0, ACH_Y, "[A] to confirm start\n");
			break;
			case MODE_ACH_END:
				CAT_set_text_colour(CAT_WHITE);
				CAT_draw_textf(WINDOW_X0, ACH_Y, "[A] to confirm end\n");
			break;
			case MODE_ACH_CALC:
				CAT_set_text_colour(CAT_WHITE);
				CAT_draw_textf(WINDOW_X0, ACH_Y, "[A] to calculate\n");
			break;
		}
	}
	else
	{
		CAT_set_text_colour(CAT_WHITE);
		CAT_set_text_scale(1);
		CAT_draw_textf(WINDOW_X0, WINDOW_Y1+4, "INVALID\n");
	}

	if(mode == MODE_VIEWING && is_ach_possible())
	{	
		if(pps == 1)
		{
			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(WINDOW_X0, ACH_Y, ach_cache[view] == -1 ? "[SELECT] for e/ACH" : "ACH:");
		}
		if(ach_cache[view] != -1)
		{
			CAT_set_text_scale(2);
			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(WINDOW_X0, ACH_Y+14, "%.2f\n", ach_cache[view]);
		}

		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(WINDOW_X0, CAT_LCD_SCREEN_H-24, "Tap graph to change view");
	}
}

