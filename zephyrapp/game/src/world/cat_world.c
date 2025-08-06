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

#define PLAYER_W (world_walk_sprite.width)
#define PLAYER_H (world_walk_sprite.height)
#define PLAYER_SPEED 64

static int player_x = 0;
static int player_y = 0;

static CAT_button movement_buttons[4] =
{
	CAT_BUTTON_UP,
	CAT_BUTTON_RIGHT,
	CAT_BUTTON_DOWN,
	CAT_BUTTON_LEFT
};
static int movement_deltas[4][2] =
{
	{0, -1},
	{1, 0},
	{0, 1},
	{-1, 0}
};
static enum
{
	NORTH,
	EAST,
	SOUTH,
	WEST
} player_direction = WEST;

static int player_dx = 0;
static int player_dy = 0;
static int player_x_remainder = 0;
static int player_y_remainder = 0;

static int player_step_frame = 0;
static int step_frame_counter = 0;

static CAT_scene_index* interactable = NULL;

void CAT_world_get_position(int* x, int* y)
{
	*x = player_x;
	*y = player_y;
}

void player_init()
{
	player_x = 0;
	player_y = 0;

	player_direction = WEST;
	player_dx = 0;
	player_dy = 0;
	player_x_remainder = 0;
	player_y_remainder = 0;

	player_step_frame = 0;
	step_frame_counter = 0;
}

bool is_walking()
{
	return player_x_remainder != 0 || player_y_remainder != 0;
}

void player_motion_input()
{
	int direction_last = player_direction;
	int frames = CAT_input_frames(movement_buttons[player_direction]);

	for(int i = 0; i < 4; i++)
	{
		if(CAT_input_held(movement_buttons[i], 0.0f))
		{
			if(i != player_direction)
			{
				int frames_current = CAT_input_frames(movement_buttons[player_direction]);
				int frames_candidate = CAT_input_frames(movement_buttons[i]);
				bool newer = frames_current == 0 || frames_candidate < frames_current;
				if(!newer)
					continue;
				player_direction = i;
				player_dx = movement_deltas[i][0];
				player_dy = movement_deltas[i][1];
				frames = frames_candidate;
			}
		}
	}

	if(frames == 1)
	{
		if(player_direction != direction_last)
			player_step_frame = !player_step_frame;
		step_frame_counter = 2;
	}
	else if(frames >= 2 && !is_walking())
	{
		player_x_remainder = abs(player_dx) * 8;
		player_y_remainder = abs(player_dy) * 8;
	}

	if(is_walking())
	{
		if(step_frame_counter == 2)
		{
			player_step_frame = !player_step_frame;
			step_frame_counter = 0;
		}
		step_frame_counter += 1;
	}
}

void player_get_aabb(int* x0, int* y0, int* x1, int* y1)
{
	*x0 = player_x - PLAYER_W/4;
	*y0 = player_y;
	*x1 = player_x + PLAYER_W/4;
	*y1 = player_y + PLAYER_W/2;
}

void player_motion_logic()
{
	if(player_x_remainder > 0)
	{
		player_x += player_dx * 4;
		player_x_remainder = max(player_x_remainder-4, 0);
	}
	if(player_y_remainder > 0)
	{
		player_y += player_dy * 4;
		player_y_remainder = max(player_y_remainder-4, 0);
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

	if(direction[1] != 0 && sgn(player_dy) != -sgn(direction[1]))
		return false;
	if(direction[1] != 0 && sgn(player_y - center_y) != sgn(direction[1]))
		return false;
	if(direction[0] != 0 && sgn(player_dx) != -sgn(direction[0]))
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
			const CAT_prop* prop = test_scene.layers[interactable->layer].props[interactable->prop].prop;
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

int player_get_walk_frame()
{
	switch (player_direction)
	{
		case NORTH: return 2;
		case EAST: return 4;
		case SOUTH: return 6;
		case WEST: return 0; 
	}
}

void draw_player()
{
	int view_x = CAT_LCD_SCREEN_W/2;
	int view_y = CAT_LCD_SCREEN_H/2;

	int x0, y0, x1, y1;
	player_get_aabb(&x0, &y0, &x1, &y1);
	CAT_strokeberry(x0-player_x+view_x, y0-player_y+view_y, x1-x0, y1-y0, CAT_WHITE);

	int frame = player_get_walk_frame() + player_step_frame;
	CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	CAT_draw_sprite(&world_walk_sprite, frame, view_x, view_y);	
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

static void viewport_transform(int x, int y, int eye_x, int eye_y, int view_w, int view_h, int* x_out, int* y_out)
{
	*x_out = x - eye_x + CAT_LCD_SCREEN_W/2;
	*y_out = y - eye_y + CAT_LCD_SCREEN_H/2;
}

void CAT_render_world()
{
	CAT_frameberry(GRASS_COLOUR);

	bool drew_player = false;
	for(int i = 0; i < test_scene.layer_count; i++)
	{
		struct layer* layer = &test_scene.layers[i];
		for(int j = 0; j < layer->prop_count; j++)
		{
			struct prop* prop = &layer->props[j];

			if(!drew_player)
			{
				int player_by = player_y-PLAYER_H/2 + world_walk_sprite.height;
				int prop_by = prop->position_y + prop->prop->sprite->height;
				if(player_by < prop_by)
				{
					draw_player();
					drew_player = true;
				}
			}

			int x, y;
			viewport_transform(prop->position_x, prop->position_y, player_x, player_y, CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H, &x, &y);
			CAT_draw_sprite_raw(prop->prop->sprite, -1, x, y);
		}
	}
	if(!drew_player)
		draw_player();

	if(CAT_in_dialogue())
	{
		CAT_render_dialogue();
	}
}