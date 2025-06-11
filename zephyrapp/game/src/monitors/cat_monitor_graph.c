#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"

#include <math.h>
#include <stdio.h>
#include <time.h>

#define GRAPH_X 12
#define GRAPH_Y 42
#define GRAPH_WIDTH (CAT_LCD_SCREEN_W - 2 * GRAPH_X)
#define GRAPH_HEIGHT 128
#define GRAPH_MAX_X (GRAPH_X + GRAPH_WIDTH - 1)
#define GRAPH_MAX_Y (GRAPH_Y + GRAPH_HEIGHT - 1)
#define GRAPH_SAMPLE_COUNT GRAPH_WIDTH
#define GRAPH_SAMPLE_DIST ((24 * 60 * 60) / GRAPH_SAMPLE_COUNT)

#define GRAPH_BG_COLOUR 0xe7bf
#define GRAPH_FG_COLOUR 0x6b11
#define GRAPH_RETICLE_COLOUR CAT_RED //0xadfa

static bool focused = false;

static struct
{
	int16_t values[GRAPH_SAMPLE_COUNT];
	int16_t value_min;
	int16_t value_max;
	uint16_t range;

	int center_x;
	int center_y;
	uint8_t pps;

	int target;
} graph;

static struct tm today;
static int graph_indices[GRAPH_SAMPLE_COUNT];
static int graph_current_cell_idx;
static CAT_log_cell graph_current_cell;
static struct tm graph_current_timestamp;

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
		case RH: return "Relative Humidity";
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

static void graph_update()
{
	graph.value_max = INT16_MIN;
	graph.value_min = INT16_MAX;
	graph.range = UINT16_MAX;

	time_t now = (time_t) CAT_get_rtc_now();
	gmtime_r(&now, &today);
	today.tm_hour = 0;
	today.tm_min = 0;
	today.tm_sec = 0;
	uint64_t start_time = mktime(&today);

	uint64_t sample_time = start_time;
	int memo = -1;

	for(int i = 0; i < GRAPH_SAMPLE_COUNT; i++)
	{
		CAT_log_cell cell;
		memo = CAT_read_log_cell_after_time(memo, sample_time, &cell);
		if(memo > 0)
		{
			graph.values[i] = graph_make_value(&cell);
			if(graph.values[i] != -1)
				graph_indices[i] = memo;
		}
		else
		{
			graph.values[i] = 16 * sin(4 * i / (float) GRAPH_WIDTH);
			graph_indices[i] = -1;
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
	yf = inv_lerp(yf, 0, range) * GRAPH_HEIGHT;
	yf *= graph.pps;
	
	yf += GRAPH_HEIGHT / 2;
	yf = GRAPH_MAX_Y - yf;
	return yf;
}

void CAT_monitor_render_graph()
{
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(12, GRAPH_Y-14, "%s", graph_get_title());
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(GRAPH_MAX_X - 8 * CAT_GLYPH_WIDTH, GRAPH_Y-14, "%d/%d/%d", today.tm_mon+1, today.tm_mday, today.tm_year);

	CAT_fillberry(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, GRAPH_BG_COLOUR);
	CAT_CSCLIP_set_rect(GRAPH_X, GRAPH_Y, GRAPH_MAX_X, GRAPH_MAX_Y);
	for(int i = 0; i < GRAPH_SAMPLE_COUNT-1; i++)
	{
		int x0 = graph_transform_x(i);
		int y0 = graph_transform_y(graph.values[i]);
		int x1 = graph_transform_x(i+1);
		int y1 = graph_transform_y(graph.values[i+1]);
		if(CAT_CSCLIP(&x0, &y0, &x1, &y1))
			CAT_lineberry(x0, y0, x1, y1, GRAPH_FG_COLOUR);
	}
	CAT_circberry(graph_transform_x(graph.target), graph_transform_y(graph.values[graph.target]), 2, CAT_RED);

	CAT_strokeberry(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, CAT_WHITE);
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(GRAPH_X, GRAPH_MAX_Y+3, "(%d, %d) at %dx", graph.center_x, graph.center_y, graph.pps);

	if(graph_current_cell_idx != -1)
	{	
		/*int16_t current_value = graph_make_value(&graph_current_cell);
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_text(GRAPH_X, GRAPH_MAX_Y+17, graph_make_value_text(current_value));
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(GRAPH_X, GRAPH_MAX_Y+17+17, "%d:%d", graph_current_timestamp.tm_hour, graph_current_timestamp.tm_min);*/
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_textf(GRAPH_X, GRAPH_MAX_Y+17, "%d", graph_current_cell_idx);
	}
	else
	{
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_text(GRAPH_X, GRAPH_MAX_Y+17, "N/A");
	}
}

void CAT_monitor_MS_graph(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			focused = false;

			graph_update();
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(focused)
			{
				if(CAT_input_pressed(CAT_BUTTON_START))
					focused = false;

				if(CAT_input_touch_rect(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT))
				{
					graph_view = (graph_view+1) % GRAPH_VIEW_MAX;
					graph_update();
				}

				if(CAT_input_pressed(CAT_BUTTON_A))
					graph.pps += 1;
				if(CAT_input_pressed(CAT_BUTTON_B))
					graph.pps -= 1;
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
					graph_current_cell_idx = graph_indices[graph.target];
					if(graph_current_cell_idx != -1)
					{
						/*CAT_read_log_cell_at_idx(graph_current_cell_idx, &graph_current_cell);
						gmtime_r(&graph_current_cell.timestamp, &graph_current_timestamp);*/
					}
				}
			}
			else
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
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}