#include "cat_world.h"

#include "cat_render.h"
#include "cat_input.h"
#include "sprite_assets.h"
#include "cat_gui.h"
#include "cat_room.h"

//////////////////////////////////////////////////////////////////////////
// MOVEMENT

#define PLAYER_W 64
#define PLAYER_H 48
#define PLAYER_SPEED 64

static int player_x = CAT_WORLD_CENTER_X;
static int player_y = CAT_WORLD_CENTER_Y;
static int player_dx = 0;
static int player_dy = 0;
static int player_tx = 0;
static int player_ty = 1;

void CAT_world_move_to(int x, int y)
{
	player_dx = x - player_x;
	player_dx = y - player_y;
	player_tx = sgn(player_dx);
	player_ty = sgn(player_dy);
}

void CAT_world_move_by(int dx, int dy)
{
	player_dx += dx;
	player_dy += dy;
	player_tx = sgn(player_dx);
	player_ty = sgn(player_dy);
}


//////////////////////////////////////////////////////////////////////////
// INTERACTION

static CAT_interactable_list interactables;
static CAT_interactable* candidate_interactable = NULL;

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

bool facing_interactable(CAT_interactable* I)
{
	if(I->ty != 0 && sgn(player_ty) != -sgn(I->ty))
		return false;
	if(I->ty != 0 && sgn(player_y - I->y) != sgn(I->ty))
		return false;
	if(I->tx != 0 && sgn(player_tx) != -sgn(I->tx))
		return false;
	if(I->tx != 0 && sgn(player_x - I->x) != sgn(I->tx))
		return false;
	return true;
}

bool touching_interactable(CAT_interactable* I)
{
	return
	CAT_int4_int4_intersects
	(
		I->x, I->y,
		I->x + I->w, I->y + I->h,
		player_x, player_y,
		player_x + PLAYER_W, player_y + PLAYER_H
	);
}

void dummy_interact_proc()
{
	CAT_gui_open_dialogue("Hello, world!\n", 2);
}

void exit_door_interact_proc()
{
	CAT_machine_transition(CAT_MS_room);
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
			player_dx = 0;
			player_dy = 0;
			player_tx = 0;
			player_ty = 1;

			CAT_world_place_interactable
			(
				(CAT_interactable)
				{
					"Skorkus",
					.x = 64, .y = 64,
					.w = 24, .h = 48,
					.tx = 0, .ty = 1,
					.sprite = &null_sprite,
					.proc = dummy_interact_proc,
				}
			);
			CAT_world_place_interactable
			(
				(CAT_interactable)
				{
					"Exit Door",
					.x = 128, .y = 228,
					.w = 24, .h = 48,
					.tx = 0, .ty = -1,
					.sprite = &null_sprite,
					.proc = exit_door_interact_proc,
				}
			);
			candidate_interactable = NULL;
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

			candidate_interactable = NULL;
			for(int i = 0; i < interactables.length; i++)
			{
				CAT_interactable* I = &interactables.data[i];

				if(!facing_interactable(I))
					continue;
				if(!touching_interactable(I))
					continue;

				candidate_interactable = I;
			}

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				if(candidate_interactable != NULL)
					candidate_interactable->proc();
			}
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
		CAT_interactable* I = &interactables.data[i];
		CAT_strokeberry(I->x - I->w/2, I->y - I->h, I->w, I->h, candidate_interactable == I ? CAT_BLUE : CAT_GREY);
		CAT_lineberry(I->x, I->y, I->x + I->tx * 16, I->y + I->ty * 16, CAT_GREEN);
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_text(I->x + I->w/2 + 2, I->y - I->h, I->name);
	}

	CAT_circberry(player_x, player_y, 2, CAT_RED);
	CAT_strokeberry(player_x - PLAYER_W/2, player_y - PLAYER_H, PLAYER_W, PLAYER_H, CAT_RED);
	CAT_lineberry(player_x, player_y, player_x + player_tx * 16, player_y + player_ty * 16, CAT_GREEN);
}