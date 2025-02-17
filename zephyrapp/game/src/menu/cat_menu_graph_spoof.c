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
uint16_t starts[] =
{
	GRAPH_WIDTH * 0.15f,
	GRAPH_WIDTH * 0.05f
};
uint16_t ends[] =
{
	(GRAPH_WIDTH-1),
	(GRAPH_WIDTH-1) * 0.9f,
};
uint16_t maxes[] =
{
	1200,
	120
};

float samples[GRAPH_WIDTH];

float shaped_sine(float low, float high, float period, float t)
{
	return
	(0.5 * (high - low)) *
	(sin(t * 6.28318530718 * period) + 1) +
	low;
}

float big_dipper(float low, float high, float t)
{
	return high * (-1.3 * sqrt(t) + 1 + 0.5 * (t * t)) + low;
}

float decay(float low, float high, float r, float t)
{
	return (high-low) * pow(M_E, -r * t) + low;
}

void fill_samples()
{
	if(mode == CO2)
	{
		float start = 0.15;
		float end = 1.0f;
		for(int i = 0; i < GRAPH_WIDTH; i++)
		{
			float t = (float) i / (float) GRAPH_WIDTH;
			if(t < start)
				samples[i] = 880;
			else if(t < end)
			{
				t = (t - start) / (end - start);
				samples[i] = decay(420, 880, 2, t * 3.5);
			}
			else
				samples[i] = 420;
		}
	}
	else
	{
		float start = 0.05;
		float end = 1.0f;
		for(int i = 0; i < GRAPH_WIDTH; i++)
		{
			float t = (float) i / (float) GRAPH_WIDTH;
			if(t < start)
				samples[i] = 100;
			else if(t < end)
			{
				t = (t - start) / (end - start);
				samples[i] = decay(0, 100, 3, t * 2);
			}
			else
				samples[i] = 0;
		}
	}
}

int sample_height(int i)
{
	return (samples[i] / maxes[mode]) * GRAPH_HEIGHT;
}

void CAT_MS_graph_spoof(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{	
			fill_samples();
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

	int x_off = GRAPH_PAD;
	int y_off = gui.cursor.y;
	CAT_fillberry(x_off, y_off, GRAPH_WIDTH+GRAPH_MARGIN+GRAPH_MARGIN, GRAPH_MARGIN, 0);
	CAT_fillberry(x_off, y_off, GRAPH_MARGIN, GRAPH_HEIGHT+GRAPH_MARGIN+GRAPH_MARGIN, 0);
	CAT_fillberry(x_off+GRAPH_WIDTH+GRAPH_MARGIN, y_off, GRAPH_MARGIN, GRAPH_HEIGHT+GRAPH_MARGIN+GRAPH_MARGIN, 0);
	CAT_fillberry(x_off, y_off+GRAPH_MARGIN+GRAPH_HEIGHT, GRAPH_WIDTH+GRAPH_MARGIN+GRAPH_MARGIN, GRAPH_MARGIN, 0);

	x_off += GRAPH_MARGIN;
	y_off += GRAPH_MARGIN;
	for(int x = 0; x < GRAPH_WIDTH-1; x++)
	{	
		int v0 = sample_height(x);
		int v1 = sample_height(x+1);
		if(v0 >= 0 && v1 >= 0)
		{
			int y0 = y_off+GRAPH_HEIGHT-v0;
			int y1 = y_off+GRAPH_HEIGHT-v1;
			CAT_lineberry(x_off+x, y0, x_off+x+1, y1, 0x001F);
		}
	}

	uint16_t red = 0b1111100000000000;
	uint16_t green = 0b0000011111100000;
	CAT_lineberry(x_off+starts[mode], y_off, x_off+starts[mode], y_off+GRAPH_HEIGHT, green);
	CAT_lineberry(x_off+ends[mode], y_off, x_off+ends[mode], y_off+GRAPH_HEIGHT, red);
	
	gui.cursor.y += GRAPH_HEIGHT+GRAPH_PAD*2+GRAPH_MARGIN*2;

	const char* unit = (mode == CO2) ? "PPM" : "#/cm\5";
	CAT_gui_textf("Start: %.0f%s \2 %2d:%02d:%02d", samples[starts[mode]], unit, 10, 43, 26);
	CAT_gui_line_break();
	CAT_gui_textf("End: %.0f%s \2 %2d:%02d:%02d", samples[ends[mode]], unit, 12, 36, 14);
	CAT_gui_line_break();

	CAT_gui_image(&icon_a_sprite, 1);
	CAT_gui_text(" to start over");
	CAT_gui_line_break();
}
