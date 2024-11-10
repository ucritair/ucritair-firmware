#include "cat_arcade.h"
#include "cat_machine.h"
#include "cat_math.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_sprite.h"

#define MAX_SNAKE_LENGTH 256
#define WIDTH 15
#define HEIGHT 20

uint8_t body_x[MAX_SNAKE_LENGTH];
uint8_t body_y[MAX_SNAKE_LENGTH];
uint8_t body_length = 1;

uint8_t x_shift = 1;
uint8_t y_shift = 0;

int move_timer_id = -1;
int moves = 0;

bool dead = false;

void CAT_arcade_init()
{
	move_timer_id = CAT_timer_init(0.1f);
	body_x[0] = 0;
	body_y[0] = 0;
}

void CAT_arcade_tick()
{
	for(int i = 0; i < body_length && !dead; i++)
	{
		dead =
		body_x[i] < 0 || body_x[i] >= WIDTH ||
		body_y[i] < 0 || body_y[i] >= HEIGHT;
	}

	if(!dead)
	{
		if(CAT_input_pressed(CAT_BUTTON_UP))
		{
			x_shift = 0;
			y_shift = -1;
		}	
		if(CAT_input_pressed(CAT_BUTTON_RIGHT))
		{
			x_shift = 1;
			y_shift = 0;
		}
		if(CAT_input_pressed(CAT_BUTTON_DOWN))
		{
			x_shift = 0;
			y_shift = 1;
		}
		if(CAT_input_pressed(CAT_BUTTON_LEFT))
		{
			x_shift = -1;
			y_shift = 0;
		}

		if(CAT_timer_tick(move_timer_id))
		{
			moves += 1;
			if(moves >= 12)
			{
				body_length += 1;
				moves = 0;
			}

			for(int i = body_length-1; i > 0; i--)
			{
				body_x[i] = body_x[i-1];
				body_y[i] = body_y[i-1];
			}
			body_x[0] += x_shift;
			body_y[0] += y_shift;

			CAT_timer_reset(move_timer_id);
		}
	}
	else
	{
		body_x[0] = 0;
		body_y[0] = 0;
		body_length = 1;
		x_shift = 1;
		y_shift = 0;
		
		moves = 0;
		CAT_timer_reset(move_timer_id);
		dead = false;
	}
}

void CAT_MS_arcade(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_arcade_init();
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, CAT_MS_menu);
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_room);
			
			CAT_arcade_tick();
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_arcade()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("ARCADE ");
	CAT_gui_image(fbut_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	int mode_cache = spriter.mode;
	spriter.mode = CAT_DRAW_MODE_DEFAULT;
	if(!dead)
	{
		for(int y = 0; y < HEIGHT; y++)
		{
			for(int x = 0; x < WIDTH; x++)
			{
				CAT_draw_sprite(base_floor_sprite, 2, x * 16, y * 16);
			}
		}
		for(int i = 0; i < body_length; i++)
		{
			CAT_draw_sprite(toy_baseball_sprite, 0, body_x[i] * 16, body_y[i] * 16);
		}
	}
	spriter.mode = mode_cache;
}