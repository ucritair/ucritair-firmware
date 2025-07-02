#pragma once

#include "cat_machine.h"
#include "cat_math.h"
#include "cat_render.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_ROOM_GRID_X 0
#define CAT_ROOM_GRID_Y 7
#define CAT_ROOM_GRID_W 15
#define CAT_ROOM_GRID_H 10
#define CAT_ROOM_GRID_SIZE (CAT_ROOM_GRID_W * CAT_ROOM_GRID_H)
#define CAT_ROOM_GRID_MAX_X (CAT_ROOM_GRID_X + CAT_ROOM_GRID_W-1)
#define CAT_ROOM_GRID_MAX_Y (CAT_ROOM_GRID_Y + CAT_ROOM_GRID_H-1)

#define CAT_ROOM_X (CAT_ROOM_GRID_X * CAT_TILE_SIZE)
#define CAT_ROOM_Y (CAT_ROOM_GRID_Y * CAT_TILE_SIZE)
#define CAT_ROOM_W (CAT_ROOM_GRID_W * CAT_TILE_SIZE)
#define CAT_ROOM_H (CAT_ROOM_GRID_H * CAT_TILE_SIZE)
#define CAT_ROOM_MAX_X (CAT_ROOM_X + CAT_ROOM_W-1)
#define CAT_ROOM_MAX_Y (CAT_ROOM_Y + CAT_ROOM_H-1)

#define CAT_MAX_PICKUP_COUNT 128


//////////////////////////////////////////////////////////////////////////
// ROOM

typedef struct CAT_room_theme
{
	const char* name;

	const CAT_sprite* wall_tiles;
	bool tile_wall;
	uint8_t* wall_map;
	CAT_rect window_rect;

	const CAT_sprite* floor_tiles;
	bool tile_floor;
	uint8_t* floor_map;
} CAT_room_theme;

typedef struct CAT_pickup
{
	CAT_ivec2 origin;
	CAT_ivec2 place;

	const CAT_sprite* sprite;
	void (*proc)();

	float timer;
} CAT_pickup;

typedef struct CAT_room
{
	const CAT_room_theme* theme;

	int prop_ids[CAT_ROOM_GRID_SIZE];
	CAT_rect prop_rects[CAT_ROOM_GRID_SIZE];
	int prop_overrides[CAT_ROOM_GRID_SIZE];
	int prop_children[CAT_ROOM_GRID_SIZE];
	int prop_count;

	CAT_pickup pickups[CAT_MAX_PICKUP_COUNT];
	int pickup_count;
} CAT_room;
extern CAT_room room;

void CAT_room_init();

CAT_ivec2 CAT_grid2world(CAT_ivec2 grid);
CAT_ivec2 CAT_world2grid(CAT_ivec2 world);

bool CAT_is_grid_point_free(CAT_ivec2 point);
bool CAT_is_world_point_free(CAT_vec2 point);
bool CAT_is_block_free(CAT_rect block);
void CAT_toggle_block(CAT_rect block, bool value);

bool CAT_has_free_space();
CAT_ivec2 CAT_first_free_space();
CAT_ivec2 CAT_rand_free_space();
CAT_ivec2 CAT_nearest_free_space(CAT_ivec2 cell);
CAT_ivec2 CAT_largest_free_space();

int CAT_room_find(int item_id);
int CAT_room_find_spatial(CAT_ivec2 place);
bool CAT_prop_fits(int item_id, CAT_ivec2 place);

int CAT_room_add_prop(int item_id, CAT_ivec2 place);
void CAT_room_stack_prop(int idx, int item_id);
void CAT_room_unstack_prop(int idx);
void CAT_room_remove_prop(int idx);
void CAT_room_flip_prop(int idx);

int CAT_spawn_pickup(CAT_ivec2 origin, CAT_ivec2 place, const CAT_sprite* sprite, void (*proc)());
void CAT_despawn_pickup(int idx);

void CAT_room_tick();
void CAT_MS_room(CAT_machine_signal signal);
void CAT_room_draw_statics();
void CAT_room_draw_props();
void CAT_room_draw_pickups();
void CAT_room_draw_pet();
void CAT_room_draw_gui();
void CAT_render_room();
