#include "menu_graph.h"
#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include <stdio.h>
#include "cat_deco.h"
#include "cat_actions.h"
#include "cat_bag.h"
#include "rtc.h"
#include "airquality.h"
#include "flash.h"

#include "menu_aqi.h"

#define M_E         2.7182818284590452354


#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(menu_graph, LOG_LEVEL_DBG);

uint64_t graph_end_time;
uint64_t graph_start_time;
int graph_step_time;
bool needs_update;

int16_t graph_data[GRAPH_W];
int graph_indicies[GRAPH_W];
int graph_max;

void update_graph()
{
	uint64_t time = graph_end_time;
	LOG_DBG("Get data");

	int memo = -1;

	for(int x = GRAPH_W-1; x >= 0; x--)
	{
		struct flash_log_cell cell;
		memo = flash_get_first_cell_before_time(memo, time, &cell);
		if (memo > 0)
		{
			graph_data[x] = cell.co2_ppmx1;
			graph_indicies[x] = memo;
		}
		else
		{
			graph_data[x] = -1;
			graph_indicies[x] = -1;
		}

		// graph_data[x] = x;

		time -= graph_step_time;
	}

	LOG_DBG("Find max");

	int max = 0;
	for (int x = 0; x < GRAPH_W; x++)
	{
		if (graph_data[x] > max)
			max = graph_data[x];
	}

	float scale = ((float)GRAPH_H)/((float)max);

	LOG_DBG("Scale graph");

	for (int x = 0; x < GRAPH_W; x++)
	{
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

void calc_ach();

int step_times[] = {3*60, 2*60, 1*60, 30, 15, 10, 5, 10*60, 5*60,};
int step_time_index = 0;

void CAT_MS_graph(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			graph_end_time = get_current_rtc_time();
			graph_step_time = 3*60;
			step_time_index = 0;
			cursor_state = SEL_START;
			cursor_end = GRAPH_W-1;
			update_graph();
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(CAT_MS_aqi);

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
					calc_ach();
				}
				else
				{
					cursor_state = SEL_START;
					cursor_end = GRAPH_W-1;
				}
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

			if (cursor_state!=SEL_DONE)
			{
				int* sel_cursor = cursor_state==SEL_END?&cursor_end:&cursor_start;

				if(CAT_input_pulse(CAT_BUTTON_LEFT))
					(*sel_cursor)--;
				if(CAT_input_pulse(CAT_BUTTON_RIGHT))
					(*sel_cursor)++;

				cursor_end = clamp(cursor_end, 0, GRAPH_W-1);
				cursor_start = clamp(cursor_start, 0, cursor_end);
			}

			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}


void CAT_do_render_graph(uint16_t* data, int max, int x, int y, int cursor_start, int cursor_end);

struct dp {
	uint64_t time;
	int ppm;
};

float ach = -1;
void calc_ach()
{
	struct dp max = {0};
	struct dp min = {.ppm=99999};

	int o_cursor_end = cursor_end;

	for (int cur = cursor_start; cur <= o_cursor_end; cur++)
	{
		struct flash_log_cell cell;
		int nr = graph_indicies[cur];
		if (nr != -1)
			flash_get_cell_by_nr(nr, &cell);
		else
			continue;

		struct dp here = {.time = cell.timestamp, .ppm = cell.co2_ppmx1};

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
		struct flash_log_cell cell;
		int nr = graph_indicies[cur];
		if (nr != -1)
			flash_get_cell_by_nr(nr, &cell);
		else
			continue;

		if (cell.co2_ppmx1 < decay_concentration)
		{
			decay_time = cell.timestamp;
			break;
		}
	}

	LOG_DBG("deacy_time = %lld", decay_time);

	if (decay_time == 0)
	{
		LOG_WRN("ACH->-1");
		ach = -1;
		return;
	}

    // Calculate ACH
    ach = 1.0 / ((double)(decay_time - max.time) / 3600.0);  // Convert to hours
    LOG_DBG("ach = %.2f", ach);
}

void text_cursor(char* name, int index)
{
	CAT_gui_text(name);
	// CAT_gui_line_break();

	struct flash_log_cell cell = {0};
	int nr = graph_indicies[index];
	if (nr != -1)
		flash_get_cell_by_nr(nr, &cell);

	struct tm t;
	uint64_t ts = cell.timestamp;
	gmtime_r(&ts, &t); 

	CAT_gui_textf("%dppm @ %2d:%02d:%02d", cell.co2_ppmx1, t.tm_hour, t.tm_min, t.tm_sec);
}

void CAT_render_graph()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("GRAPH ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_line_break();

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_text("CO2 Graph");
	CAT_gui_line_break();

	CAT_do_render_graph(graph_data, graph_max, GRAPH_PAD, gui.cursor.y, cursor_start, cursor_state>SEL_START?cursor_end:-1);

	gui.cursor.y += GRAPH_H+GRAPH_PAD*2+GRAPH_MARGIN*2;

	text_cursor("Start:", cursor_start);
	CAT_gui_line_break();

	if (cursor_state == SEL_START)
	{
		CAT_gui_image(icon_a_sprite, 1);
		CAT_gui_text("to select start");
	}

	if (cursor_state > SEL_START)
	{
		text_cursor("End  :", cursor_end);
		CAT_gui_line_break();
	}

	if (cursor_state == SEL_END)
	{
		CAT_gui_image(icon_a_sprite, 1);
		CAT_gui_text("to select end");
	}

	if (cursor_state == SEL_DONE)
	{
		CAT_gui_textf("ACH: %.1f", (double)ach);
		CAT_gui_line_break();
		CAT_gui_image(icon_a_sprite, 1);
		CAT_gui_text("to start over");
	}

	if (cursor_state == SEL_START)
	{
		CAT_gui_line_break();
		CAT_gui_image(icon_select_sprite, 1);
		CAT_gui_text("to change scale");
		CAT_gui_line_break();
		CAT_gui_textf("(Currently %.1fh wide)", (double)(GRAPH_W*graph_step_time)/3600.);
	}
}
