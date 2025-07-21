#include "cat_world.h"

#include "cat_render.h"
#include "cat_input.h"
#include "sprite_assets.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_dialogue.h"
#include "dialogue_assets.h"
#include "cat_combat.h"

//////////////////////////////////////////////////////////////////////////
// INTERACTION

static CAT_interactable_list interactables;
static CAT_interactable* candidate_interactable = NULL;

CAT_interactable_list* CAT_world_get_interactables()
{
	return &interactables;
}

int CAT_world_place_interactable(const char* name, const CAT_sprite* sprite, int x, int y, int tx, int ty, void (*proc)())
{
	if(interactables.length >= CAT_WORLD_MAX_INTERACTABLE_COUNT)
		return -1;
	int idx = interactables.length;
	interactables.data[idx] = (CAT_interactable)
	{
		.name = name,
		.sprite = sprite,
		.x = clamp(x, 0, UINT16_MAX), .y =clamp(y, 0, UINT16_MAX),
		.w = sprite->width, .h = sprite->height,
		.tx = tx, .ty = ty,
		.proc = proc
	};
	interactables.length += 1;
	return idx;
}

void draw_interactables()
{
	for(int i = 0; i < interactables.length; i++)
	{
		CAT_interactable* I = &interactables.data[i];

		CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_BOTTOM);
		CAT_draw_sprite(I->sprite, -1, I->x, I->y);

		CAT_strokeberry(I->x - I->w/2, I->y - I->h, I->w, I->h, candidate_interactable == I ? CAT_BLUE : CAT_GREY);
		CAT_lineberry(I->x, I->y, I->x + I->tx * 16, I->y + I->ty * 16, CAT_GREEN);
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_text(I->x + I->w/2 + 2, I->y - I->h, I->name);
	}
}

//////////////////////////////////////////////////////////////////////////
// PLAYER

#define PLAYER_W (pet_world_walk_sprite.width)
#define PLAYER_H (pet_world_walk_sprite.height)
#define PLAYER_SPEED 64

static int player_x = CAT_WORLD_CENTER_X;
static int player_y = CAT_WORLD_CENTER_Y;
static int player_dx = 0;
static int player_dy = 0;
static int player_tx = 0;
static int player_ty = 1;

static uint8_t player_walk_row = 0;
static uint8_t player_walk_frame = 0;

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

void CAT_world_get_position(int* x, int* y)
{
	*x = player_x;
	*y = player_y;
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
		I->x, I->y-I->h,
		I->x + I->w, I->y,
		player_x, player_y-PLAYER_H,
		player_x + PLAYER_W, player_y
	);
}

void init_player()
{
	player_x = CAT_WORLD_CENTER_X;
	player_y = CAT_WORLD_CENTER_Y;
	player_dx = 0;
	player_dy = 0;
	player_tx = 0;
	player_ty = 1;

	player_walk_row = 0;
	player_walk_frame = 0;
}

void tick_player()
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

	if(player_dx != 0 || player_dy != 0)
	{
		player_tx = sgn(player_dx);
		player_ty = sgn(player_dy);

		if(player_tx == -1)
			player_walk_row = 0;
		else if(player_tx == 1)
			player_walk_row = 2;
		else if(player_ty == -1)
			player_walk_row = 1;
		else if(player_ty == 1)
			player_walk_row = 3;
		player_walk_frame = wrap(player_walk_frame+1, pet_world_walk_sprite.frame_count / 4);
	}

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
		else
		{
			CAT_attack_bullet(player_x, player_y, player_tx, player_ty, 180);
		}
	}
	if(CAT_input_pressed(CAT_BUTTON_B))
	{
		CAT_attack_swipe(player_x, player_y, player_tx, player_ty);
	}
}

void draw_player()
{
	CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_BOTTOM);
	CAT_draw_sprite(&pet_world_walk_sprite, player_walk_row * 2 + player_walk_frame, player_x, player_y);

	CAT_circberry(player_x, player_y, 2, CAT_RED);
	CAT_strokeberry(player_x - PLAYER_W/2, player_y - PLAYER_H, PLAYER_W, PLAYER_H, CAT_RED);
	CAT_lineberry(player_x, player_y, player_x + player_tx * 16, player_y + player_ty * 16, CAT_GREEN);
}

void npc_interact_proc()
{
	CAT_enter_dialogue(&dialogue_test_a);
}

void door_interact_proc()
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

			init_player();

			CAT_world_place_interactable
			(
				"Inspector Reed",
				&npc_reed_sprite,
				64, 64,
				0, 1,
				npc_interact_proc
			);
			CAT_world_place_interactable
			(
				"Exit Door",
				&null_sprite,
				128, 228,
				0, -1,
				door_interact_proc
			);
			candidate_interactable = NULL;

			for(int i = 0; i < 5; i++)
			{
				CAT_spawn_enemy(CAT_rand_int(CAT_WORLD_X, CAT_WORLD_MAX_X), CAT_rand_int(CAT_WORLD_Y, CAT_WORLD_MAX_Y));
			}
		}
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_in_dialogue())
			{
				CAT_dialogue_io();
				break;
			}

			tick_player();
			CAT_tick_enemies();
			CAT_tick_attacks();
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
	CAT_frameberry(CAT_GREY);
	CAT_strokeberry(CAT_WORLD_X, CAT_WORLD_Y, CAT_WORLD_W, CAT_WORLD_H, CAT_WHITE);

	draw_interactables();
	draw_player();

	CAT_render_enemies();
	CAT_render_attacks();

	if(CAT_in_dialogue())
	{
		CAT_render_dialogue();
	}	
}