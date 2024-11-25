#pragma once

#include "cat_machine.h"
#include "cat_math.h"

#define CAT_GRID_WIDTH 15
#define CAT_GRID_HEIGHT 10
#define CAT_GRID_SIZE (CAT_GRID_WIDTH * CAT_GRID_HEIGHT)

#define CAT_MAX_COIN_COUNT 24

#ifdef CAT_DESKTOP
#define CAT_EARN_TICK_SECS 5
#else 
#define CAT_EARN_TICK_SECS 1800
#endif

typedef struct CAT_space
{
	CAT_ivec2 grid_place;
	CAT_ivec2 grid_shape;	

	CAT_ivec2 world_shape;
	CAT_rect world_rect;

	int cells[CAT_GRID_SIZE];

	CAT_ivec2 free_list[CAT_GRID_SIZE];
	int free_list_length;

} CAT_space;
extern CAT_space space;

void CAT_space_init();
CAT_ivec2 CAT_grid2world(CAT_ivec2 grid);
CAT_ivec2 CAT_world2grid(CAT_ivec2 world);

int CAT_get_cell(CAT_ivec2 cell);
void CAT_set_cell(CAT_ivec2 cell, int colour);

bool CAT_block_free(CAT_rect block);
void CAT_set_block(CAT_rect block, int colour);

void CAT_build_free_list();
bool CAT_has_free_space();
CAT_ivec2 CAT_first_free_space();
CAT_ivec2 CAT_rand_free_space();

typedef struct CAT_room
{
	CAT_ivec2 grid_cursor;

	int prop_ids[CAT_GRID_SIZE];
	CAT_ivec2 prop_places[CAT_GRID_SIZE];
	int prop_overrides[CAT_GRID_SIZE];
	int prop_count;

	CAT_vec2 coin_origins[CAT_MAX_COIN_COUNT];
	CAT_vec2 coin_places[CAT_MAX_COIN_COUNT];
	int coin_move_timers[CAT_MAX_COIN_COUNT];
	int coin_count;
	int earn_timer_id;

	CAT_machine_state button_modes[5];
	int mode_selector;
} CAT_room;
extern CAT_room room;

void CAT_room_init();

int CAT_room_find(int item_id);
int CAT_room_add_prop(int item_id, CAT_ivec2 place);
void CAT_room_remove_prop(int idx);
void CAT_room_flip_prop(int idx);

void CAT_room_add_coin(CAT_vec2 origin, CAT_vec2 place);
void CAT_room_remove_coin(int idx);
void CAT_room_earn(int ticks);

void CAT_room_cursor();

void CAT_room_tick(bool capture_input);
void CAT_MS_room(CAT_machine_signal signal);
void CAT_render_room(int cycle);
