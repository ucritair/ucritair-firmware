#include "cat_menu.h"

#include "cat_input.h"
#include "cat_render.h"
#include "cat_gui.h"
#include <math.h>

#define PALETTE_SIZE 8

static int palette[PALETTE_SIZE+1][3];
static int colour_idx = 0;

static int channel_idx = 0;

static enum
{
	PALETTE,
	CHANNELS
} mode;

void CAT_MS_palette_picker(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_palette_picker);
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_back();

			switch(mode)
			{
				case PALETTE:
				{
					if(CAT_input_pressed(CAT_BUTTON_A))
					{
						mode = CHANNELS;
					}
					if(CAT_input_pressed(CAT_BUTTON_B))
					{
						mode = CHANNELS;
					}

					if(CAT_input_pressed(CAT_BUTTON_UP))
						colour_idx = wrap(colour_idx-1, PALETTE_SIZE+1);
					if(CAT_input_pressed(CAT_BUTTON_DOWN))
						colour_idx = wrap(colour_idx+1, PALETTE_SIZE+1);
				}
				break;

				case CHANNELS:
				{
					if(CAT_input_pressed(CAT_BUTTON_B))
					{
						mode = PALETTE;
					}

					if(CAT_input_pressed(CAT_BUTTON_UP))
						channel_idx = wrap(channel_idx-1, 3);
					if(CAT_input_pressed(CAT_BUTTON_DOWN))
						channel_idx = wrap(channel_idx+1, 3);

					if(CAT_input_held(CAT_BUTTON_LEFT, 0))
						palette[colour_idx][channel_idx] = wrap(palette[colour_idx][channel_idx]-1, 256);
					if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
						palette[colour_idx][channel_idx] = wrap(palette[colour_idx][channel_idx]+1, 256);
				}
				break;
			}
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

#define BOX_SIZE 32
#define X0 ((CAT_LCD_SCREEN_W-BOX_SIZE)/2)
#define Y0 12

void CAT_render_palette_picker()
{
	CAT_frameberry(RGB8882565(palette[PALETTE_SIZE][0], palette[PALETTE_SIZE][1], palette[PALETTE_SIZE][2]));

	switch(mode)
	{
		case PALETTE:
		{
			for(int i = 0; i < PALETTE_SIZE; i++)
			{
				uint16_t c = RGB8882565(palette[i][0], palette[i][1], palette[i][2]);
				CAT_fillberry(X0, Y0 + BOX_SIZE*i, BOX_SIZE, BOX_SIZE, c);
			}
			CAT_strokeberry(X0, Y0 + BOX_SIZE*colour_idx, BOX_SIZE, BOX_SIZE, CAT_WHITE);
		}
		break;

		case CHANNELS:
		{
			int cursor_y = 160-96/2+14;
			CAT_fillberry(120-48, cursor_y - 28, 96, 96, CAT_WHITE);
			for(int i = 0; i < 3; i++)
			{
				const char* fmt = i == channel_idx ? "< %.3d >\n" : "%.3d\n";
				int fmt_len = i == channel_idx ? strlen("< XXX >") : strlen("XXX");
				cursor_y = CAT_draw_textf(120 - (fmt_len * CAT_GLYPH_WIDTH)/2, cursor_y, fmt, palette[colour_idx][i]);
			}			
		}
		break;
	}
}