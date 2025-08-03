#include "cat_world.h"

#include "cat_render.h"
#include "cat_input.h"
#include "sprite_assets.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_dialogue.h"
#include "dialogue_assets.h"
#include "cat_combat.h"
#include "cat_scene.h"
#include "scene_assets.h"
#include "cat_menu.h"

//////////////////////////////////////////////////////////////////////////
// PLAYER

#define PLAYER_W (pet_world_walk_sprite.width)
#define PLAYER_H (pet_world_walk_sprite.height)
#define PLAYER_SPEED 64

static int player_x = 0;
static int player_y = 0;
static int player_dx = 0;
static int player_dy = 0;
static int player_tx = 0;
static int player_ty = 1;

static uint8_t player_walk_row = 0;
static uint8_t player_walk_frame = 0;

static CAT_scene_index* interactable = NULL;

void player_move_to(int x, int y)
{
	player_dx = x - player_x;
	player_dx = y - player_y;
}

void player_move_by(int dx, int dy)
{
	player_dx += dx;
	player_dy += dy;
}

void CAT_world_get_position(int* x, int* y)
{
	*x = player_x;
	*y = player_y;
}

void player_init()
{
	player_x = 0;
	player_y = 0;
	player_dx = 0;
	player_dy = 0;
	player_tx = 0;
	player_ty = 1;

	player_walk_row = 0;
	player_walk_frame = 0;	
}

void player_motion_input()
{
	player_dx = 0;
	player_dy = 0;

	if(CAT_input_held(CAT_BUTTON_UP, 0))
		player_move_by(0, -PLAYER_SPEED * CAT_get_delta_time_s());
	if(CAT_input_held(CAT_BUTTON_DOWN, 0))
		player_move_by(0, PLAYER_SPEED * CAT_get_delta_time_s());
	if(CAT_input_held(CAT_BUTTON_LEFT, 0))
		player_move_by(-PLAYER_SPEED * CAT_get_delta_time_s(), 0);
	if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
		player_move_by(PLAYER_SPEED  * CAT_get_delta_time_s(), 0);
}

void player_get_aabb(int* x0, int* y0, int* x1, int* y1)
{
	*x0 = player_x - PLAYER_W/4;
	*y0 = player_y - PLAYER_H/4;
	*x1 = player_x + PLAYER_W/4;
	*y1 = player_y + PLAYER_H/4;
}

void player_hand(int* x, int* y)
{
	*x = player_x + player_tx * PLAYER_W/2;
	if(player_ty == 0)
		*y = player_y - PLAYER_H/2;
	else if(player_ty == 1)
		*y = player_y;
	else
		*y = player_y - PLAYER_H;
}

void player_motion_logic()
{
	player_x += player_dx;
	player_y += player_dy;

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
}

static CAT_scene_index* collisions;
static int collision_count = 0;

void player_collision()
{
	interactable = NULL;

	int x0, y0, x1, y1;
	player_get_aabb(&x0, &y0, &x1, &y1);
	collisions = CAT_detect_collisions(&test_scene, x0, y0, x1, y1, &collision_count);

	for(int i = 0; i < collision_count; i++)
	{
		CAT_scene_index* col = &collisions[i];
		
		if(col->leaf == TRIGGER)
			interactable = col;

		if(col->leaf == BLOCKER)
		{
			CAT_scene_AABB aabb;
			CAT_scene_get_AABB(&test_scene, col, aabb);
			int sep_x = max(x0, aabb[0]) - min(x1, aabb[2]);
			int sep_y = max(y0, aabb[1]) - min(y1, aabb[3]);
			if(sep_x != 0 && sep_y != 0)
			{
				if(abs(sep_x) > abs(sep_y))
				{
					player_y += sep_y * sgn(player_dy);
					player_get_aabb(&x0, &y0, &x1, &y1);
					if
					(
						CAT_rect_rect_intersecting
						(
							aabb[0], aabb[1], aabb[2], aabb[3],
							x0, y0, x1, y1
						)
					)
					{
						player_x += sep_x * sgn(player_dx);
					}
				}
				else
				{				
					player_x += sep_x * sgn(player_dx);
					player_get_aabb(&x0, &y0, &x1, &y1);
					if
					(
						CAT_rect_rect_intersecting
						(
							aabb[0], aabb[1], aabb[2], aabb[3],
							x0, y0, x1, y1
						)
					)
					{
						player_y += sep_y * sgn(player_dy);
					}
				}
				
			}
		}
	}
}

bool facing_interactable()
{
	if(interactable == NULL)
		return false;

	CAT_scene_AABB aabb;
	CAT_scene_get_AABB(&test_scene, interactable, aabb);
	CAT_scene_vector direction;
	CAT_scene_get_direction(&test_scene, interactable, direction);

	int center_x = (aabb[0]+aabb[2])/2;
	int center_y = (aabb[1]+aabb[3])/2;

	if(direction[1] != 0 && sgn(player_ty) != -sgn(direction[1]))
		return false;
	if(direction[1] != 0 && sgn(player_y - center_y) != sgn(direction[1]))
		return false;
	if(direction[0] != 0 && sgn(player_tx) != -sgn(direction[0]))
		return false;
	if(direction[0] != 0 && sgn(player_x - center_x) != sgn(direction[0]))
		return false;
	return true;
}

void player_interaction_logic()
{
	if(interactable == NULL)
		return;

	if(CAT_input_pressed(CAT_BUTTON_A))
	{
		if(facing_interactable())
		{
			CAT_prop* prop = test_scene.layers[interactable->layer].props[interactable->prop].prop;
			void (*proc)() = prop->triggers[interactable->trigger].proc;
			if (proc != NULL)
				proc();
		}
	}
}

void tick_player()
{
	player_motion_input();
	player_motion_logic();
	player_collision();
	player_interaction_logic();
}

void draw_player()
{
	int view_x = CAT_LCD_SCREEN_W/2;
	int view_y = CAT_LCD_SCREEN_H/2;

	int x0, y0, x1, y1;
	player_get_aabb(&x0, &y0, &x1, &y1);
	CAT_strokeberry(x0-player_x+view_x, y0-player_y+view_y, x1-x0, y1-y0, CAT_WHITE);
	
	CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	CAT_draw_sprite(&pet_world_walk_sprite, player_walk_row * 2 + player_walk_frame, view_x, view_y);	
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
			player_init();
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

			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_menu);
		}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		{

		}
		break;
	}
}

#define GRASS_COLOUR RGB8882565(122, 146, 57)

void CAT_render_world()
{
	CAT_frameberry(GRASS_COLOUR);

	CAT_render_scene(&test_scene, player_x, player_y);

	draw_player();

	if(CAT_in_dialogue())
	{
		CAT_render_dialogue();
	}
}