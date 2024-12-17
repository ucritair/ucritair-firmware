#include "cat_arcade.h"

#include "cat_input.h"
#include "cat_room.h"
#include "cat_sprite.h"

static CAT_vec2 pos = {120, 160};
static enum {N, E, S, W} dir = 2;
static float speed = 80.0f;

static int frame_count = 0;
static int frame_switch = 0;

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
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
			
			if(CAT_input_held(CAT_BUTTON_UP, 0.0f))
			{
				pos.y -= speed * CAT_get_delta_time();
				dir = N;
				frame_count++;
			}
			else if(CAT_input_held(CAT_BUTTON_RIGHT, 0.0f))
			{
				pos.x += speed * CAT_get_delta_time();
				dir = E;
				frame_count++;
			}
			else if(CAT_input_held(CAT_BUTTON_DOWN, 0.0f))
			{
				pos.y += speed * CAT_get_delta_time();
				dir = S;
				frame_count++;
			}
			else if(CAT_input_held(CAT_BUTTON_LEFT, 0.0f))
			{
				pos.x -= speed * CAT_get_delta_time();
				dir = W;
				frame_count++;
			}

			if(frame_count >= 20)
			{
				frame_switch = !frame_switch;
				frame_count = 0;
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
	int frame = 2 * dir + frame_switch;
	CAT_draw_sprite(fighter_sprite, frame, (int) pos.x, (int) pos.y);
}