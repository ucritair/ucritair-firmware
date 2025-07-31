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
	MODE_PRIMED,
	MODE_LOADING,
	MODE_VIEWING,
	MODE_CALCULATING,
	MODE_EXIT
} graph_mode;
static int mode = MODE_PRIMED;

static CAT_datetime start_date;
static CAT_datetime start_date_last;
static int seek_bookmark;
static int seek_direction;

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
static int32_t indices[SAMPLE_COUNT];
static int16_t value_min;
static uint16_t value_min_idx;
static int16_t value_max;
static uint16_t value_max_idx;

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
	uint64_t start_time = CAT_make_timestamp(&start_date);
	seek_direction = CAT_datecmp(&start_date, &start_date_last);
	
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
	start_date_last = start_date;

	uint64_t sample_time = start_time;
	int sample_bookmark = seek_bookmark;
	value_max = INT16_MIN;
	value_min = INT16_MAX;

	for(int i = 0; i < SAMPLE_COUNT; i++)
	{
		CAT_log_cell cell;
		sample_bookmark = CAT_read_log_cell_after_time(sample_bookmark, sample_time, &cell);

		if(sample_bookmark > 0)
		{
			values[i] = make_value(&cell, view);
			if(values[i] != -1)
				indices[i] = sample_bookmark;

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
}

void CAT_monitor_graph_load_date(CAT_datetime date)
{
	start_date = date;
	mode = MODE_PRIMED;
}

void CAT_monitor_graph_logic()
{
	if(mode == MODE_PRIMED)
	{
		mode = MODE_LOADING;
		return;
	}
	if(mode == MODE_LOADING)
	{
		load_graph_data();
		mode = MODE_VIEWING;
	}

	if(CAT_input_touch_rect(WINDOW_X0, WINDOW_Y0, WINDOW_W, WINDOW_H))
	{
		view = (view+1) % VIEW_MAX;
		mode = MODE_PRIMED;
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

	int dx = 0;
	if(CAT_input_held(CAT_BUTTON_LEFT, 0))
		dx -= 1;
	if(CAT_input_held(CAT_BUTTON_LEFT, 1))
		dx -= 3;
	if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
		dx += 1;
	if(CAT_input_held(CAT_BUTTON_RIGHT, 1))
		dx += 3;
	reticle_value_idx += dx;
	reticle_value_idx = clamp(reticle_value_idx, 0, SAMPLE_COUNT-1);

	int scaled_halfwidth = (WINDOW_W/2) / pps;
	center_x = clamp(reticle_value_idx, scaled_halfwidth, WINDOW_W-scaled_halfwidth);
	if(pps > 1)
		center_y = values[reticle_value_idx];
	else
		center_y = (value_min + value_max) / 2;
}

bool CAT_monitor_graph_is_loading()
{
	return mode == MODE_PRIMED;
}

bool CAT_monitor_graph_should_exit()
{
	return mode == MODE_EXIT;
}

static bool load_reticle_cell(CAT_log_cell* cell)
{
	int idx = indices[reticle_value_idx];
	if(idx != -1)
	{
		CAT_read_log_cell_at_idx(idx, cell);
		return true;
	}
	return false;
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

void CAT_monitor_graph_render()
{
	draw_subpage_markers(32, VIEW_MAX, view);

	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(12, WINDOW_Y0-14, "%s", get_title_string(view));
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(WINDOW_X1 - 10 * CAT_GLYPH_WIDTH, WINDOW_Y0-14, "%.2d/%.2d/%.4d", start_date.month, start_date.day, start_date.year);

	CAT_fillberry(WINDOW_X0, WINDOW_Y0, WINDOW_W, WINDOW_H, BG_COLOUR);
	CAT_strokeberry(WINDOW_X0, WINDOW_Y0, WINDOW_W, WINDOW_H, CAT_WHITE);

	if(mode == MODE_PRIMED)
	{
		center_textf(WINDOW_X0 + WINDOW_W/2, WINDOW_Y0 + WINDOW_H/2, 3, CAT_MONITOR_BLUE, "Loading");
		return;
	}
	if(value_min == value_max && indices[0] == -1)
	{
		center_textf(WINDOW_X0 + WINDOW_W/2, WINDOW_Y0 + WINDOW_H/2, 3, CAT_RED, "N/A");
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

	CAT_log_cell reticle_cell;
	if(load_reticle_cell(&reticle_cell))
	{
		CAT_circberry(window_x(reticle_value_idx), window_y(values[reticle_value_idx]), 3, CAT_RED);

		int16_t reticle_value = make_value(&reticle_cell, view);
		uint64_t reticle_time= reticle_cell.timestamp;
		CAT_datetime reticle_date;
		CAT_make_datetime(reticle_cell.timestamp, &reticle_date);

		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(WINDOW_X1-CAT_GLYPH_WIDTH*3, WINDOW_Y1 + 4, "%.2dx", pps);
		
		CAT_set_text_colour(CAT_WHITE);
		CAT_set_text_scale(2);
		CAT_draw_textf(WINDOW_X0, WINDOW_Y1+4, "%s", make_value_string(view, reticle_value));

		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(WINDOW_X0, WINDOW_Y1+32, "at %.2d:%.2d", reticle_date.hour, reticle_date.minute);
	}

	float ach = CAT_monitor_graph_calculate_ACH(view, values, indices, 0, SAMPLE_COUNT-1);
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(WINDOW_X0, WINDOW_Y1+32+16, "ACH: %.2f\n", ach);
}

