#include "cat_arcade.h"

#include "cat_input.h"
#include "cat_room.h"
#include "cat_render.h"

#define SPEED 80.0f
static CAT_vec2 pos = {120, 160};
static enum {N, E, S, W} dir = 2;
static int keys[4] = {CAT_BUTTON_UP, CAT_BUTTON_RIGHT, CAT_BUTTON_DOWN, CAT_BUTTON_LEFT};
static float dx[4] = {0, SPEED, 0, -SPEED};
static float dy[4] = {-SPEED, 0, SPEED, 0};

static int frame_count = 0;
static int frame_switch = 0;

#define ATTACK_DURATION 20
static int attack_timer = ATTACK_DURATION;

void CAT_MS_crawl(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
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
					pos.x += dx[dir] * CAT_get_delta_time();
					pos.y += dy[dir] * CAT_get_delta_time();

					frame_count++;
					if(frame_count >= 20)
					{
						frame_switch = !frame_switch;
						frame_count = 0;
					}
				}
			}

			if(CAT_input_pressed(CAT_BUTTON_B))
			{
				if(attack_timer >= ATTACK_DURATION)
					attack_timer = 0;
			}
			if(attack_timer < ATTACK_DURATION)
				attack_timer += 1;
				
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

	int frame = 2 * dir + frame_switch;
	CAT_draw_sprite(&fighter_sprite, frame, (int) pos.x, (int) pos.y);

	if(attack_timer < ATTACK_DURATION)
	{
		int attack_x = pos.x;
		if(dir == 1)
			attack_x += 32;
		else if(dir == 3)
			attack_x -= 32;
		int attack_y = pos.y;
		if(dir == 0)
			attack_y -= 32;
		if(dir == 2)
			attack_y += 32;
		CAT_draw_sprite(&attack_sprite, 0, attack_x, attack_y);
	}
}