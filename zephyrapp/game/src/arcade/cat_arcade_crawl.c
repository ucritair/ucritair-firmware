#include "cat_arcade.h"

#include "cat_input.h"
#include "cat_room.h"
#include "cat_render.h"
#include <math.h>

#ifdef CAT_DESKTOP
#define FPS 60
#else
#define FPS 20
#endif

#define SPEED 80.0f
static float pos_x = 120;
static float pos_y = 160;
static enum {N, E, S, W} dir = 2;
static int keys[4] = {CAT_BUTTON_UP, CAT_BUTTON_RIGHT, CAT_BUTTON_DOWN, CAT_BUTTON_LEFT};
static float dx[4] = {0, SPEED, 0, -SPEED};
static float dy[4] = {-SPEED, 0, SPEED, 0};

#define WALK_CYCLE_DURATION (FPS / 3)
static int walk_cycle_timer = 0;
static int walk_cycle_idx = 0;

#define ATTACK_DURATION (FPS / 6)
static int attack_timer = ATTACK_DURATION;
CAT_rect attack_rect = {{0, 0}, {32, 32}};

#define MAX_ENEMY_COUNT 100
static int enemy_id[MAX_ENEMY_COUNT];
static int enemy_x[MAX_ENEMY_COUNT];
static int enemy_y[MAX_ENEMY_COUNT];
static enum {ALIVE, DEAD} enemy_status[MAX_ENEMY_COUNT];
static int enemy_count;

void spawn(int x, int y)
{
	enemy_id[enemy_count] = enemy_count;
	enemy_x[enemy_count] = x;
	enemy_y[enemy_count] = y;
	enemy_count += 1;
}

void swap(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

void kill(int idx)
{
	enemy_count -= 1;
	swap(&enemy_id[idx], &enemy_id[enemy_count]);
	swap(&enemy_x[idx], &enemy_x[enemy_count]);
	swap(&enemy_y[idx], &enemy_y[enemy_count]);
}

void CAT_MS_crawl(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			for(int i = 0; i < 20; i++)
			{
				int x = CAT_rand_int(0, LCD_SCREEN_W-1);
				int y = CAT_rand_int(0, LCD_SCREEN_H-1);
				spawn(x, y);
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{	
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
			
			for(int i = 0; i < 4; i++)
			{
				if(CAT_input_held(keys[i], 0.0f))
				{
					if(i != dir)
					{
						if(attack_timer < ATTACK_DURATION)
							continue;
						float time_current = CAT_input_time(keys[dir]);
						float time_candidate = CAT_input_time(keys[i]);
						bool newer = time_current == 0 || time_candidate < time_current;
						if(!newer)
							continue;
					}
				
					dir = i;
					pos_x += dx[dir] * CAT_get_delta_time();
					pos_y += dy[dir] * CAT_get_delta_time();

					walk_cycle_timer++;
					if(walk_cycle_timer >= WALK_CYCLE_DURATION)
					{
						walk_cycle_idx = !walk_cycle_idx;
						walk_cycle_timer = 0;
					}
				}
			}

			if(CAT_input_pressed(CAT_BUTTON_B))
			{
				if(attack_timer >= ATTACK_DURATION)
					attack_timer = 0;
			}
			if(attack_timer < ATTACK_DURATION)
			{
				if(dir == 0)
					attack_rect = CAT_rect_center(pos_x, pos_y - 32, 32, 32);
				else if(dir == 1)
					attack_rect = CAT_rect_center(pos_x + 32, pos_y, 32, 32);
				else if(dir == 2)
					attack_rect = CAT_rect_center(pos_x, pos_y + 32, 32, 32);
				else
					attack_rect = CAT_rect_center(pos_x - 32, pos_y, 32, 32);

				if(attack_timer == 0)
				{
					for(int i = 0; i < enemy_count; i++)
					{
						CAT_rect enemy_rect = CAT_rect_center(enemy_x[i], enemy_y[i], 32, 32);
						if(CAT_rect_overlaps(attack_rect, enemy_rect))
						{
							kill(i);
						}
					}
				}

				attack_timer += 1;
			}
					
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

void CAT_render_crawl()
{
	CAT_frameberry(0x0000);

	spriter.mode = CAT_DRAW_MODE_CENTER_Y | CAT_DRAW_MODE_CENTER_X;

	int frame = 2 * dir + walk_cycle_idx;
	CAT_draw_sprite(&fighter_sprite, frame, round(pos_x), round(pos_y));

	for(int i = 0; i < enemy_count; i++)
	{
		CAT_draw_sprite(&enemy_sprite, 0, enemy_x[i], enemy_y[i]);
	}
	
	spriter.mode = CAT_DRAW_MODE_DEFAULT;
	if(attack_timer < ATTACK_DURATION)
	{
		CAT_draw_sprite(&attack_sprite, 0, attack_rect.min.x, attack_rect.min.y);
	}
}