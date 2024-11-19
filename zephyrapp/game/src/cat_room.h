#pragma once

#include "cat_machine.h"
#include "cat_math.h"

#define CAT_MAX_PROP_COUNT 150
#define CAT_MAX_COIN_COUNT 24

#ifdef CAT_DESKTOP
#define CAT_EARN_TICK_SECS 1800
#else 
#define CAT_EARN_TICK_SECS 1800
#endif

typedef struct CAT_room
{
	CAT_rect bounds;
	CAT_ivec2 cursor;

	int prop_ids[CAT_MAX_PROP_COUNT];
	CAT_ivec2 prop_places[CAT_MAX_PROP_COUNT];
	int prop_overrides[CAT_MAX_PROP_COUNT];
	int prop_count;

	CAT_vec2 coin_origins[CAT_MAX_COIN_COUNT];
	CAT_vec2 coin_places[CAT_MAX_COIN_COUNT];
	int coin_move_timers[CAT_MAX_COIN_COUNT];
	int coin_count;
	int earn_timer_id;

	CAT_machine_state buttons[5];
	int selector;
} CAT_room;
extern CAT_room room;

int CAT_room_find(int item_id);
bool CAT_room_fits(CAT_rect rect);

void CAT_room_add_prop(int item_id, CAT_ivec2 place);
void CAT_room_remove_prop(int idx);
void CAT_room_flip_prop(int idx);

void CAT_room_add_coin(CAT_vec2 origin, CAT_vec2 place);
void CAT_room_remove_coin(int idx);
void CAT_room_earn(int ticks);

void CAT_room_move_cursor();

void CAT_room_init();
void CAT_room_tick(bool capture_input);

void CAT_MS_room(CAT_machine_signal signal);
void CAT_render_room(int cycle);
