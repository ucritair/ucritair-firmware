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

#define CAT_ROOM_MAX_PICKUP_COUNT 64


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

typedef struct
{
	struct prop_list_item
	{
		int prop;
		uint8_t x0, y0, x1, y1;
		int override;
		int child;
	} data[CAT_ROOM_GRID_SIZE];
	int length;
} CAT_prop_list;

typedef struct CAT_pickup
{
	uint8_t x0, y0, x1, y1;

	const CAT_sprite* sprite;
	void (*proc)();

	float timer;
} CAT_pickup;

typedef struct
{
	CAT_pickup data[CAT_ROOM_MAX_PICKUP_COUNT];
	int length;
} CAT_pickup_list;

void CAT_room_set_theme(const CAT_room_theme* theme);
const CAT_room_theme* CAT_room_get_theme();

void CAT_room_point2cell(int x, int y, int* x_out, int* y_out);
void CAT_room_cell2point(int x, int y, int* x_out, int* y_out);
bool CAT_room_in_bounds(int x, int y);
bool CAT_room_in_grid_bounds(int x, int y);

bool CAT_room_is_point_free(int x, int y);
bool CAT_room_is_cell_free(int x, int y);
bool CAT_room_is_block_free(int x, int y, int w, int h);
void CAT_room_set_block(int x, int y, int w, int h, int item_id);

bool CAT_room_has_free_cell();
void CAT_room_nearest_free_cell(int x, int y, int* x_out, int* y_out);
void CAT_room_random_free_cell(int* x_out, int* y_out);

CAT_prop_list* CAT_room_get_props();
int CAT_room_prop_lookup(int item_id);
int CAT_room_cell_lookup(int x, int y);

bool CAT_room_fits_prop(int x, int y, int item_id);
int CAT_room_place_prop(int x, int y, int item_id);
int CAT_room_remove_prop(int idx);

bool CAT_room_prop_has_child(int idx);
bool CAT_room_stack_prop(int idx, int item_id);
int CAT_room_unstack_prop(int idx);
int CAT_room_alter_prop(int idx);

CAT_pickup_list* CAT_room_get_pickups();
int CAT_room_spawn_pickup(CAT_pickup pickup);
void CAT_room_consume_pickup(int idx);

int CAT_room_touch_query();


//////////////////////////////////////////////////////////////////////////
// ROOM

void CAT_room_init();
void CAT_room_tick();
void CAT_room_intro();
void CAT_MS_room(CAT_FSM_signal signal);

void CAT_room_draw_statics();
void CAT_room_draw_props();
void CAT_room_draw_pickups();
void CAT_room_draw_pet();
void CAT_room_draw_gui();
void CAT_render_room();
