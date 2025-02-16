#include "cat_menu.h"

#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include <stdio.h>
#include "cat_deco.h"
#include "cat_actions.h"
#include "cat_bag.h"
#include "cat_render.h"
#include <math.h>

#define GRAPH_PAD 6
#define GRAPH_MARGIN 2
#define GRAPH_WIDTH (LCD_SCREEN_W-(GRAPH_PAD*2)-(GRAPH_MARGIN*2))
#define GRAPH_HEIGHT 120

static enum 
{
	CO2,
	PM
} mode = CO2;
uint16_t samples[GRAPH_WIDTH];
int cursor_left = GRAPH_WIDTH * 0.25;
int cursor_right = GRAPH_WIDTH * 0.75;

float shaped_sine(float low, float high, float period, float t)
{
	return
	(0.5 * (high - low)) *
	(sin(t * 6.28318530718 * period) + 1) +
	low;
}

void fill_samples()
{
	if(mode == CO2)
	{
		float low = GRAPH_HEIGHT * 0.65;
		float high = GRAPH_HEIGHT * 0.95;
		float period = 3;
		for(int i = 0; i < GRAPH_WIDTH; i++)
		{
			float t = (float) i / (float) GRAPH_WIDTH;
			float sin_a = shaped_sine(low * 0.5, high, period, t);
			float sin_b = shaped_sine(low, high * 0.5, period + 4, t * 1.5);
			float noise = CAT_rand_float(-5, 5);
			samples[i] = clamp(0.5 * (sin_a + sin_b) + noise, 0, GRAPH_HEIGHT);
		}
	}
	else
	{
		float low = GRAPH_HEIGHT * 0.65;
		float high = GRAPH_HEIGHT * 0.95;
		float period = 2;
		for(int i = 0; i < GRAPH_WIDTH; i++)
		{
			float t = (float) i / (float) GRAPH_WIDTH;
			float x = shaped_sine(low, high, period, t);
			samples[i] = clamp(x, 0, GRAPH_HEIGHT);
		}
	}
}

void CAT_MS_graph_spoof(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{	
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			
			if(CAT_input_pressed(CAT_BUTTON_SELECT))
			{
				if(mode == CO2)
					mode = PM;
				else
					mode = CO2;
				fill_samples();
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_graph_spoof()
{
	CAT_gui_title(false, NULL, &icon_exit_sprite, "GRAPH");
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_text(mode == CO2 ? "CO2:" : "PM10:");
	CAT_gui_image(&icon_n_sprite, 1);
	CAT_gui_textf("%d/%d/%d", 2, 15, 2025);
	CAT_gui_image(&icon_s_sprite, 1);
	CAT_gui_line_break();

	int x_off = GRAPH_MARGIN;
	int y_off = gui.cursor.y;
	CAT_fillberry(x_off, y_off, GRAPH_WIDTH+GRAPH_MARGIN+GRAPH_MARGIN, GRAPH_MARGIN, 0);
	CAT_fillberry(x_off, y_off, GRAPH_MARGIN, GRAPH_HEIGHT+GRAPH_MARGIN+GRAPH_MARGIN, 0);
	CAT_fillberry(x_off+GRAPH_WIDTH+GRAPH_MARGIN, y_off, GRAPH_MARGIN, GRAPH_HEIGHT+GRAPH_MARGIN+GRAPH_MARGIN, 0);
	CAT_fillberry(x_off, y_off+GRAPH_MARGIN+GRAPH_HEIGHT, GRAPH_WIDTH+GRAPH_MARGIN+GRAPH_MARGIN, GRAPH_MARGIN, 0);

	x_off += GRAPH_MARGIN;
	y_off += GRAPH_MARGIN;
	for(int x = 0; x < GRAPH_WIDTH-1; x++)
	{	
		int v0 = samples[x];
		int v1 = samples[x+1];
		if(v0 >= 0 && v1 >= 0)
		{
			int y0 = y_off+GRAPH_HEIGHT-v0;
			int y1 = y_off+GRAPH_HEIGHT-v1;
			CAT_lineberry(x_off+x, y0, x_off+x+1, y1, 0x001F);
		}
	}

	uint16_t red = 0b1111100000000000;
	uint16_t green = 0b0000011111100000;
	CAT_lineberry(x_off+cursor_left, y_off, x_off+cursor_left, y_off+GRAPH_HEIGHT-1, green);
	CAT_lineberry(x_off+cursor_right, y_off, x_off+cursor_right, y_off+GRAPH_HEIGHT-1, red);
	
	gui.cursor.y += GRAPH_HEIGHT+GRAPH_PAD*2+GRAPH_MARGIN*2;

	const char* unit = (mode == CO2) ? "PPM" : "#/cm\5";
	CAT_gui_textf("Start: %d%s \2 %2d:%02d:%02d", samples[cursor_left], unit, 10, 43, 26);
	CAT_gui_line_break();
	CAT_gui_textf("End: %d%s \2 %2d:%02d:%02d", samples[cursor_right], unit, 12, 36, 14);
	CAT_gui_line_break();

	CAT_gui_image(&icon_a_sprite, 1);
	CAT_gui_text(" to start over");
	CAT_gui_line_break();
}
