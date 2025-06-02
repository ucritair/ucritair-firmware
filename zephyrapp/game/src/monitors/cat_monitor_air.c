#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include "cat_aqi.h"
#include "sprite_assets.h"
#include <math.h>
#include <stdio.h>
#include <time.h>

enum
{
	SUMMARY,
	GRAPH,
	CLOCK,
	PAGE_MAX
};
static int page = SUMMARY;
static bool focused = false;

void summary_tick()
{
	focused = false;
}

static void render_page_markers(int x, int y)
{
	int start_x = x - ((16 + 2) * PAGE_MAX) / 2;
	for(int i = 0; i < PAGE_MAX; i++)
	{
		int x = start_x + i * (16 + 2);
		CAT_draw_sprite(&ui_radio_button_diamond_sprite, page == i, x, y);
	}
}

static void render_summary()
{
	int cursor_y = 32;	
	CAT_push_text_scale(2);
	CAT_push_text_colour(CAT_WHITE);
	CAT_draw_text(12, cursor_y, "Air Quality");
	cursor_y += 48;

	if(!CAT_is_AQ_initialized())
	{
		CAT_push_text_flags(CAT_TEXT_FLAG_WRAP);
		CAT_push_text_line_width(CAT_LCD_SCREEN_W-24);
		CAT_push_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_text(12, cursor_y, "Air quality sensors are coming online. Please wait...");
	}
	else
	{
		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "CO2  %.0f ppm", readings.sunrise.ppm_filtered_compensated);
		cursor_y += 14;

		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "PM2.5  %.0f \4g/m\5", readings.sen5x.pm2_5);
		cursor_y += 14;

		if (readings.sen5x.nox_index && readings.sen5x.voc_index)
		{
			CAT_push_text_colour(CAT_WHITE);
			CAT_draw_textf(12, cursor_y, "NOX %.0f / VOC %.0f", readings.sen5x.nox_index, readings.sen5x.voc_index);
			cursor_y += 14;
		}

		float deg_c = readings.sen5x.temp_degC;
		float deg_mapped = CAT_AQ_map_celsius(deg_c);
		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "%.0f %s / %.0f%% RH", deg_mapped, CAT_AQ_get_temperature_unit_string(), readings.sen5x.humidity_rhpct);
		cursor_y += 28;

		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "%.1f%% rebreathed", ((((double) readings.sunrise.ppm_filtered_compensated)-420.)/38000.)*100.);
		cursor_y += 14;

		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "uCritAQI %.1f%%", CAT_AQI_aggregate());
		cursor_y += 14;
		
		CAT_datetime t;
		CAT_get_datetime(&t);
		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "at %2d:%02d:%02d", t.hour, t.minute, t.second);
		cursor_y += 14;
	}
}

#define GRAPH_X 12
#define GRAPH_Y 42
#define GRAPH_WIDTH (CAT_LCD_SCREEN_W - 2 * GRAPH_X)
#define GRAPH_HEIGHT 128
#define GRAPH_MAX_X (GRAPH_X + GRAPH_WIDTH - 1)
#define GRAPH_MAX_Y (GRAPH_Y + GRAPH_HEIGHT - 1)
#define GRAPH_SAMPLE_COUNT GRAPH_WIDTH

#define GRAPH_TABS_X GRAPH_X
#define GRAPH_TABS_Y (GRAPH_MAX_Y + 4)
#define GRAPH_TAB_H 16
#define GRAPH_TAB_MARGIN 2
#define GRAPH_TAB_W ((GRAPH_WIDTH - GRAPH_TAB_MARGIN * GRAPH_VIEW_MAX) / GRAPH_VIEW_MAX)
#define GRAPH_TABS_MAX_Y (GRAPH_TABS_Y + GRAPH_TAB_H)


#define GRAPH_BG_COLOUR 0xe7bf
#define GRAPH_FG_COLOUR 0x6b11
#define GRAPH_RETICLE_COLOUR 0xadfa

static struct
{
	int16_t values[GRAPH_SAMPLE_COUNT];
	int16_t value_max;

	int center_x;
	int center_y;
	int pps;

	int marks[2];
} graph;

static bool graph_show_reticle = false;
static int graph_mark_idx = 1;

static uint64_t graph_end_time;
static int graph_time_step;
static bool graph_needs_update;

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

int16_t graph_get_value(CAT_log_cell* cell)
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

static enum
{
	NONE,
	ACH,
	EACH
} graph_get_ACH_mode()
{
	switch(graph_view)
	{
		case CO2:
			return ACH;
		case PN_10_0:
			return EACH;
		default:
			return NONE;
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

static char* graph_get_text(int16_t value)
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
	uint64_t time = graph_end_time;
	int memo = -1;
	graph.value_max = INT_FAST16_MIN;

	for(int i = GRAPH_SAMPLE_COUNT-1; i >= 0; i--)
	{
		CAT_log_cell cell;
		memo = CAT_read_log_cell_before_time(memo, time, &cell);
		if(memo > 0)
		{
			graph.values[i] = graph_get_value(&cell);
			graph.value_max = max(graph.value_max, graph.values[i]);
			if(graph.values[i] != -1)
				graph_indices[i] = memo;
		}
		else
		{
			graph.values[i] = -1;
			graph_indices[i] = -1;
		}

		time -= graph_time_step;
	}
}

static void init_graph()
{
	memset(graph.values, 0, sizeof(graph.values));
	graph.value_max = 0;

	graph.center_x = 0;
	graph.center_y = 0;
	graph.pps = 1;

	graph.marks[0] = 0;
	graph.marks[1] = 0;

	for(int i = 0; i < GRAPH_SAMPLE_COUNT; i++)
		graph_indices[i] = -1;
	graph_end_time = CAT_get_rtc_now();
	graph_time_step = 385;

	graph_update();
}

static void graph_tick()
{
	if(focused)
	{
		int dx = 0;
		int dy = 0;
		if(CAT_input_held(CAT_BUTTON_LEFT, 0))
			dx -= 2;
		if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
			dx += 2;
		if(CAT_input_held(CAT_BUTTON_UP, 0))
			dy += 2;
		if(CAT_input_held(CAT_BUTTON_DOWN, 0))
			dy -= 2;
		graph.center_x += dx;
		graph.center_y += dy;

		/*if(dx != 0)
		{
			graph_end_time += dx * graph_time_step;
			graph_end_time = min(graph_end_time, CAT_get_rtc_now());
			graph_update();
		}*/

		if(CAT_input_released(CAT_BUTTON_START))
			graph.pps += 1;
		if(CAT_input_released(CAT_BUTTON_SELECT))
			graph.pps -= 1;
		graph.pps = clamp(graph.pps, 1, 16);

		if(CAT_input_pressed(CAT_BUTTON_START) && CAT_input_pressed(CAT_BUTTON_SELECT))
			graph_show_reticle = !graph_show_reticle;
	}

	if(CAT_input_touching())
	{
		for(int i = 0; i < GRAPH_VIEW_MAX; i++)
		{
		
			int x = GRAPH_TABS_X + (GRAPH_TAB_W + GRAPH_TAB_MARGIN) * i;
			int y = GRAPH_TABS_Y;
			if(CAT_input_touch_rect(x, y, GRAPH_TAB_W, GRAPH_TAB_H))
			{
				graph_view = i;
				graph_update();
			}
		}
	}

	gmtime_r(&graph_end_time, &graph_current_timestamp);

	if(graph.center_x >= 0 && graph.center_x < GRAPH_SAMPLE_COUNT)
	{
		graph_current_cell_idx = graph_indices[graph.center_x];
		if(graph_current_cell_idx != -1)
			CAT_read_log_cell_at_idx(graph_current_cell_idx, &graph_current_cell);
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
	y -= graph.center_y;
	y *= graph.pps;
	
	y += GRAPH_HEIGHT / 2;
	y = GRAPH_MAX_Y - y;
	return y;
}

static int graph_sample_x(int i)
{
	int x = i;
	return graph_transform_x(x);
}

static int graph_sample_y(int i)
{
	int y = graph.values[i];
	float t = inv_lerp(y, 0, graph.value_max);
	y = t * GRAPH_HEIGHT;
	return graph_transform_y(y);
}

static void render_graph()
{
	CAT_push_text_colour(CAT_WHITE);
	CAT_draw_textf(12, GRAPH_Y-14, "%s", graph_get_title());
	CAT_push_text_colour(CAT_WHITE);
	CAT_draw_textf(GRAPH_MAX_X - 8 * CAT_GLYPH_WIDTH, GRAPH_Y-14, "%d/%d/%d", graph_current_timestamp.tm_mon+1, graph_current_timestamp.tm_mday, graph_current_timestamp.tm_year);

	CAT_fillberry(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, GRAPH_BG_COLOUR);
	CAT_CSCLIP_set_rect(GRAPH_X, GRAPH_Y, GRAPH_MAX_X, GRAPH_MAX_Y);
	for(int i = 0; i < GRAPH_SAMPLE_COUNT-1; i++)
	{
		int x0 = graph_sample_x(i);
		int y0 = graph_sample_y(i);
		int x1 = graph_sample_x(i+1);
		int y1 = graph_sample_y(i+1);
		if(CAT_CSCLIP(&x0, &y0, &x1, &y1))
			CAT_lineberry(x0, y0, x1, y1, GRAPH_FG_COLOUR);
	}
	for(int i = 0; i < 2; i++)
	{
		int x = graph_transform_x(graph.marks[i]);
		if(x >= GRAPH_X && x <= GRAPH_MAX_X)
			CAT_lineberry(x, GRAPH_Y+1, x, GRAPH_MAX_Y, i == 0 ? CAT_GREEN : CAT_RED);
	}

	if(graph_show_reticle)
	{
		int c_x = GRAPH_X + GRAPH_WIDTH / 2;
		int c_y = GRAPH_Y + GRAPH_HEIGHT / 2;
		CAT_strokeberry(c_x-graph.pps, c_y-graph.pps, graph.pps*2+1, graph.pps*2+1, GRAPH_RETICLE_COLOUR);
		CAT_lineberry(GRAPH_X+1, c_y, c_x-graph.pps+1, c_y, GRAPH_RETICLE_COLOUR);
		CAT_lineberry(c_x+graph.pps, c_y, GRAPH_MAX_X, c_y, GRAPH_RETICLE_COLOUR);
		CAT_lineberry(c_x, GRAPH_Y+1, c_x, c_y-graph.pps, GRAPH_RETICLE_COLOUR);
		CAT_lineberry(c_x, c_y+graph.pps, c_x, GRAPH_MAX_Y, GRAPH_RETICLE_COLOUR);
	}
	CAT_strokeberry(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, CAT_WHITE);

	for(int i = 0; i < GRAPH_VIEW_MAX; i++)
	{
		int x = GRAPH_TABS_X + (GRAPH_TAB_W + GRAPH_TAB_MARGIN) * i;
		int y = GRAPH_TABS_Y;
		CAT_strokeberry(x, y, GRAPH_TAB_W, GRAPH_TAB_H, CAT_WHITE);
	}

	CAT_push_text_colour(CAT_WHITE);
	CAT_draw_textf(GRAPH_X, GRAPH_TABS_MAX_Y+3, "(%d, %d) at %dx", graph.center_x, graph.center_y, graph.pps);
	CAT_push_text_colour(CAT_WHITE);

	if(graph_current_cell_idx != -1)
	{	
		int16_t current_value = graph_get_value(&graph_current_cell);
		CAT_draw_text(GRAPH_X, GRAPH_TABS_MAX_Y+17, graph_get_text(current_value));
	}
	else
	{
		CAT_draw_text(GRAPH_X, GRAPH_TABS_MAX_Y+17, "N/A");
	}
}

#define CLOCK_X 120
#define CLOCK_Y 132
#define CLOCK_RADIUS 96
#define CLOCK_HOUR_HAND_LENGTH (CLOCK_RADIUS * 0.6f)
#define CLOCK_MINUTE_HAND_LENGTH (CLOCK_RADIUS * 0.75f)
#define CLOCK_SECOND_HAND_LENGTH (CLOCK_RADIUS * 0.9f)

struct
{
	CAT_datetime datetime;
	float hour_arc;
	float minute_arc;
	float second_arc;
} monitor_clock;

static void clock_tick()
{
	focused = false;

	CAT_get_datetime(&monitor_clock.datetime);

	float arc_base = M_PI * 0.5f;

	float hour_t = monitor_clock.datetime.hour / 24.0f;
	float min_t = monitor_clock.datetime.minute / 60.0f;
	float sec_t = monitor_clock.datetime.second / 60.0f;

	monitor_clock.hour_arc = arc_base - hour_t * (4 * M_PI);
	monitor_clock.minute_arc = arc_base - min_t * (2 * M_PI);
	monitor_clock.second_arc = arc_base - sec_t * (2 * M_PI);
}

CAT_vec2 clock_point(float t, float r)
{
	float dx = cos(t) * r;
	float dy = sin(t) * r;
	float x = CLOCK_X + dx;
	float y = CLOCK_Y - dy;
	return (CAT_vec2) {x, y};
}

static void render_clock()
{
	CAT_push_text_colour(CAT_WHITE);
	CAT_draw_textf(12, 24, "%d:%d\n%ds", monitor_clock.datetime.hour, monitor_clock.datetime.minute, monitor_clock.datetime.second);

	CAT_circberry(120, 132, 96, CAT_WHITE);

	CAT_vec2 hour_point = clock_point(monitor_clock.hour_arc, CLOCK_HOUR_HAND_LENGTH);
	CAT_lineberry(CLOCK_X, CLOCK_Y, hour_point.x, hour_point.y, CAT_WHITE);

	CAT_vec2 minute_point = clock_point(monitor_clock.minute_arc, CLOCK_MINUTE_HAND_LENGTH);
	CAT_lineberry(CLOCK_X, CLOCK_Y, minute_point.x, minute_point.y, CAT_WHITE);

	CAT_vec2 second_point = clock_point(monitor_clock.second_arc, CLOCK_SECOND_HAND_LENGTH);
	CAT_lineberry(CLOCK_X, CLOCK_Y, second_point.x, second_point.y, CAT_WHITE);
}

static void render_monitor_air()
{
	CAT_frameberry(RGB8882565(35, 157, 235));
	render_page_markers(120, 8);

	CAT_push_draw_flags(CAT_DRAW_FLAG_BOTTOM);
	CAT_draw_sprite(&monitor_clouds_sprite, 0, 0, 320);

	CAT_push_text_colour(RGB8882565(35, 157, 235));
	CAT_draw_text(64, 304, "[START] to enter game");

	switch (page)
	{
		case SUMMARY:
			render_summary();
		break;

		case GRAPH:
			render_graph();
		break;

		case CLOCK:
			render_clock();
		break;
	}
}

void CAT_MS_monitor_air(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(render_monitor_air);
			init_graph();
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(!focused)
			{
				if(CAT_input_pressed(CAT_BUTTON_START))
					CAT_machine_transition(CAT_MS_room);

				if (CAT_input_pressed(CAT_BUTTON_RIGHT))
					page += 1;
				if (CAT_input_pressed(CAT_BUTTON_LEFT))
					page -= 1;
				page = (page + PAGE_MAX) % PAGE_MAX;

				if(CAT_input_pressed(CAT_BUTTON_A))
					focused = true;
			}
			else
			{
				if(CAT_input_released(CAT_BUTTON_B))
					focused = false;
			}

			switch (page)
			{
				case SUMMARY:
					summary_tick();
				break;

				case GRAPH:
					graph_tick();
				break;

				case CLOCK:
					clock_tick();
				break;
			}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}