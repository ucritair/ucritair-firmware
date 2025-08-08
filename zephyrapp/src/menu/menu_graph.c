#include "menu_graph.h"

#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include <stdio.h>
#include "cat_deco.h"
#include "cat_item.h"
#include "rtc.h"
#include "flash.h"
#include "sprite_assets.h"

#include "menu_aqi.h"
#include "menu_graph_rendering.c"

#define M_E         2.7182818284590452354


#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(menu_graph, LOG_LEVEL_DBG);

uint64_t graph_end_time;
int graph_step_time;
bool needs_update;

int16_t graph_data[GRAPH_W];
int graph_indicies[GRAPH_W];
int graph_max;

enum view_opt {
	CO2 = 0,
	PN10_0,
	PM2_5,
	TEMP,
	RH,
	PRESS,
	VOC,
	NOX,
	VIEW_OPT_END
} viewing = CO2;

int16_t get_graph_data(CAT_log_cell* cell)
{
	switch (viewing)
	{
#define NEG1_IF_NOT_FLAG(x, flag) ((cell->flags&flag)?x:-1)
		case CO2:
			return NEG1_IF_NOT_FLAG(cell->co2_ppmx1, CAT_LOG_CELL_FLAG_HAS_CO2);
		case PM2_5:
			return NEG1_IF_NOT_FLAG(cell->pm_ugmx100[1], CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case PN10_0:
			return NEG1_IF_NOT_FLAG(cell->pn_ugmx100[4], CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case TEMP:
			return NEG1_IF_NOT_FLAG(cell->temp_Cx1000/10, CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
		case RH:
			return NEG1_IF_NOT_FLAG(cell->rh_pctx100, CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES);
#define NEG1_IF_0(x) (x==0?-1:x)
		case PRESS:
			return NEG1_IF_0(cell->pressure_hPax10);
		case VOC:
			return NEG1_IF_0(cell->voc_index);
		case NOX:
			return NEG1_IF_0(cell->nox_index);
		default:
			return 0;
	}
}

enum ach_mode {
	NONE,
	ACH,
	EACH
} get_ach_mode()
{
	switch (viewing)
	{
	case CO2:
		return ACH;
	case PN10_0:
		return EACH;
	default:
		return NONE;
	}
}

char* get_string(int16_t graph_data)
{
	static char buf[32];

	switch (viewing)
	{
		case CO2:
			snprintf(buf, sizeof(buf), "%dppm", graph_data);
			break;
		case PM2_5:
			snprintf(buf, sizeof(buf), "%.1f\4g/m\5", ((double)graph_data)/100.);
			break;
		case PN10_0:
			snprintf(buf, sizeof(buf), "%.1f#/cm\5", ((double)graph_data)/100.);
			break;
		case TEMP:
			snprintf(buf, sizeof(buf), "%.1f%s", CAT_AQ_map_celsius(((double)graph_data)/100.), CAT_AQ_get_temperature_unit_string());
			break;
		case RH:
			snprintf(buf, sizeof(buf), "%.1f%%RH", ((double)graph_data)/100.);
			break;
		case PRESS:
			snprintf(buf, sizeof(buf), "%.1fhPa", ((double)graph_data)/10.);
			break;
		case VOC:
		case NOX:
			snprintf(buf, sizeof(buf), "%d", graph_data);
			break;
		default:
			snprintf(buf, sizeof(buf), "???");
	}

	return buf;
}

char* get_name()
{
	switch (viewing)
	{
	case CO2: return "CO2";
	case PM2_5: return "PM2.5";
	case PN10_0: return "PN10";
	case TEMP: return "TEMPERATURE";
	case RH: return "HUMIDITY";
	case PRESS: return "PRESSURE";
	case VOC: return "VOC INDEX";
	case NOX: return "NOX INDEX";
	}
}

void update_graph()
{
	uint64_t time = graph_end_time;
	LOG_DBG("Get data");

	int memo = -1;

	for(int x = GRAPH_W-1; x >= 0; x--)
	{
		CAT_log_cell cell;
		memo = flash_get_first_cell_before_time(memo, time, &cell);
		if (memo > 0)
		{
			graph_data[x] = get_graph_data(&cell);
			if (graph_data[x] != -1)
				graph_indicies[x] = memo;
		}
		else
		{
			graph_data[x] = -1;
			graph_indicies[x] = -1;
		}

		time -= graph_step_time;
	}

	LOG_DBG("Find max");

	int max = 0;
	for (int x = 0; x < GRAPH_W; x++)
	{
		if (graph_data[x] > max)
			max = graph_data[x];
	}

	if (max<2) max = 2;

	float scale = ((float)GRAPH_H)/((float)max);
	scale *= 0.95; // don't completely fill

	LOG_DBG("Scale graph");

	for (int x = 0; x < GRAPH_W; x++)
	{
		if (graph_data[x] != -1)
			graph_data[x] = ((float)graph_data[x])*scale;
	}

	graph_max = max;
}

int cursor_end = GRAPH_W-1;
int cursor_start = GRAPH_W-50;
enum sel_state {
	SEL_START = 0,
	SEL_END,
	SEL_DONE
} cursor_state = SEL_START;

int cursor_velocity = 1;

void calc_ach();

#define STEPH(x) ((3600.*x)/(float)GRAPH_W)

int step_times[] = {STEPH(24), STEPH(12.05), STEPH(6), STEPH(3), STEPH(1), STEPH(0.5), STEPH(0.25), STEPH(0.125)};
int step_time_index = 0;

int day_scroll_accel = 1;

void CAT_MS_graph(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_graph);
			graph_end_time = get_current_rtc_time();
			step_time_index = 0;
			graph_step_time = step_times[step_time_index];
			cursor_state = SEL_START;
			viewing = 0;
			cursor_end = GRAPH_W-1;
			update_graph();
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				if (cursor_state == SEL_START)
				{
					cursor_state = SEL_END;
					cursor_end = cursor_start + 20;
				}
				else if (cursor_state == SEL_END)
				{
					cursor_state = SEL_DONE;

					if (get_ach_mode() != NONE)
					{
						calc_ach();
					}
				}
				else
				{
					cursor_state = SEL_START;
					cursor_end = GRAPH_W-1;
				}
			}

			if(CAT_input_held(CAT_BUTTON_UP, 0.2) || CAT_input_pressed(CAT_BUTTON_UP))
			{
				graph_end_time += 60*60*24 * MAX(1, day_scroll_accel-10);
				graph_end_time = MIN(graph_end_time, get_current_rtc_time() - 1);
				day_scroll_accel += 1;
			}

			if (CAT_input_held(CAT_BUTTON_DOWN, 0.2) || CAT_input_pressed(CAT_BUTTON_DOWN))
			{
				graph_end_time -= 60*60*24 * MAX(1, day_scroll_accel-10);
				day_scroll_accel += 1;
			}

			if (CAT_input_released(CAT_BUTTON_UP) || CAT_input_released(CAT_BUTTON_DOWN))
			{
				update_graph();
				day_scroll_accel = 1;
			}

			if (CAT_input_pressed(CAT_BUTTON_SELECT) && cursor_state == SEL_START)
			{
				step_time_index++;
				if (step_time_index==((sizeof(step_times)/sizeof(step_times[0]))))
				{
					step_time_index=0;
				}
				graph_step_time=step_times[step_time_index];
				update_graph();
			}

			if (CAT_input_pressed(CAT_BUTTON_START && cursor_state == SEL_START))
			{
				viewing++;
				if (viewing == VIEW_OPT_END)
					viewing = 0;
				update_graph();
			}

			if (cursor_state!=SEL_DONE)
			{
				int* sel_cursor = cursor_state==SEL_END?&cursor_end:&cursor_start;

				if(CAT_input_held(CAT_BUTTON_LEFT, 0.05))
				{
					*sel_cursor -= MAX(1, cursor_velocity-10);
					cursor_velocity += 1;
				}
				else if(CAT_input_held(CAT_BUTTON_RIGHT, 0.05))
				{
					*sel_cursor += MAX(1, cursor_velocity-10);
					cursor_velocity += 1;
				}
				else
				{
					cursor_velocity = 1;
				}

				if (*sel_cursor < 0)
				{
					graph_end_time -= graph_step_time * -*sel_cursor;
					*sel_cursor = 0;
					update_graph();
				}

				if (*sel_cursor > GRAPH_W-1)
				{
					graph_end_time += graph_step_time * (*sel_cursor - (GRAPH_W-1));
					graph_end_time = MIN(graph_end_time, get_current_rtc_time() - 1);
					*sel_cursor = GRAPH_W-1;
					update_graph();
				}
			}

			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}


void CAT_do_render_graph(int16_t* data, int max, int x, int y, int cursor_start, int cursor_end);

struct dp {
	uint64_t time;
	int ppm;
};

float ach_cache = -1;
void calc_ach()
{
	struct dp max = {0};
	struct dp min = {.ppm=99999};

	int o_cursor_end = cursor_end;

	for (int cur = cursor_start; cur <= o_cursor_end; cur++)
	{
		CAT_log_cell cell;
		int nr = graph_indicies[cur];
		if (nr != -1)
			flash_get_cell_by_nr(nr, &cell);
		else
			continue;

		struct dp here = {.time = cell.timestamp, .ppm = get_graph_data(&cell)};

		if (here.ppm > max.ppm)
		{
			max = here;
			cursor_start = cur;
		}
		if (here.ppm < min.ppm)
		{
			min = here;
			cursor_end = cur;
		}
	}

	LOG_DBG("max.time = %lld, max.ppm = %d", max.time, max.ppm);
	LOG_DBG("min.time = %lld, min.ppm = %d", min.time, min.ppm);

	double decay_concentration = (double)min.ppm + ((double)max.ppm - (double)min.ppm) / M_E;

	LOG_DBG("decay_concentration = %.2f", decay_concentration);
    
    // Decay time (Assume for simplicity it's between max and baseline times)
   // <A FUNCTION THAT GIVES ME THE TIME that the concentration dropped below decay_concentration and returns decay_time)

	uint64_t decay_time = 0;
	for (int cur = cursor_start; cur <= cursor_end; cur++)
	{
		CAT_log_cell cell;
		int nr = graph_indicies[cur];
		if (nr != -1)
			flash_get_cell_by_nr(nr, &cell);
		else
			continue;

		if (get_graph_data(&cell) < decay_concentration)
		{
			decay_time = cell.timestamp;
			break;
		}
	}

	LOG_DBG("deacy_time = %lld", decay_time);

	if (decay_time == 0 || decay_time==max.time || max.ppm == min.ppm || max.ppm == 0 || min.ppm == 99999)
	{
		LOG_WRN("ACH->-1");
		ach_cache = -1;
		return;
	}

    // Calculate ACH
    ach_cache = 1.0 / ((double)(decay_time - max.time) / 3600.0);  // Convert to hours
    LOG_DBG("ach = %.2f", ach_cache);
}

int text_cursor(char* name, int index)
{
	CAT_gui_text(name);
	// CAT_gui_line_break();

	CAT_log_cell cell = {0};
	int nr = graph_indicies[index];
	if (nr != -1)
		flash_get_cell_by_nr(nr, &cell);

	struct tm t;
	uint64_t ts = cell.timestamp;
	gmtime_r(&ts, &t);

	int gd = get_graph_data(&cell);

	CAT_gui_textf("%s \2 %2d:%02d:%02d", gd==-1?"?":get_string(gd), t.tm_hour, t.tm_min, t.tm_sec);

	return gd;
}

void CAT_render_graph()
{
	CAT_gui_title(false, "GRAPH");
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	struct tm t;
	gmtime_r(&graph_end_time, &t); 

	CAT_gui_textf("%s: ", get_name());
	CAT_gui_image(&null_sprite, 1);
	CAT_gui_textf("%d/%d/%d", t.tm_mon+1, t.tm_mday, t.tm_year);
	CAT_gui_image(&null_sprite, 1);
	CAT_gui_line_break();

	CAT_do_render_graph(graph_data, graph_max, GRAPH_PAD, gui.cursor.y, cursor_start, cursor_state>SEL_START?cursor_end:-1);

	gui.cursor.y += GRAPH_H+GRAPH_PAD*2+GRAPH_MARGIN*2;

	if (cursor_state == SEL_START) CAT_gui_image(&null_sprite, 1);
	text_cursor("Start:", cursor_start);
	if (cursor_state == SEL_START) CAT_gui_image(&null_sprite, 1);
	CAT_gui_line_break();

	int end_val = 0;

	if (cursor_state == SEL_START)
	{
		CAT_gui_image(&null_sprite, 1);
		CAT_gui_text("to select start");
	}

	if (cursor_state > SEL_START)
	{
		if (cursor_state == SEL_END) CAT_gui_image(&null_sprite, 1);
		end_val = text_cursor("End  :", cursor_end);
		if (cursor_state == SEL_END) CAT_gui_image(&null_sprite, 1);
		CAT_gui_line_break();
	}

	if (cursor_state == SEL_END)
	{
		CAT_gui_image(&null_sprite, 1);
		CAT_gui_text("to select end");
	}

	if (cursor_state == SEL_DONE)
	{
		if (get_ach_mode() == NONE)
		{
			// does not produce an [e]ACH
			CAT_gui_text("(This does not produce e/ACH)");
		}
		else
		{
			if (ach_cache != -1)
			{
				CAT_gui_textf("%s: %.1f", (get_ach_mode()==EACH)?"eACH":"ACH", (double)ach_cache);
			}
			else
			{
				CAT_gui_text("Calculation failed");
			}
		}
		CAT_gui_line_break();
		CAT_gui_image(&null_sprite, 1);
		CAT_gui_text("to start over");

		if (get_ach_mode() == ACH && end_val > 460)
		{
			CAT_gui_line_break();
			CAT_gui_text("WARNING: End CO2 value high");
			CAT_gui_line_break();
			CAT_gui_text("ACH may be inaccurate!");
		}
	}

	if (cursor_state == SEL_START)
	{
		CAT_gui_line_break();
		CAT_gui_image(&null_sprite, 1);
		CAT_gui_text("to change scale");
		CAT_gui_line_break();
		CAT_gui_textf("(Currently %.1fh wide)", (double)(GRAPH_W*graph_step_time)/3600.);
		CAT_gui_line_break();
		CAT_gui_image(&null_sprite, 1);
		CAT_gui_text("to change parameter");
	}

	if (viewing == PRESS || viewing == NOX || viewing == VOC)
	{
		CAT_gui_line_break();
		CAT_gui_text("(This data is not always\nlogged.)");
	}
}
