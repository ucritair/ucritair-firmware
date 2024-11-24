#include "cat_arcade.h"

#include "cat_machine.h"
#include "cat_math.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_sprite.h"
#include "cat_bag.h"
#include <stdio.h>

#ifdef CAT_EMBEDDED
#include "menu_system.h"
#include "menu_aqi.h"
#include "menu_graph.h"
#endif

#define WIDTH 15
#define HEIGHT 20
#define SNAKE_MAX_LENGTH (WIDTH * HEIGHT)
#define SNAKE_MAX_ITERATIONS SNAKE_MAX_LENGTH * 2
#ifdef CAT_EMBEDDED
#define TICK_PERIOD 3
#else
#define TICK_PERIOD 9
#endif
#define ANIM_PERIOD 4

enum {SELECT, PLAY, LOSE} mode;
static int selector = 0;

int grasses[10] = {8, 2, 4, 10, 0, 11, 11, 7, 7, 12};

int snake_x[SNAKE_MAX_LENGTH];
int snake_y[SNAKE_MAX_LENGTH];
int snake_length = 2;

int ldx = 0;
int ldy = 0;
int dx = 1;
int dy = 0;

int food_x = -1;
int food_y = -1;
int food_sprite_id = -1;

int score = 0;
int snake_high_score = 0;
bool new_high_score = false;

int tick_count = 0;
int anim_frame = 0;
int eat_count = 0;

void spawn_food()
{
	/*bool valid = false;
	int its = 0;
	while(!valid && its < SNAKE_MAX_ITERATIONS)
	{
		food_x = CAT_rand_int(1, WIDTH-2);
		food_y = CAT_rand_int(1, HEIGHT-2);
		valid = true;
		for(int i = 0; i < snake_length; i++)
		{
			if
			(
				(food_x == snake_x[i] && food_y == snake_y[i]) ||
				(food_x == lfx && food_y == lfy)
			)
			{
				valid = false;
				break;
			}
		}
		its += 1;
	}*/

	int guess_y = CAT_rand_int(1, HEIGHT-2);
	int steps_y = 0;
	int guess_x = CAT_rand_int(1, WIDTH-2);
	int steps_x = 0;

food_spawn_fail:
	guess_y = CAT_rand_int(1, HEIGHT-2);
	guess_x = CAT_rand_int(1, WIDTH-2);
	for(int y = guess_y; steps_y < HEIGHT; y++)
	{
		if(y >= HEIGHT)
			y = 0;

		for(int x = guess_x; steps_x < WIDTH; x++)
		{
			if(x >= WIDTH)
				x = 0;

			for(int i = 0; i < snake_length; i++)
			{
				if
				(
					(x == snake_x[i] && y == snake_y[i]) ||
					(x == food_x && y == food_y)
				)
				{
					goto food_spawn_fail;
				}
			}
			goto food_spawn_success;
		}
	}

food_spawn_success:
	food_x = guess_x;
	food_y = guess_y;

	int food_options[] = 
	{
		cigarette_sprite,
		padkaprow_sprite,
		sausage_sprite,
		coffee_sprite
	};
	food_sprite_id = eat_count == 4 ? coin_world_sprite : food_options[CAT_rand_int(0, 3)];
}

void snake_init()
{
	snake_x[0] = 2;
	snake_y[0] = 1;
	snake_x[1] = 1;
	snake_y[1] = 1;
	snake_length = 2;

	ldx = 0;
	ldy = 0;
	dx = 1;
	dy = 0;

	spawn_food();

	score = 0;
	new_high_score = false;

	tick_count = 0;
	anim_frame = 0;
	eat_count = 0;
}

void snake_tick()
{	
	if(CAT_input_pressed(CAT_BUTTON_UP) && ldy != 1)
	{
		dx = 0;
		dy = -1;
	}	
	if(CAT_input_pressed(CAT_BUTTON_RIGHT) && ldx != -1)
	{
		dx = 1;
		dy = 0;
	}
	if(CAT_input_pressed(CAT_BUTTON_DOWN) && ldy != -1)
	{
		dx = 0;
		dy = 1;
	}
	if(CAT_input_pressed(CAT_BUTTON_LEFT) && ldx != 1)
	{
		dx = -1;
		dy = 0;
	}

	if(tick_count >= TICK_PERIOD)
	{
		ldx = dx;
		ldy = dy;
		int x = snake_x[0] + dx;
		int y = snake_y[0] + dy;

		if
		(
			x < 0 || x >= WIDTH ||
			y < 0 || y >= HEIGHT
		)
		{
			mode = LOSE;
		}

		if(x == food_x && y == food_y)
		{	
			eat_count += 1;
			if(eat_count == 5)
			{
				coins += 1;
				eat_count = 0;
			}

			score += 1;
			if(score > snake_high_score)
			{
				snake_high_score = score;
				new_high_score = true;
			}

			snake_length += 1;
			spawn_food();
		}

		for(int i = snake_length-1; i > 0; i--)
		{
			snake_x[i] = snake_x[i-1];
			snake_y[i] = snake_y[i-1];
			if(snake_x[i] == x && snake_y[i] == y)
			{
				mode = LOSE;
			}
		}
		snake_x[0] = x;
		snake_y[0] = y;

		tick_count = 0;

		anim_frame += 1;
		if(anim_frame >= ANIM_PERIOD)
			anim_frame = 0;
	}
	tick_count += 1;
}

void snake_render_map()
{
	CAT_frameberry(RGB8882565(122, 146, 57));
	for(int y = 0; y < 20; y += 2)
	{
		CAT_draw_sprite(grass_floor_sprite, 17, grasses[y/2]*16, y*16);
	}
}

void snake_render_character()
{
	int dx = snake_x[0] - snake_x[1];
	int dy = snake_y[0] - snake_y[1];

	int head_x = snake_x[0] * 16;
	int head_y = snake_y[0] * 16;
	if(dx == 1)
		CAT_draw_sprite(snake_head_sprite, 0, head_x, head_y);
	else if(dy == 1)
		CAT_draw_sprite(snake_head_sprite, 1, head_x, head_y);
	else if(dx == -1)
		CAT_draw_sprite(snake_head_sprite, 2, head_x, head_y);
	else if(dy == -1)
		CAT_draw_sprite(snake_head_sprite, 3, head_x, head_y);

	for(int i = 1; i < snake_length; i++)
	{
		int dbx = snake_x[i-1] - snake_x[i];
		int dby = snake_y[i-1] - snake_y[i];

		int body_x = snake_x[i] * 16;
		int body_y = snake_y[i] * 16;
		
		if(i == snake_length-1)
		{
			if(dbx == 1)
				CAT_draw_sprite(snake_tail_sprite, 0, body_x, body_y);
			else if(dby == 1)
				CAT_draw_sprite(snake_tail_sprite, 1, body_x, body_y);
			else if(dbx == -1)
				CAT_draw_sprite(snake_tail_sprite, 2, body_x, body_y);
			else if(dby == -1)
				CAT_draw_sprite(snake_tail_sprite, 3, body_x, body_y);
			break;
		}

		int dfx = snake_x[i] - snake_x[i+1];
		int dfy = snake_y[i] - snake_y[i+1];

		if(dfx == 1 && dbx == 1)
			CAT_draw_sprite(snake_body_sprite, 0, body_x, body_y);
		else if(dfy == 1 && dby == 1)
			CAT_draw_sprite(snake_body_sprite, 1, body_x, body_y);
		else if(dfx == -1 && dbx == -1)
			CAT_draw_sprite(snake_body_sprite, 2, body_x, body_y);
		else if(dfy == -1 && dby == -1)
			CAT_draw_sprite(snake_body_sprite, 3, body_x, body_y);
		else if((dfx == -1 && dby == 1) || (dfy == -1 && dbx == 1))
			CAT_draw_sprite(snake_corner_sprite, 0, body_x, body_y);
		else if((dfx == 1 && dby == 1) || (dfy == -1 && dbx == -1))
			CAT_draw_sprite(snake_corner_sprite, 1, body_x, body_y);
		else if((dfx == 1 && dby == -1) || (dfy == 1 && dbx == -1))
			CAT_draw_sprite(snake_corner_sprite, 2, body_x, body_y);
		else if((dfx == -1 && dby == -1) || (dfy == 1 && dbx == 1))
			CAT_draw_sprite(snake_corner_sprite, 3, body_x, body_y);
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
				if(CAT_input_pressed(CAT_BUTTON_B))
					CAT_machine_back();
				if(CAT_input_pressed(CAT_BUTTON_START))
					CAT_machine_transition(CAT_MS_room);

				if(CAT_input_pulse(CAT_BUTTON_UP))
					selector -= 1;
				if(CAT_input_pulse(CAT_BUTTON_DOWN))
					selector += 1;
				selector = clamp(selector, 0, 2);
				
				if(CAT_input_pressed(CAT_BUTTON_A))
				{
#ifdef CAT_EMBEDDED
					if(selector == 0)
					{
						CAT_machine_transition(CAT_MS_aqi);
					}
					if(selector == 1)
					{
						CAT_machine_transition(CAT_MS_graph);
					}
#endif
					if(selector == 2)
					{
						snake_init();
						mode = PLAY;
					}
				}
			}
			else if(mode == PLAY)
			{
				if(CAT_input_pressed(CAT_BUTTON_B))
					mode = SELECT;
				if(CAT_input_pressed(CAT_BUTTON_START))
					CAT_machine_transition(CAT_MS_room);

				snake_tick();
			}
			else if(mode == LOSE)
			{
				if(CAT_input_pressed(CAT_BUTTON_START))
					CAT_machine_transition(CAT_MS_room);
				else if(CAT_input_pressed(CAT_BUTTON_A) || CAT_input_pressed(CAT_BUTTON_B))
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

		// This part is hideous. Ask M about it
		CAT_gui_textf("uCritAir Score %0.1f%%\n", CAT_AQI_aggregate());
		CAT_gui_div("AIR");
		CAT_gui_text("  & Air Quality Log ");
		if(selector == 0)
			CAT_gui_image(icon_pointer_sprite, 0);
		CAT_gui_line_break();
		CAT_gui_text("  & Air Quality Graph ");
		if(selector == 1)
			CAT_gui_image(icon_pointer_sprite, 0);
		CAT_gui_line_break();
		CAT_gui_div("PLAY");
		CAT_gui_text("  & Snack ");
		if(selector == 2)
			CAT_gui_image(icon_pointer_sprite, 0);
		CAT_gui_line_break();		
	}
	else if(mode == PLAY)
	{
		spriter.mode = CAT_DRAW_MODE_DEFAULT;

		snake_render_map();
		snake_render_character();

		CAT_sprite* sprite = CAT_sprite_get(food_sprite_id);
		int frame_idx = clamp(anim_frame, 0, sprite->frame_count-1);
		CAT_draw_sprite(food_sprite_id, frame_idx, food_x * 16, food_y * 16);
	}
	else if(mode == LOSE)
	{
		CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 20});
		for(int i = 1; i < 14; i++)
		{
			CAT_draw_sprite(snake_head_sprite, 1, i * 16, 16);
			CAT_draw_sprite(snake_head_sprite, 1, i * 16, 19*16);
		}
		for(int i = 1; i < 20; i++)
		{
			CAT_draw_sprite(snake_head_sprite, 1, 16, i * 16);
			CAT_draw_sprite(snake_head_sprite, 1, 13*16, i * 16);
		}

		gui.cursor.x += 28;
		gui.cursor.y += 24;
		gui.start.x += 28;
		gui.start.y += 24;
		CAT_gui_textf
		(
			"Rest in peace,\nSnack Cat.\n"
			"\n"
			"Your long journey has\n"
			"come to a tragic end.\n"
			"\n"
			"Press A or B to\n"
			"return from whence\n"
			"you came.\n"
			"\n"
			"SCORE: %d\n"
			"HIGH SCORE: %d\n"
			,score,
			snake_high_score
		);
		if(new_high_score)
		{
			CAT_gui_text("New high score!");
		}
	}
}