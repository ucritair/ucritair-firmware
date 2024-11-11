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
uint8_t body_length = 2;
int8_t x_shift = 1;
int8_t y_shift = 0;
int move_timer_id = -1;

int food_sprite_id = -1;
int8_t food_x = -1;
int8_t food_y = -1;
int food_timer_id = -1;

bool dead = false;

void CAT_arcade_init()
{
	body_x[0] = 1;
	body_y[0] = 0;
	body_x[1] = 0;
	body_y[1] = 0;
	move_timer_id = CAT_timer_init(0.15f);

	food_sprite_id = padkaprow_sprite;
	food_x = CAT_rand_int(0, WIDTH);
	food_y = CAT_rand_int(0, HEIGHT);
	food_timer_id = CAT_timer_init(2.0f);
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
		if(CAT_input_pressed(CAT_BUTTON_UP) && y_shift != 1)
		{
			x_shift = 0;
			y_shift = -1;
		}	
		if(CAT_input_pressed(CAT_BUTTON_RIGHT) && x_shift != -1)
		{
			x_shift = 1;
			y_shift = 0;
		}
		if(CAT_input_pressed(CAT_BUTTON_DOWN) && y_shift != -1)
		{
			x_shift = 0;
			y_shift = 1;
		}
		if(CAT_input_pressed(CAT_BUTTON_LEFT) && x_shift != 1)
		{
			x_shift = -1;
			y_shift = 0;
		}

		CAT_timer_tick(food_timer_id);
		if(CAT_timer_tick(move_timer_id))
		{
			int x = body_x[0] + x_shift;
			int y = body_y[0] + y_shift;
			
			if(CAT_timer_progress(food_timer_id) >= 1)
			{
				if(x == food_x && y == food_y)
				{
					body_length += 1;
					food_x = CAT_rand_int(0, WIDTH-1);
					food_y = CAT_rand_int(0, HEIGHT-1);
					CAT_timer_reset(food_timer_id);

					int food_options[] = 
					{
						cigarette_sprite,
						padkaprow_sprite,
						sausage_sprite,
						coffee_sprite	
					};
					food_sprite_id = food_options[CAT_rand_int(0, 3)];
				}
			}

			for(int i = body_length-1; i > 0; i--)
			{
				body_x[i] = body_x[i-1];
				body_y[i] = body_y[i-1];
				if(body_x[i] == x && body_y[i] == y)
					dead = true;
			}
			body_x[0] = x;
			body_y[0] = y;
			
			CAT_timer_reset(move_timer_id);
		}
	}
	else
	{
		body_length = 2;
		body_x[0] = 1;
		body_y[0] = 0;
		body_x[1] = 0;
		body_y[1] = 0;
		x_shift = 1;
		y_shift = 0;
		
		CAT_timer_reset(move_timer_id);
		CAT_timer_reset(food_timer_id);
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
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
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
	spriter.mode = CAT_DRAW_MODE_DEFAULT;

	if(!dead)
	{
		CAT_draw_tiles(grass_floor_sprite, 20, 0, 20);

		int dx = body_x[0] - body_x[1];
		int dy = body_y[0] - body_y[1];
		if(dx == 1)
			CAT_draw_sprite(snake_head_sprite, 0, body_x[0] * 16, body_y[0] * 16);
		else if(dy == 1)
			CAT_draw_sprite(snake_head_sprite, 1, body_x[0] * 16, body_y[0] * 16);
		else if(dx == -1)
			CAT_draw_sprite(snake_head_sprite, 2, body_x[0] * 16, body_y[0] * 16);
		else if(dy == -1)
			CAT_draw_sprite(snake_head_sprite, 3, body_x[0] * 16, body_y[0] * 16);
	
		for(int i = 1; i < body_length; i++)
		{
			int dbx = body_x[i-1] - body_x[i];
			int dby = body_y[i-1] - body_y[i];

			if(i == body_length-1)
			{
				if(dbx == 1)
					CAT_draw_sprite(snake_tail_sprite, 0, body_x[i] * 16, body_y[i] * 16);
				else if(dby == 1)
					CAT_draw_sprite(snake_tail_sprite, 1, body_x[i] * 16, body_y[i] * 16);
				else if(dbx == -1)
					CAT_draw_sprite(snake_tail_sprite, 2, body_x[i] * 16, body_y[i] * 16);
				else if(dby == -1)
					CAT_draw_sprite(snake_tail_sprite, 3, body_x[i] * 16, body_y[i] * 16);
				break;
			}

			int dfx = body_x[i] - body_x[i+1];
			int dfy = body_y[i] - body_y[i+1];

			if(dfx == 1 && dbx == 1)
				CAT_draw_sprite(snake_body_sprite, 0, body_x[i] * 16, body_y[i] * 16);
			else if(dfy == 1 && dby == 1)
				CAT_draw_sprite(snake_body_sprite, 1, body_x[i] * 16, body_y[i] * 16);
			else if(dfx == -1 && dbx == -1)
				CAT_draw_sprite(snake_body_sprite, 2, body_x[i] * 16, body_y[i] * 16);
			else if(dfy == -1 && dby == -1)
				CAT_draw_sprite(snake_body_sprite, 3, body_x[i] * 16, body_y[i] * 16);
			else if(dfx == -1 && dby == 1 || dfy == -1 && dbx == 1)
				CAT_draw_sprite(snake_corner_sprite, 0, body_x[i] * 16, body_y[i] * 16);
			else if(dfx == 1 && dby == 1 || dfy == -1 && dbx == -1)
				CAT_draw_sprite(snake_corner_sprite, 1, body_x[i] * 16, body_y[i] * 16);
			else if(dfx == 1 && dby == -1 || dfy == 1 && dbx == -1)
				CAT_draw_sprite(snake_corner_sprite, 2, body_x[i] * 16, body_y[i] * 16);
			else if(dfx == -1 && dby == -1 || dfy == 1 && dbx == 1)
				CAT_draw_sprite(snake_corner_sprite, 3, body_x[i] * 16, body_y[i] * 16);
		}
	}

	if(CAT_timer_progress(food_timer_id) >= 1)
	{
		CAT_draw_sprite(food_sprite_id, 0, food_x * 16, food_y * 16);
	}
}