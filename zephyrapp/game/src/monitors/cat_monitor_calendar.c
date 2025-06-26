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

static bool focused = false;

static enum
{
	CALENDAR,
	GRAPH
} page = CALENDAR;

enum
{
	YEAR,
	MONTH,
	DAY
};

static CAT_datetime origin;
static CAT_datetime today;

static CAT_datetime target;
static CAT_datetime target_last;

static int bookmark = -1;
static int search_direction = -1;

#define GRAPH_X 12
#define GRAPH_Y 64
#define GRAPH_WIDTH (CAT_LCD_SCREEN_W - 2 * GRAPH_X)
#define GRAPH_HEIGHT 128
#define GRAPH_MAX_X (GRAPH_X + GRAPH_WIDTH - 1)
#define GRAPH_MAX_Y (GRAPH_Y + GRAPH_HEIGHT - 1)
#define GRAPH_SAMPLE_COUNT GRAPH_WIDTH
#define GRAPH_SAMPLE_DIST ((24 * 60 * 60) / GRAPH_SAMPLE_COUNT)

#define GRAPH_BG_COLOUR 0xe7bf
#define GRAPH_FG_COLOUR 0x6b11
#define GRAPH_RETICLE_COLOUR CAT_RED //0xadfa

static struct
{
	int16_t values[GRAPH_SAMPLE_COUNT];
	int32_t indices[GRAPH_SAMPLE_COUNT];
	int16_t value_min;
	int16_t value_max;
	uint16_t range;

	int center_x;
	int center_y;
	uint8_t pps;

	int target;
} graph;

static int graph_current_cell_idx;
static CAT_log_cell graph_current_cell;

static enum
{
	CO2,
	PN_10_0,
	PM_2_5,
	TEMP,
	RH,
	PRESS,
	VOC,
	NOX,
	GRAPH_VIEW_MAX
} graph_view = CO2;

static int16_t graph_make_value(CAT_log_cell* cell)
{
#define LOG_CELL_HAS_FLAG(flag) (cell->flags & (flag))
#define NEG1_IF_NOT_FLAG(x, flag) (LOG_CELL_HAS_FLAG(flag) ? (x) : -1)
#define NEG1_IF_ZERO(x) ((x) == 0 ? -1 : x)

	switch(graph_view)
	{
		case CO2:
			return NEG1_IF_NOT_FLAG(cell->co2_ppmx1, CAT_LOG_CELL_FLAG_HAS_CO2);
		case PM_2_5:
			return NEG1_IF_NOT_FLAG(cell->pm_ugmx100[1], CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case PN_10_0:
			return NEG1_IF_NOT_FLAG(cell->pn_ugmx100[4], CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case TEMP:
			return NEG1_IF_NOT_FLAG(cell->temp_Cx1000/10, CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case RH:
			return NEG1_IF_NOT_FLAG(cell->rh_pctx100, CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case PRESS:
			return NEG1_IF_ZERO(cell->pressure_hPax10);
		case VOC:
			return NEG1_IF_ZERO(cell->voc_index);
		case NOX:
			return NEG1_IF_ZERO(cell->nox_index);
		default:
			return 0;
	}
}

static const char* graph_get_title()
{
	switch(graph_view)
	{
		case CO2: return "CO2";
		case PN_10_0: return "PN 10";
		case PM_2_5: return "PM 2.5";
		case TEMP: return "Temperature";
		case RH: return "Rel. Humidity";
		case PRESS: return "Pressure";
		case VOC: return "VOC";
		case NOX: return "NOX";
		default: return "None";
	}
}

static char* graph_make_value_text(int16_t value)
{
	static char buf[32];

	switch(graph_view)
	{
		case CO2:
			snprintf(buf, sizeof(buf), "%d ppm", value);
		break;
		case PM_2_5:
			snprintf(buf, sizeof(buf), "%.1f\4 g/m\5", (float) value / 100.0f);
		break;
		case PN_10_0:
			snprintf(buf, sizeof(buf), "%.1f #/cm\5", (float) value / 100.0f);
		break;
		case TEMP:
			snprintf(buf, sizeof(buf), "%.1f %s", CAT_AQ_map_celsius((float) value / 100.0f), CAT_AQ_get_temperature_unit_string());
		break;
		case RH:
			snprintf(buf, sizeof(buf), "%.1f%% RH", (float) value / 100.0f);
		break;
		case PRESS:
			snprintf(buf, sizeof(buf), "%.1f hPa", (float) value / 10.0f);
		break;
		case VOC:
		case NOX:
			snprintf(buf, sizeof(buf), "%d", value);
		break;
		default:
			snprintf(buf, sizeof(buf), "N/A");
		break;
	}

	return buf;
}

static void graph_init()
{
	graph.value_max = INT16_MIN;
	graph.value_min = INT16_MAX;
	graph.range = UINT16_MAX;

	struct tm target_tm = {0};
	target_tm.tm_year = target.year;
	target_tm.tm_mon = target.month-1;
	target_tm.tm_mday = target.day;
	uint64_t start_time = mktime(&target_tm);

	search_direction = CAT_datecmp(&target, &target_last);
	CAT_printf("Direction is %d. Starting from %d\n", search_direction, bookmark);
	CAT_log_cell cell;
	if(bookmark == -1)
	{
		bookmark = CAT_read_log_cell_before_time(bookmark, start_time, &cell);
	}
	else
	{
		if(search_direction == -1)
			bookmark = CAT_read_log_cell_before_time(bookmark, start_time, &cell);
		else if(search_direction == 1)
			bookmark = CAT_read_log_cell_after_time(bookmark, start_time, &cell);
	}
	target_last = target;
	CAT_printf("Filling from %d\n", bookmark);

	int memo = bookmark;
	uint64_t sample_time = start_time;

	for(int i = 0; i < GRAPH_SAMPLE_COUNT; i++)
	{
		CAT_log_cell cell;
		memo = CAT_read_log_cell_after_time(memo, sample_time, &cell);

		if(memo > 0)
		{
			graph.values[i] = graph_make_value(&cell);
			if(graph.values[i] != -1)
				graph.indices[i] = memo;
		}
		else
		{
			graph.values[i] = -1;
			graph.indices[i] = -1;
		}

		if(graph.values[i] > graph.value_max)
		{
			graph.value_max = graph.values[i];
			graph.target = i;
		}
		graph.value_min = min(graph.value_min, graph.values[i]);

		sample_time += GRAPH_SAMPLE_DIST;
	}

	graph.pps = 1;
	graph.center_x = GRAPH_WIDTH/2;
	graph.center_y = (graph.value_min + graph.value_max) / 2;
	graph.range = graph.value_max - graph.value_min;
}

void graph_logic()
{
	if(CAT_input_touch_rect(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT))
	{
		graph_view = (graph_view+1) % GRAPH_VIEW_MAX;
		graph_init();
	}

	if(CAT_input_pressed(CAT_BUTTON_A))
		graph.pps += 1;
	if(CAT_input_pressed(CAT_BUTTON_B))
	{
		graph.pps -= 1;
		if(graph.pps == 0)
		{
			page = CALENDAR;
			return;
		}
	}
	graph.pps = clamp(graph.pps, 1, 16);

	float dx = 0;
	float dy = 0;
	if(CAT_input_held(CAT_BUTTON_LEFT, 0))
		dx -= 1;
	if(CAT_input_held(CAT_BUTTON_LEFT, 1))
		dx -= 3;
	if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
		dx += 1;
	if(CAT_input_held(CAT_BUTTON_RIGHT, 1))
		dx += 3;
	graph.target += dx;
	graph.target = clamp(graph.target, 0, GRAPH_WIDTH-1);
	
	int wdw_hw = (GRAPH_WIDTH/2) / graph.pps;
	graph.center_x = clamp(graph.target, wdw_hw, GRAPH_WIDTH-wdw_hw);

	if(graph.pps > 1)
		graph.center_y = graph.values[graph.target];
	else
		graph.center_y = (graph.value_min + graph.value_max) / 2;

	graph_current_cell_idx = -1;
	if(graph.target >= 0 && graph.target < GRAPH_SAMPLE_COUNT)
	{
		graph_current_cell_idx = graph.indices[graph.target];
		if(graph_current_cell_idx != -1)
		{
			CAT_read_log_cell_at_idx(graph_current_cell_idx, &graph_current_cell);
		}
	}
}

static int graph_transform_x(int x)
{
	x -= graph.center_x;
	x *= graph.pps;

	x += GRAPH_X + GRAPH_WIDTH / 2;
	return x;
}

static int graph_transform_y(int y)
{
	float yf = y;
	yf -= graph.center_y;
	float range = graph.value_max-graph.value_min;
	float margin = range * 0.25f;
	range += margin * 2;
	if(range == 0)
		return GRAPH_MAX_Y - GRAPH_HEIGHT / 2;
	yf = inv_lerp(yf, 0, range) * GRAPH_HEIGHT;
	yf *= graph.pps;
	
	yf += GRAPH_HEIGHT / 2;
	yf = GRAPH_MAX_Y - yf;
	return yf;
}

void render_graph()
{
	draw_subpage_markers(32, GRAPH_VIEW_MAX, graph_view);

	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(12, GRAPH_Y-14, "%s", graph_get_title());
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(GRAPH_MAX_X - 10 * CAT_GLYPH_WIDTH, GRAPH_Y-14, "%.2d/%.2d/%.4d", target.month, target.day, target.year);

	CAT_fillberry(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, GRAPH_BG_COLOUR);
	CAT_CSCLIP_set_rect(GRAPH_X, GRAPH_Y, GRAPH_MAX_X, GRAPH_MAX_Y);
	for(int i = 0; i < GRAPH_SAMPLE_COUNT-1; i++)
	{
		if(graph.values[i] == -1 || graph.values[i+1] == -1)
			continue;

		int x0 = graph_transform_x(i);
		int y0 = graph_transform_y(graph.values[i]);
		int x1 = graph_transform_x(i+1);
		int y1 = graph_transform_y(graph.values[i+1]);
		if(CAT_CSCLIP(&x0, &y0, &x1, &y1))
			CAT_lineberry(x0, y0, x1, y1, GRAPH_FG_COLOUR);
	}
	CAT_strokeberry(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, CAT_WHITE);

	if(graph.value_min == -1 && graph.value_max == -1)
	{
		center_textf(GRAPH_X + GRAPH_WIDTH/2, GRAPH_Y + GRAPH_HEIGHT/2, 3, CAT_RED, "N/A");
	}
	else
	{
		if(graph_current_cell_idx != -1 && graph.values[graph.target != -1])
		{
			CAT_circberry(graph_transform_x(graph.target), graph_transform_y(graph.values[graph.target]), 3, CAT_RED);

			int16_t current_value = graph_make_value(&graph_current_cell);
			uint64_t current_timestamp = graph_current_cell.timestamp;
			struct tm current_datetime;
			gmtime_r(&current_timestamp, &current_datetime);

			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(GRAPH_MAX_X-CAT_GLYPH_WIDTH*3, GRAPH_MAX_Y + 4, "%.2dx", graph.pps);
			
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_scale(2);
			CAT_draw_textf(GRAPH_X, GRAPH_MAX_Y+4, "%s", graph_make_value_text(current_value));

			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(GRAPH_X, GRAPH_MAX_Y+32, "at %.2d:%.2d", current_datetime.tm_hour, current_datetime.tm_min);
		}
		else
		{
			CAT_circberry(graph_transform_x(graph.target), graph_transform_y(graph.center_y), 2, CAT_GREY);

			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_text(GRAPH_X, GRAPH_MAX_Y+4, "N/A");
		}
	}	
}

#define DATE_Y 48
#define DATE_X 12

#define GRID_Y (DATE_Y + 48)
#define GRID_COLS 7
#define GRID_MARGIN 8
#define GRID_SPACING 4
#define GRID_X GRID_MARGIN
#define GRID_CELL_R (((CAT_LCD_SCREEN_W - (GRID_MARGIN * 2) - (GRID_SPACING * (GRID_COLS-1))) / GRID_COLS) / 2)

#define CELL_GREY RGB8882565(164, 164, 164)

bool is_leap_year(int year)
{
	return (year % 4) || ((year % 100 == 0) && (year % 400)) ? 0 : 1;
}

int days_in_month(int year, int month)
{
	return month == 2 ? (28 + is_leap_year(year)) : 31 - (month-1) % 7 % 2;
}

static int clamp_date_part(int phase, int year, int month, int day)
{
	switch(phase)
	{
		case YEAR:
		{
			return clamp(year, origin.year, today.year);
		}

		case MONTH:
		{
			int min_month = 1;
			int max_month = 12;
			if(year == origin.year)
				min_month = origin.month;
			if(year == today.year)
				max_month = today.month;
			return clamp(month, min_month, max_month);
		}

		case DAY:
		{
			int days = days_in_month(year, month);
			int min_days = (year == origin.year && month == origin.month) ? origin.day : 1;
			int max_days = (year == today.year && month == today.month) ? today.day : days;
			return clamp(day, min_days, max_days);
		}
	}
	
	return -1;
}

int get_day_cell(int x, int y)
{
	x -= GRID_X;
	y -= GRID_Y;
	x /= GRID_CELL_R * 2 + GRID_SPACING;
	y /= GRID_CELL_R * 2 + GRID_SPACING;
	return y * 7 + x + 1;
}

void calendar_logic()
{
	if(!focused)
	{
		if(CAT_input_dismissal())
				CAT_monitor_soft_exit();
		if(CAT_input_pressed(CAT_BUTTON_LEFT))
			CAT_monitor_retreat();
		if(CAT_input_pressed(CAT_BUTTON_RIGHT))
			CAT_monitor_advance();

		if(CAT_input_released(CAT_BUTTON_A))
			focused = true;
	}
	else
	{
		if(CAT_input_pressed(CAT_BUTTON_B))
				focused = false;
			
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
		
		if(CAT_input_touch_down())
		{
			int was = target.day;
			target.day = get_day_cell(input.touch.x, input.touch.y);
			if(target.day == was)
			{
				graph_init();
				page = GRAPH;
			}
		}

		target.day = clamp_date_part(DAY, target.year, target.month, target.day);
	}
}

void render_calendar()
{
	if(!focused)
	{
		int cursor_y = center_textf(120, 60, 2, CAT_WHITE, "Calendar");
		cursor_y = underline(120, cursor_y, 2, CAT_WHITE, "Calendar");

		CAT_fillberry(120 - 60, 160 - 20, 120, 40, RGB8882565(35, 157, 235));
		center_textf(120, 160, CAT_input_held(CAT_BUTTON_A, 0) ? 3 : 2 ,CAT_WHITE, "Press A");
		return;
	}

	CAT_set_text_scale(2);
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(DATE_X, DATE_Y, "%.2d/%.2d/%.4d", target.month, target.day, target.year);

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
				CAT_datecmp(&date, &origin) >= 0 &&
				CAT_datecmp(&date, &today) <= 0 &&
				day <= days_in_month(target.year, target.month)
			)
			{
				if(day == target.day)
				{
					CAT_discberry(x + GRID_CELL_R, y + GRID_CELL_R, GRID_CELL_R, CAT_WHITE);
					CAT_circberry(x + GRID_CELL_R, y + GRID_CELL_R, GRID_CELL_R, CAT_WHITE);
					center_textf(x + GRID_CELL_R, y + GRID_CELL_R, 1, CAT_MONITOR_BLUE, "%d", day);
				}
				else
				{	
					CAT_circberry(x + GRID_CELL_R, y + GRID_CELL_R, GRID_CELL_R, CAT_WHITE);
					center_textf(x + GRID_CELL_R, y + GRID_CELL_R, 1, CAT_WHITE, "%d", day);
				}
				
			}
			else
			{
				CAT_circberry(x + GRID_CELL_R, y + GRID_CELL_R, GRID_CELL_R, CELL_GREY);
			}
			day += 1;
		}
	}

	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_text(GRID_X, GRID_Y + (GRID_CELL_R * 2 + GRID_SPACING) * 5 + 20, "Double-tap to select a date");
}

void CAT_monitor_MS_calendar(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			focused = false;

			CAT_log_cell first;
			CAT_read_first_calendar_cell(&first);
			CAT_make_datetime(first.timestamp, &origin);

			CAT_get_datetime(&today);
			target = today;

			target_last = today;
			bookmark = -1;
			search_direction = -1;
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			switch (page)
			{
				case CALENDAR:
					calendar_logic();
				break;
				case GRAPH:
					graph_logic();
				break;
			}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}
void CAT_monitor_render_calendar()
{
	switch (page)
	{
		case CALENDAR:
			render_calendar();
		break;
		case GRAPH:
			render_graph();
		break;
	}
}