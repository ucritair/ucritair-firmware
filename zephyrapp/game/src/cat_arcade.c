#include "cat_arcade.h"
#include "cat_machine.h"
#include "cat_math.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_sprite.h"

#define MAX_SNAKE_LENGTH 256

int width = 20;
int height = 10;

int body_x[MAX_SNAKE_LENGTH];
int body_y[MAX_SNAKE_LENGTH];
int body_length = 1;

int x_shift = 0;
int y_shift = 0;

int move_timer_id = -1;
int moves = 0;

bool dead = false;

void CAT_arcade_init()
{
	move_timer_id = CAT_timer_init(0.25f);
	body_x[0] = 0;
	body_y[0] = 0;
}

void CAT_arcade_tick()
{
	for(int i = 0; i < body_length && !dead; i++)
	{
		dead =
		body_x[i] < 0 || body_x[i] >= width ||
		body_y[i] < 0 || body_y[i] >= height;
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
			if(moves >= 12 && body_length < MAX_SNAKE_LENGTH)
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
		for(int y = 0; y < height; y++)
		{
			for(int x = 0; x < width; x++)
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