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

//////////////////////////////////////////////////////////////////////////
// MACHINE

static int selector = 0;

void CAT_MS_arcade(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
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
					CAT_machine_transition(CAT_MS_snake);
				}
			}
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
}


//////////////////////////////////////////////////////////////////////////
// SNAKE

#define SNAKE_ARENA_WIDTH 15
#define SNAKE_ARENA_HEIGHT 20
#define SNAKE_MAX_LENGTH (SNAKE_ARENA_WIDTH * SNAKE_ARENA_HEIGHT)
#ifdef CAT_EMBEDDED
#define SNAKE_TICK_PERIOD 3
#else
#define SNAKE_TICK_PERIOD 12
#endif

int snake_high_score = 0;

static struct
{
	int xs[SNAKE_MAX_LENGTH];
	int ys[SNAKE_MAX_LENGTH];
	int length;

	int ldx;
	int ldy;
	int dx;
	int dy;

	bool dead;
} snake;

static struct
{
	int x;
	int y;
	int sprite_id;
} food;

static int snake_ticks = 0;
static int snake_score = 0;
static bool snake_is_score_high = false;
static int snake_eat_count = 0;

void snake_init()
{
	snake.xs[0] = 1;
	snake.xs[1] = 0;
	snake.ys[0] = 0;
	snake.ys[1] = 0;
	snake.length = 2;

	snake.ldx = 0;
	snake.ldy = 0;
	snake.dx = 1;
	snake.dy = 0;

	snake.dead = false;
}

void snake_food_init()
{
	int guess_y = CAT_rand_int(1, SNAKE_ARENA_HEIGHT-2);
	int steps_y = 0;
	int guess_x = CAT_rand_int(1, SNAKE_ARENA_WIDTH-2);
	int steps_x = 0;

food_spawn_fail:
	guess_y = CAT_rand_int(1, SNAKE_ARENA_HEIGHT-2);
	guess_x = CAT_rand_int(1, SNAKE_ARENA_WIDTH-2);
	for(int y = guess_y; steps_y < SNAKE_ARENA_HEIGHT; y++)
	{
		if(y >= SNAKE_ARENA_HEIGHT)
			y = 0;

		for(int x = guess_x; steps_x < SNAKE_ARENA_WIDTH; x++)
		{
			if(x >= SNAKE_ARENA_WIDTH)
				x = 0;

			for(int i = 0; i < snake.length; i++)
			{
				if
				(
					(x == snake.xs[i] && y == snake.ys[i]) ||
					(x == food.x && y == food.y)
				)
				{
					goto food_spawn_fail;
				}
			}
			goto food_spawn_success;
		}
	}

food_spawn_success:
	food.x = guess_x;
	food.y = guess_y;

	int food_sprites[] = 
	{
		cigarette_sprite,
		padkaprow_sprite,
		sausage_sprite,
		coffee_sprite
	};
	int num_food_sprites = sizeof(food_sprites) / sizeof(food_sprites[0]);

	food.sprite_id =
	snake_eat_count == 4 ?
	coin_world_sprite :
	food_sprites[CAT_rand_int(0, num_food_sprites-1)];
}

void CAT_MS_snake(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			snake_init();
			snake_food_init();
			snake_ticks = 0;
			snake_score = 0;
			snake_eat_count = 0;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(!snake.dead)
			{
				if(CAT_input_pressed(CAT_BUTTON_B))
					CAT_machine_back();
				if(CAT_input_pressed(CAT_BUTTON_START))
					CAT_machine_transition(CAT_MS_room);

				if(CAT_input_pressed(CAT_BUTTON_UP) && snake.ldy != 1)
				{
					snake.dx = 0;
					snake.dy = -1;
				}	
				if(CAT_input_pressed(CAT_BUTTON_RIGHT) && snake.ldx != -1)
				{
					snake.dx = 1;
					snake.dy = 0;
				}
				if(CAT_input_pressed(CAT_BUTTON_DOWN) && snake.ldy != -1)
				{
					snake.dx = 0;
					snake.dy = 1;
				}
				if(CAT_input_pressed(CAT_BUTTON_LEFT) && snake.ldx != 1)
				{
					snake.dx = -1;
					snake.dy = 0;
				}

				if(snake_ticks >= SNAKE_TICK_PERIOD)
				{
					snake.ldx = snake.dx;
					snake.ldy = snake.dy;
					int x = snake.xs[0] + snake.dx;
					int y = snake.ys[0] + snake.dy;

					snake.dead |= x < 0;
					snake.dead |= x >= SNAKE_ARENA_WIDTH;
					snake.dead |= y < 0;
					snake.dead |= y >= SNAKE_ARENA_HEIGHT;

					for(int i = 0; i < snake.length; i++)
					{
						snake.dead |=
						snake.xs[i] == x &&
						snake.ys[i] == y;
					}

					if(x == food.x && y == food.y)
					{	
						snake_eat_count += 1;
						if(snake_eat_count == 5)
						{
							coins += 1;
							snake_eat_count = 0;
						}
						snake_food_init();

						snake_score += 1;
						snake.length += 1;
						snake.dead |= snake.length >= SNAKE_MAX_LENGTH;	
					}

					for(int i = snake.length-1; i > 0; i--)
					{
						snake.xs[i] = snake.xs[i-1];
						snake.ys[i] = snake.ys[i-1];
					}
					snake.xs[0] = x;
					snake.ys[0] = y;
					
					snake_ticks = 0;
				}
				snake_ticks += 1;
			}
			else
			{
				if(CAT_input_pressed(CAT_BUTTON_A) || CAT_input_pressed(CAT_BUTTON_B))
					CAT_machine_back();
				if(CAT_input_pressed(CAT_BUTTON_START))
					CAT_machine_transition(CAT_MS_room);

				if(snake_score > snake_high_score)
				{
					snake_high_score = snake_score;
					snake_is_score_high = true;
				}
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}


static int grasses[10] = {8, 2, 4, 10, 0, 11, 11, 7, 7, 12};

void CAT_render_snake()
{
	spriter.mode = CAT_DRAW_MODE_DEFAULT;

	if(!snake.dead)
	{
		CAT_frameberry(RGB8882565(122, 146, 57));
		for(int y = 0; y < 20; y += 2)
		{
			CAT_draw_sprite(grass_floor_sprite, 17, grasses[y/2]*16, y*16);
		}

		int dx = snake.xs[0] - snake.xs[1];
		int dy = snake.ys[0] - snake.ys[1];
		int head_x = snake.xs[0] * 16;
		int head_y = snake.ys[0] * 16;

		if(dx == 1)
			CAT_draw_sprite(snake_head_sprite, 0, head_x, head_y);
		else if(dy == 1)
			CAT_draw_sprite(snake_head_sprite, 1, head_x, head_y);
		else if(dx == -1)
			CAT_draw_sprite(snake_head_sprite, 2, head_x, head_y);
		else if(dy == -1)
			CAT_draw_sprite(snake_head_sprite, 3, head_x, head_y);

		for(int i = 1; i < snake.length; i++)
		{
			int dbx = snake.xs[i-1] - snake.xs[i];
			int dby = snake.ys[i-1] - snake.ys[i];

			int body_x = snake.xs[i] * 16;
			int body_y = snake.ys[i] * 16;
			
			if(i == snake.length-1)
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

			int dfx = snake.xs[i] - snake.xs[i+1];
			int dfy = snake.ys[i] - snake.ys[i+1];

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

		CAT_draw_sprite(food.sprite_id, 0, food.x * 16, food.y * 16);
	}
	else
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
			, snake_score,
			snake_high_score
		);

		if(snake_is_score_high)
		{
			CAT_gui_text("New high score!");
		}
	}
}