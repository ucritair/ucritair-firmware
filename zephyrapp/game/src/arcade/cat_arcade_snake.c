#include "cat_arcade.h"

#include "cat_input.h"
#include "cat_render.h"
#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "sprite_assets.h"
#include "sound_assets.h"
#include "item_assets.h"

#define GRID_WIDTH 15
#define GRID_HEIGHT 20
#define SNAKE_MAX_LENGTH (GRID_WIDTH * GRID_HEIGHT)
#define SNAKE_TICK_PERIOD 3

int snake_high_score = 0;
static int score = 0;
static bool is_high_score = false;

static struct
{
	uint8_t xs[SNAKE_MAX_LENGTH];
	uint8_t ys[SNAKE_MAX_LENGTH];
	uint8_t length;

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
	const CAT_sprite* sprite;
} food;

static int ticks = 0;
static int eat_tracker = 0;

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

void food_init()
{
	int guess_y = CAT_rand_int(1, GRID_HEIGHT-2);
	int steps_y = 0;
	int guess_x = CAT_rand_int(1, GRID_WIDTH-2);
	int steps_x = 0;

food_spawn_fail:
	guess_y = CAT_rand_int(1, GRID_HEIGHT-2);
	guess_x = CAT_rand_int(1, GRID_WIDTH-2);
	for(int y = guess_y; steps_y < GRID_HEIGHT; y++)
	{
		if(y >= GRID_HEIGHT)
			y = 0;

		for(int x = guess_x; steps_x < GRID_WIDTH; x++)
		{
			if(x >= GRID_WIDTH)
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

	const CAT_sprite* food_sprites[] = 
	{
		&bread_sprite,
		&soup_sprite,
		&green_curry_sprite,
		&red_curry_sprite,
		&padkaprow_sprite,
		&sausage_sprite,
		&coffee_sprite,
		&salad_sprite
	};
	int num_food_sprites = sizeof(food_sprites) / sizeof(food_sprites[0]);

	food.sprite =
	eat_tracker == 4 ?
	&coin_world_sprite :
	food_sprites[CAT_rand_int(0, num_food_sprites-1)];
}

void CAT_MS_snake(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_snake);
			snake_init();
			food_init();
			ticks = 0;
			score = 0;
			is_high_score = false;
			eat_tracker = 0;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(!snake.dead)
			{
				if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
						CAT_gui_open_popup("Quit Snack?\n\nProgress will not be saved!\n\n");
				else if(CAT_gui_consume_popup())
				{
					CAT_machine_back();
					break;
				}
				if(CAT_gui_popup_is_open())
					break;

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

				if(ticks >= SNAKE_TICK_PERIOD)
				{
					snake.ldx = snake.dx;
					snake.ldy = snake.dy;
					int x = snake.xs[0] + snake.dx;
					int y = snake.ys[0] + snake.dy;

					snake.dead |= x < 0;
					snake.dead |= x >= GRID_WIDTH;
					snake.dead |= y < 0;
					snake.dead |= y >= GRID_HEIGHT;

					for(int i = 0; i < snake.length; i++)
					{
						snake.dead |=
						snake.xs[i] == x &&
						snake.ys[i] == y;
					}

					if(x == food.x && y == food.y)
					{	
						eat_tracker += 1;
						if(eat_tracker == 5)
						{
							CAT_inventory_add(coin_item, 1);
							eat_tracker = 0;
						}

						food_init();

						score += 1;
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
					
					ticks = 0;
				}
				ticks += 1;
			}
			else
			{
				if(score > snake_high_score)
				{
					snake_high_score = score;
					is_high_score = true;
				}

				if(CAT_input_pressed(CAT_BUTTON_A) || CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
					CAT_machine_back();
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
	if(!snake.dead)
	{
		CAT_frameberry(RGB8882565(122, 146, 57));
		for(int y = 0; y < 20; y += 2)
		{
			CAT_draw_tile_alpha(&floor_grass_tile_sprite, 17, grasses[y/2]*16, y*16);
		}

		int dx = snake.xs[0] - snake.xs[1];
		int dy = snake.ys[0] - snake.ys[1];
		int head_x = snake.xs[0] * 16;
		int head_y = snake.ys[0] * 16;

		if(dx == 1)
			CAT_draw_tile_alpha(&snake_head_sprite, 0, head_x, head_y);
		else if(dy == 1)
			CAT_draw_tile_alpha(&snake_head_sprite, 1, head_x, head_y);
		else if(dx == -1)
			CAT_draw_tile_alpha(&snake_head_sprite, 2, head_x, head_y);
		else if(dy == -1)
			CAT_draw_tile_alpha(&snake_head_sprite, 3, head_x, head_y);

		for(int i = 1; i < snake.length; i++)
		{
			int dbx = snake.xs[i-1] - snake.xs[i];
			int dby = snake.ys[i-1] - snake.ys[i];

			int body_x = snake.xs[i] * 16;
			int body_y = snake.ys[i] * 16;
			
			if(i == snake.length-1)
			{
				if(dbx == 1)
					CAT_draw_tile_alpha(&snake_tail_sprite, 0, body_x, body_y);
				else if(dby == 1)
					CAT_draw_tile_alpha(&snake_tail_sprite, 1, body_x, body_y);
				else if(dbx == -1)
					CAT_draw_tile_alpha(&snake_tail_sprite, 2, body_x, body_y);
				else if(dby == -1)
					CAT_draw_tile_alpha(&snake_tail_sprite, 3, body_x, body_y);
				break;
			}

			int dfx = snake.xs[i] - snake.xs[i+1];
			int dfy = snake.ys[i] - snake.ys[i+1];

			if(dfx == 1 && dbx == 1)
				CAT_draw_tile_alpha(&snake_body_sprite, 0, body_x, body_y);
			else if(dfy == 1 && dby == 1)
				CAT_draw_tile_alpha(&snake_body_sprite, 1, body_x, body_y);
			else if(dfx == -1 && dbx == -1)
				CAT_draw_tile_alpha(&snake_body_sprite, 2, body_x, body_y);
			else if(dfy == -1 && dby == -1)
				CAT_draw_tile_alpha(&snake_body_sprite, 3, body_x, body_y);
			else if((dfx == -1 && dby == 1) || (dfy == -1 && dbx == 1))
				CAT_draw_tile_alpha(&snake_corner_sprite, 0, body_x, body_y);
			else if((dfx == 1 && dby == 1) || (dfy == -1 && dbx == -1))
				CAT_draw_tile_alpha(&snake_corner_sprite, 1, body_x, body_y);
			else if((dfx == 1 && dby == -1) || (dfy == 1 && dbx == -1))
				CAT_draw_tile_alpha(&snake_corner_sprite, 2, body_x, body_y);
			else if((dfx == -1 && dby == -1) || (dfy == 1 && dbx == 1))
				CAT_draw_tile_alpha(&snake_corner_sprite, 3, body_x, body_y);
		}

		CAT_draw_tile_alpha(food.sprite, 0, food.x * 16, food.y * 16);
	}
	else
	{
		CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 20});
		
		for(int x = 0; x < 15; x++)
		{
			CAT_draw_tile_alpha(&snake_head_sprite, 1, x * CAT_TILE_SIZE, 0);
			CAT_draw_tile_alpha(&snake_head_sprite, 1, x * CAT_TILE_SIZE, 19 * CAT_TILE_SIZE);
		}
		for(int y = 0; y < 20; y++)
		{
			CAT_draw_tile_alpha(&snake_head_sprite, 1, 0,                  y * CAT_TILE_SIZE);
			CAT_draw_tile_alpha(&snake_head_sprite, 1, 14 * CAT_TILE_SIZE, y * CAT_TILE_SIZE);
		}

		CAT_gui_panel((CAT_ivec2) {1, 1}, (CAT_ivec2) {13, 18});
		CAT_gui_set_flag(CAT_GUI_FLAG_WRAPPED);
		CAT_gui_textf
		(
			"Rest in peace,\nSnack Cat.\n\n"
			"Your long journey has\ncome to a tragic end.\n\n"
			"Press A or B to return from whence you came.\n\n"
			"SCORE: %d\n"
			"HIGH SCORE: %d\n"
			, score,
			snake_high_score
		);

		if(is_high_score)
		{
			CAT_gui_text("New high score!");
		}
	}
}