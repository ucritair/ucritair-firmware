#include "cat_menu.h"

#include "cat_input.h"
#include "cat_render.h"
#include "cat_gui.h"
#include <math.h>

static float V = 0.5f;
static CAT_ivec2 cursor = {0, 0};
static bool show_details = false;

uint16_t HSV2RGB(float H, float S, float V, uint8_t* R_out, uint8_t* G_out, uint8_t* B_out)
{
	float C = (1 - fabsf(2 * V - 1)) * S;
	float H_p = H / 60.0f;
	float X = C * (1 - abs(((int) H_p) % 2 - 1));

	float R_1 = 0;
	float G_1 = 0;
	float B_1 = 0;
	if(H_p < 1)
	{
		R_1 = C;
		G_1 = X;
	}
	else if(H_p < 2)
	{
		R_1 = X;
		G_1 = C;
	}
	else if(H_p < 3)
	{
		G_1 = C;
		B_1 = X;
	}
	else if(H_p < 4)
	{
		G_1 = X;
		B_1 = C;
	}
	else if(H_p < 5)
	{
		B_1 = C;
		R_1 = X;
	}
	else
	{
		B_1 = X;
		R_1 = C;
	}

	float m = V - C * 0.5f;
	float R_n = R_1 + m;
	float G_n = G_1 + m;
	float B_n = B_1 + m;
	uint8_t R = SCALEBYTE(255, R_n);
	uint8_t G = SCALEBYTE(255, G_n);
	uint8_t B = SCALEBYTE(255, B_n);

	if(R_out != NULL)
		*R_out = R;
	if(G_out != NULL)
		*G_out = G;
	if(B_out != NULL)
		*B_out = B;
	return RGB8882565(R, G, B);
}

void CAT_MS_colour_picker(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_colour_picker);
			break;
		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_back();

			if(CAT_input_held(CAT_BUTTON_A, 0))
				V += 0.05f;
			if(CAT_input_held(CAT_BUTTON_B, 0))
				V -= 0.05f;
			V = clampf(V, 0, 1);

			if(CAT_input_touching())
			{
				cursor = (CAT_ivec2) {input.touch.x, input.touch.y};
			}
			else
			{
				if(CAT_input_held(CAT_BUTTON_LEFT, 0))
					cursor.x -= 2;
				if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
					cursor.x += 2;
				if(CAT_input_held(CAT_BUTTON_UP, 0))
					cursor.y -= 2;
				if(CAT_input_held(CAT_BUTTON_DOWN, 0))
					cursor.y += 2;
			}	
			cursor.x = clamp(cursor.x, 0, CAT_LCD_SCREEN_W-1);
			cursor.y = clamp(cursor.y, 0, CAT_LCD_SCREEN_H-1);

			if(CAT_input_pressed(CAT_BUTTON_SELECT))
				show_details = !show_details;			
			
			break;
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_colour_picker()
{
	for(int y = 0; y < CAT_LCD_SCREEN_H; y++)
	{
		float y_n = (float) y / (float) CAT_LCD_SCREEN_H;
		float S = y_n;

		for(int x = 0; x < CAT_LCD_SCREEN_W; x++)
		{
			float x_n = (float) x / (float) CAT_LCD_SCREEN_W;
			float H = x_n * 360;
			
			uint16_t colour = HSV2RGB(H, S, V, NULL, NULL, NULL);
			CAT_fillberry(x, y, 1, 1, colour);
		}
	}

	uint8_t highlight_brightness = V < 0.75 ? 255 : 0;
	uint16_t highlight_colour = RGB8882565(highlight_brightness, highlight_brightness, highlight_brightness);
	CAT_strokeberry(cursor.x-2, cursor.y-2, 5, 5, highlight_colour);

	if(show_details)
	{
		float S = (float) cursor.y / (float) CAT_LCD_SCREEN_H;
		float H = 360 * (float) cursor.x / (float) CAT_LCD_SCREEN_W;
		uint8_t R, G, B;
		uint16_t colour = HSV2RGB(H, S, V, &R, &G, &B);

		CAT_gui_panel(CAT_iv2(2, 6), CAT_iv2(11, 8));
		CAT_strokeberry(2 * 16, 6 * 16, 11 * 16, 8 * 16, 0x0000);
		CAT_gui_textf("HSV: %0.0f %0.2f %0.2f\nRGB: %d %d %d\n565: %#x\n", H, S, V, R, G, B, colour);
	}
}