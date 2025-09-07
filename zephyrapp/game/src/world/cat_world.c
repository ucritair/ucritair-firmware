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
#include "prop_assets.h"

//////////////////////////////////////////////////////////////////////////
// PLAYER

#define PLAYER_SPRITE_W (world_walk_sprite.width)
#define PLAYER_SPRITE_H (world_walk_sprite.height)
#define PLAYER_W 1
#define PLAYER_H 1
#define WALK_FRAME_PERIOD 2

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
} player_direction = SOUTH;

static int player_dx = 0;
static int player_dy = 0;
static int player_x_slide = 0;
static int player_y_slide = 0;

static int player_step_frame = 0;
static int step_frame_counter = 0;

static CAT_scene_index* interactable = NULL;
static CAT_scene_index* blocker = NULL;

void CAT_world_get_position(int* x, int* y)
{
	*x = player_x;
	*y = player_y;
}

void player_init()
{
	player_x = 0;
	player_y = 0;

	player_direction = SOUTH;
	player_dx = 0;
	player_dy = 0;
	player_x_slide = 0;
	player_y_slide = 0;

	step_frame_counter = 0;
}

void player_get_aabb(int* x0, int* y0, int* x1, int* y1)
{
	*x0 = player_x;
	*y0 = player_y;
	*x1 = player_x + PLAYER_W;
	*y1 = player_y + PLAYER_H;
}

void player_get_forward_aabb(int* x0, int* y0, int* x1, int* y1)
{
	*x0 = player_x + player_dx;
	*y0 = player_y + player_dy;
	*x1 = player_x + PLAYER_W + player_dx;
	*y1 = player_y + PLAYER_H + player_dy;
}

bool is_walking()
{
	return player_x_slide != 0 || player_y_slide != 0;
}

bool is_blocked()
{
	if(blocker != NULL)
		return true;
	int x0, y0, x1, y1;
	player_get_forward_aabb(&x0, &y0, &x1, &y1);
	if(x0 <= test_scene.bounds[0])
		return true;
	if(y0 <= test_scene.bounds[1])
		return true;
	if(x1 > test_scene.bounds[2])
		return true;
	if(y1 > test_scene.bounds[3])
		return true;
	return false;
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
				if(!newer || is_walking())
					continue;
				player_direction = i;
				player_dx = movement_deltas[i][0];
				player_dy = movement_deltas[i][1];
				frames = frames_candidate;
				return;
			}
		}
	}

	if(frames == 1)
	{
		if(player_direction != direction_last)
			player_step_frame = !player_step_frame;
		step_frame_counter = WALK_FRAME_PERIOD;
	}
	else if(frames >= 2 && !is_walking() && !is_blocked())
	{
		player_x_slide = abs(player_dx) * CAT_TILE_SIZE;
		player_y_slide = abs(player_dy) * CAT_TILE_SIZE;
	}

	if(is_walking())
	{
		if(step_frame_counter == WALK_FRAME_PERIOD)
		{
			player_step_frame = !player_step_frame;
			step_frame_counter = 0;
		}
		step_frame_counter += 1;
	}
}

void player_motion_logic()
{
	if(player_x_slide > 0)
	{
		player_x_slide = max(player_x_slide-4, 0);
		if(player_x_slide == 0)
			player_x += player_dx;
	}
	if(player_y_slide > 0)
	{
		player_y_slide = max(player_y_slide-4, 0);
		if(player_y_slide == 0)
			player_y += player_dy;
	}
}
	

static CAT_scene_index* collisions;
static int collision_count = 0;

void player_collision()
{
	interactable = NULL;
	blocker = NULL;

	int x0, y0, x1, y1;
	player_get_forward_aabb(&x0, &y0, &x1, &y1);
	collisions = CAT_detect_collisions(&test_scene, x0, y0, x1, y1, &collision_count);

	for(int i = 0; i < collision_count; i++)
	{
		CAT_scene_index* col = &collisions[i];	
		
		if(col->leaf == TRIGGER)
			interactable = col;
		if(col->leaf == BLOCKER)
			blocker = col;
	}
}

bool facing_interactable()
{
	if(interactable == NULL)
		return false;

	CAT_scene_vector direction;
	CAT_scene_get_direction(&test_scene, interactable, direction);

	if(player_dx != 0 && player_dx != -direction[0])
		return false;
	if(player_dy != 0 && player_dy != -direction[1])
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
	player_collision();
	player_motion_input();
	player_motion_logic();
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

void CAT_MS_world(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_world);
			player_init();
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
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
				CAT_pushdown_transition(CAT_MS_menu);
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		{

		}
		break;
	}
}

static int eye_x;
static int eye_y;

static void position_eye()
{
	eye_x = player_x * CAT_TILE_SIZE + CAT_TILE_SIZE/2;
	eye_y = player_y * CAT_TILE_SIZE + CAT_TILE_SIZE/2;
	if(is_walking())
	{
		eye_x += (CAT_TILE_SIZE - player_x_slide) * player_dx;
		eye_y += (CAT_TILE_SIZE - player_y_slide) * player_dy;
	}
}

static void view_transform_point(int x, int y, int* x_out, int* y_out)
{
	*x_out = x * CAT_TILE_SIZE - eye_x + CAT_LCD_SCREEN_W/2;
	*y_out = y * CAT_TILE_SIZE- eye_y + CAT_LCD_SCREEN_H/2;
}

static void view_transform_AABB(int x0, int y0, int x1, int y1, int* x0_out, int* y0_out, int* x1_out, int* y1_out)
{
	view_transform_point(x0, y0, x0_out, y0_out);
	view_transform_point(x1, y1, x1_out, y1_out);
}

void draw_player()
{
	int frame = player_get_walk_frame() + player_step_frame;
	CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	CAT_draw_sprite(&world_walk_sprite, frame, CAT_LCD_SCREEN_W/2, CAT_LCD_SCREEN_H/2);

	/*int x0, y0, x1, y1;
	player_get_aabb(&x0, &y0, &x1, &y1);
	view_transform_AABB(x0, y0, x1, y1, &x0, &y0, &x1, &y1);
	CAT_strokeberry(x0, y0, x1-x0, y1-y0, CAT_WHITE);
	CAT_circberry(CAT_LCD_SCREEN_W/2, CAT_LCD_SCREEN_H/2, 2, CAT_RED);*/
}

void CAT_render_world()
{
	position_eye();

	CAT_frameberry(test_scene.background.colour);

	for(int i = 0; i < test_scene.background.tile_count; i++)
	{
		struct tile* tile = &test_scene.background.tiles[i];
		int x, y;
		view_transform_point(tile->x, tile->y, &x, &y);
		CAT_draw_sprite_raw(test_scene.background.palette, tile->frame, x, y);
	}

	bool drew_player = false;
	for(int i = 0; i < test_scene.layer_count; i++)
	{
		struct layer* layer = &test_scene.layers[i];
		for(int j = 0; j < layer->prop_count; j++)
		{
			struct prop* prop = &layer->props[j];

			const CAT_sprite* sprite = prop->prop->sprite;
			if(!CAT_rect_rect_intersecting(
				eye_x-CAT_LCD_SCREEN_W/2, eye_y-CAT_LCD_SCREEN_H/2,
				eye_x+CAT_LCD_SCREEN_W/2, eye_y+CAT_LCD_SCREEN_H/2,
				prop->position_x*CAT_TILE_SIZE, prop->position_y*CAT_TILE_SIZE,
				prop->position_x*CAT_TILE_SIZE + sprite->width, prop->position_y*CAT_TILE_SIZE + sprite->height
			))
			{
				continue;
			}

			if(!drew_player && i == 0)
			{
				int player_by = eye_y + world_walk_sprite.height/2-5;
				int prop_by = (prop->position_y * CAT_TILE_SIZE) + sprite->height;
				if(player_by < prop_by)
				{
					draw_player();
					drew_player = true;
				}
			}

			int x, y;
			view_transform_point(prop->position_x, prop->position_y, &x, &y);
			CAT_draw_sprite_raw(sprite, prop->variant, x, y);
			
			/*for(int k = 0; k < prop->prop->blocker_count; k++)
			{
				CAT_scene_AABB blocker;
				CAT_scene_index index = {.leaf = BLOCKER, .layer = i, .prop = j, .blocker = k};
				CAT_scene_get_AABB(&test_scene, &index, blocker);

				int x0, y0, x1, y1;
				view_transform_AABB(blocker[0], blocker[1], blocker[2], blocker[3], &x0, &y0, &x1, &y1);
				CAT_strokeberry(x0, y0, x1-x0, y1-y0, CAT_RED);
			}*/
		}
		if(!drew_player && i == 0)
			draw_player();
	}

	if(CAT_in_dialogue())
	{
		CAT_render_dialogue();
	}
}