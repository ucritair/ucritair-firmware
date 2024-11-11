#include "cat_arcade.h"

#include "cat_machine.h"
#include "cat_math.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_sprite.h"
#include "cat_bag.h"
#include <stdio.h>

#define WIDTH 15
#define HEIGHT 20
#define MAX_SNAKE_LENGTH (WIDTH * HEIGHT)

uint8_t body_x[MAX_SNAKE_LENGTH];
uint8_t body_y[MAX_SNAKE_LENGTH];
uint8_t body_length = 2;

int8_t x_shift = 1;
int8_t y_shift = 0;
int move_timer_id = -1;

int8_t food_x = -1;
int8_t food_y = -1;
int food_sprite_id = -1;

bool grow = false;
int score = 0;

enum {SELECT, PLAY, LOSE} mode;

void spawn_food()
{
	bool valid = false;
	while(!valid)
	{
		food_x = CAT_rand_int(1, WIDTH-2);
		food_y = CAT_rand_int(1, HEIGHT-2);
		valid = true;
		for(int i = 0; i < body_length; i++)
		{
			if(food_x == body_x[i] || food_y == body_y[i])
			{
				valid = false;
				break;
			}
		}
	}

	int food_options[] = 
	{
		cigarette_sprite,
		padkaprow_sprite,
		sausage_sprite,
		coffee_sprite
	};
	food_sprite_id = score < 4 ? food_options[CAT_rand_int(0, 3)] : coin_static_sprite;
}

void snake_init()
{
	body_x[0] = 2;
	body_y[0] = 1;
	body_x[1] = 1;
	body_y[1] = 1;
	body_length = 2;

	x_shift = 1;
	y_shift = 0;
	move_timer_id = CAT_timer_init(0.15f);

	food_x = CAT_rand_int(1, WIDTH-2);
	food_y = CAT_rand_int(1, HEIGHT-2);
	food_sprite_id = padkaprow_sprite;

	grow = false;
	score = 0;
}

void snake_tick()
{
	if(mode == LOSE)
		return;

	// TODO: right now, if you rapidly switch, you can jump the fence and crash into yourself
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

	if(CAT_timer_tick(move_timer_id))
	{
		int x = body_x[0] + x_shift;
		int y = body_y[0] + y_shift;
		if
		(
			x < 0 || x >= WIDTH ||
			y < 0 || y >= HEIGHT
		)
		{
			mode = LOSE;
		}

		if(grow)
		{
			body_length += 1;
			grow = false;
		}

		for(int i = body_length-1; i > 0; i--)
		{
			body_x[i] = body_x[i-1];
			body_y[i] = body_y[i-1];
			if(body_x[i] == x && body_y[i] == y)
			{
				mode = LOSE;
			}
		}
		body_x[0] = x;
		body_y[0] = y;

		if(x == food_x && y == food_y)
		{
			grow = true;
			score += 1;
			if(score >= 5)
			{
				bag.coins += 1;
				score = 0;
			}
			spawn_food();
		}
		
		CAT_timer_reset(move_timer_id);
	}
}

void CAT_MS_arcade(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			mode = SELECT;
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(mode == SELECT)
			{
				if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
					CAT_machine_transition(&machine, CAT_MS_room);
				if(CAT_input_pressed(CAT_BUTTON_A))
				{
					snake_init();
					mode = PLAY;
				}
			}
			else if(mode == PLAY)
			{
				if(CAT_input_pressed(CAT_BUTTON_B))
					mode = SELECT;
				if(CAT_input_pressed(CAT_BUTTON_START))
					CAT_machine_transition(&machine, CAT_MS_room);

				snake_tick();
			}
			else if(mode == LOSE)
			{
				if(CAT_input_pressed(CAT_BUTTON_START))
					CAT_machine_transition(&machine, CAT_MS_room);
				else if(CAT_input_any())
					mode = SELECT;
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_arcade()
{
	if(mode == SELECT)
	{
		CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
		CAT_gui_text("ARCADE ");
		CAT_gui_image(icon_a_sprite, 1);
		CAT_gui_image(icon_enter_sprite, 0);
		CAT_gui_image(icon_b_sprite, 1);
		CAT_gui_image(icon_exit_sprite, 0);

		CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

		CAT_gui_text("# SNAKE");
		CAT_gui_image(icon_pointer_sprite, 0);
	}
	else if(mode == PLAY)
	{
		CAT_clear_frame(0xf75b);
		spriter.mode = CAT_DRAW_MODE_DEFAULT;

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
			else if((dfx == -1 && dby == 1) || (dfy == -1 && dbx == 1))
				CAT_draw_sprite(snake_corner_sprite, 0, body_x[i] * 16, body_y[i] * 16);
			else if((dfx == 1 && dby == 1) || (dfy == -1 && dbx == -1))
				CAT_draw_sprite(snake_corner_sprite, 1, body_x[i] * 16, body_y[i] * 16);
			else if((dfx == 1 && dby == -1) || (dfy == 1 && dbx == -1))
				CAT_draw_sprite(snake_corner_sprite, 2, body_x[i] * 16, body_y[i] * 16);
			else if((dfx == -1 && dby == -1) || (dfy == 1 && dbx == 1))
				CAT_draw_sprite(snake_corner_sprite, 3, body_x[i] * 16, body_y[i] * 16);
		}

		CAT_draw_sprite(food_sprite_id, 0, food_x * 16, food_y * 15);
	}
	else if(mode == LOSE)
	{
		CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 20});
		for(int i = 1; i < 14; i++)
		{
			CAT_draw_sprite(snake_death_up_sprite, 1, i * 16, 16);
			CAT_draw_sprite(snake_death_up_sprite, 1, i * 16, 19*16);
		}
		for(int i = 1; i < 20; i++)
		{
			CAT_draw_sprite(snake_death_up_sprite, 1, 16, i * 16);
			CAT_draw_sprite(snake_death_up_sprite, 1, 13*16, i * 16);
		}

		gui.cursor.x += 28;
		gui.cursor.y += 24;
		gui.start.x += 28;
		gui.start.y += 24;
		CAT_gui_text
		(
			"Your long journey has\n"
			"come to a tragic end.\n"
			"\n"
			"Press any button to\n"
			"return from whence\n"
			"you came."
		);
	}
}