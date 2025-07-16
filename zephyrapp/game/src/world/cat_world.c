#include "cat_world.h"

#include "cat_render.h"
#include "cat_input.h"
#include "sprite_assets.h"
#include "cat_gui.h"

//////////////////////////////////////////////////////////////////////////
// MOVEMENT

#define PLAYER_W 64
#define PLAYER_H 48
#define PLAYER_SPEED 64

static int player_x = CAT_WORLD_CENTER_X;
static int player_y = CAT_WORLD_CENTER_Y;
static int player_dx = 0;
static int player_dy = 0;

void CAT_world_move_to(int x, int y)
{
	player_dx = x - player_x;
	player_dx = y - player_y;
}

void CAT_world_move_by(int dx, int dy)
{
	player_dx += dx;
	player_dy += dy;
}


//////////////////////////////////////////////////////////////////////////
// INTERACTION

static CAT_interactable_list interactables;

CAT_interactable_list* CAT_world_get_interactables()
{
	return &interactables;
}

int CAT_world_place_interactable(CAT_interactable interactable)
{
	if(interactables.length >= CAT_WORLD_MAX_INTERACTABLE_COUNT)
		return -1;
	int idx = interactables.length;
	interactables.data[idx] = interactable;
	interactables.length += 1;
	return idx;
}

void dummy_interact_proc()
{
	CAT_gui_open_dialogue("Hello, world!\n", 2);
}

void CAT_MS_world(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_world);

			player_x = CAT_WORLD_CENTER_X;
			player_y = CAT_WORLD_CENTER_Y;

			CAT_world_place_interactable
			(
				(CAT_interactable)
				{
					.x = 64,
					.y = 64,
					.w = 24,
					.h = 48,
					.sprite = &null_sprite,
					.proc = dummy_interact_proc,
				}
			);
		}
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_held(CAT_BUTTON_UP, 0))
				CAT_world_move_by(0, -PLAYER_SPEED * CAT_get_delta_time_s());
			if(CAT_input_held(CAT_BUTTON_DOWN, 0))
				CAT_world_move_by(0, PLAYER_SPEED * CAT_get_delta_time_s());
			if(CAT_input_held(CAT_BUTTON_LEFT, 0))
				CAT_world_move_by(-PLAYER_SPEED * CAT_get_delta_time_s(), 0);
			if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
				CAT_world_move_by(PLAYER_SPEED  * CAT_get_delta_time_s(), 0);

			player_x += player_dx;
			player_y += player_dy;
			player_x = clamp(player_x, CAT_WORLD_X+PLAYER_W/2, CAT_WORLD_MAX_X-PLAYER_W/2);
			player_y = clamp(player_y, CAT_WORLD_Y+PLAYER_H, CAT_WORLD_MAX_Y);
			player_dx = 0;
			player_dy = 0;
		}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		{

		}
		break;
	}
}

void CAT_render_world()
{
	CAT_frameberry(CAT_BLACK);
	CAT_strokeberry(CAT_WORLD_X, CAT_WORLD_Y, CAT_WORLD_W, CAT_WORLD_H, CAT_WHITE);

	for(int i = 0; i < interactables.length; i++)
	{
		CAT_interactable I = interactables.data[i];
		CAT_strokeberry(I.x - I.w/2, I.y - I.h, I.w, I.h, CAT_BLUE);
	}

	CAT_circberry(player_x, player_y, 2, CAT_RED);
	CAT_strokeberry(player_x - PLAYER_W/2, player_y - PLAYER_H, PLAYER_W, PLAYER_H, CAT_RED);
}