#include "cat_menu.h"

#include "cat_input.h"
#include "cat_render.h"
#include "cat_gui.h"
#include <math.h>

#define CAT_HSV_MAX_HUE (255 * 5)

static uint8_t V = 128;
static CAT_ivec2 cursor = {120, 160};
static bool show_details = false;

uint8_t saturate_colour(uint8_t colour, uint8_t saturation)
{
	uint8_t offset = (uint16_t) ((255 - saturation) * (255 - colour)) / 255;
	return colour + offset;
}

void HSV2RGB(uint16_t H, uint8_t S, uint8_t V, uint8_t* R_out, uint8_t* G_out, uint8_t* B_out)
{
	H %= CAT_HSV_MAX_HUE;
	uint8_t arc = H / 255;
	uint8_t angle = H % 255;

	switch (arc)
	{
		case 0:
			*R_out = V;
			*G_out = saturate_colour(angle, S) * ((uint16_t) V) / 255;
			*B_out = (255 - S) * ((uint16_t) V) / 255;
		break;
		case 1:
			*R_out = saturate_colour(255 - angle, S) * ((uint16_t) V) / 255;
			*G_out = V;
			*B_out = (255 - S) * ((uint16_t) V) / 255;
		break;
		case 2:
			*R_out = (255 - S) * ((uint16_t) V) / 255;
			*G_out = V;
			*B_out = saturate_colour(angle, S) * ((uint16_t) V) / 255;
		break;
		case 3:
			*R_out = (255 - S) * ((uint16_t) V) / 255;
			*G_out = saturate_colour(255 - angle, S) * ((uint16_t) V) / 255;
			*B_out = V;
		break;
		case 4:
			*R_out = saturate_colour(angle, S) * ((uint16_t) V) / 255;
			*G_out = (255 - S) * ((uint16_t) V) / 255;
			*B_out = V;
		break;
		case 5:
			*R_out = V;
			*G_out = saturate_colour(255 - angle, S) * ((uint16_t) V) / 255;
			*B_out = (255 - S) * ((uint16_t) V) / 255;
		break;
	}
}

void CAT_MS_colour_picker(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_colour_picker);
			break;
		case CAT_FSM_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_pushdown_back();

			if(CAT_input_held(CAT_BUTTON_A, 0))
				V += 4;
			if(CAT_input_held(CAT_BUTTON_B, 0))
				V -= 4;

			if(CAT_input_touching())
			{
				cursor = (CAT_ivec2) {input.touch.x, input.touch.y};
			}
			else
			{
				if(CAT_input_held(CAT_BUTTON_LEFT, 0))
					cursor.x -= 4;
				if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
					cursor.x += 4;
				if(CAT_input_held(CAT_BUTTON_UP, 0))
					cursor.y -= 4;
				if(CAT_input_held(CAT_BUTTON_DOWN, 0))
					cursor.y += 4;
			}	
			cursor.x = clamp(cursor.x, 0, CAT_LCD_SCREEN_W-1);
			cursor.y = clamp(cursor.y, 0, CAT_LCD_SCREEN_H-1);

			if(CAT_input_pressed(CAT_BUTTON_SELECT))
				show_details = !show_details;			
			
			break;
		case CAT_FSM_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_colour_picker()
{
	uint16_t lower_row = CAT_LCD_FRAMEBUFFER_H * CAT_get_render_cycle();
	for(uint16_t y = lower_row; y < lower_row + CAT_LCD_FRAMEBUFFER_H; y++)
	{
		uint16_t H = ((uint32_t) y * CAT_HSV_MAX_HUE) / CAT_LCD_SCREEN_H;
		for(uint16_t x = 0; x < CAT_LCD_SCREEN_W; x++)
		{
			uint8_t S = ((uint16_t) x * 255) / CAT_LCD_SCREEN_W;
			uint8_t R, G, B;
			HSV2RGB(H, S, V, &R, &G, &B);
			uint16_t colour = RGB8882565(R, G, B);
			CAT_pixberry(x, y, colour);
		}
	}

	uint8_t highlight_value = V < 128 ? 255 : 0;
	uint16_t highlight_colour = RGB8882565(highlight_value, highlight_value, highlight_value);
	CAT_strokeberry(cursor.x-2, cursor.y-2, 5, 5, highlight_colour);

	if(show_details)
	{
		uint16_t H = ((uint32_t) cursor.y * CAT_HSV_MAX_HUE) / CAT_LCD_SCREEN_H;
		uint8_t S = ((uint16_t) cursor.x * 255) / CAT_LCD_SCREEN_W;	
		uint8_t R, G, B;
		HSV2RGB(H, S, V, &R, &G, &B);
		uint16_t colour = RGB8882565(R, G, B);

		CAT_gui_panel((CAT_ivec2) {2, 6}, (CAT_ivec2) {11, 8});
		CAT_strokeberry(2 * 16, 6 * 16, 11 * 16, 8 * 16, 0x0000);
		CAT_gui_textf("HSV: %d %d %d\nRGB: %d %d %d\n565: %#x\n", H, S, V, R, G, B, colour);
		CAT_fillberry(2 * 16 + 32, 6 * 16 + 72, 11 * 16 - 64, 8 * 16 - 96, colour);
	}
}