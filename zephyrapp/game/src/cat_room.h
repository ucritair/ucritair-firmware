#ifndef CAT_ROOM_H
#define CAT_ROOM_H

#include "cat_machine.h"
#include "cat_math.h"

#define CAT_MAX_PROP_COUNT 210

extern CAT_machine_state machine;
extern void CAT_MS_room(CAT_machine_signal);
extern void CAT_MS_feed(CAT_machine_signal);
extern void CAT_MS_study(CAT_machine_signal);
extern void CAT_MS_play(CAT_machine_signal);
extern void CAT_MS_deco(CAT_machine_signal);
extern void CAT_MS_menu(CAT_machine_signal);
extern void CAT_MS_stats(CAT_machine_signal);
extern void CAT_MS_bag(CAT_machine_signal);
extern void CAT_MS_vending(CAT_machine_signal);
extern void CAT_MS_arcade(CAT_machine_signal);
extern void CAT_MS_manual(CAT_machine_signal);

extern void CAT_render_room(int cycle);
extern void CAT_render_action();
extern void CAT_render_deco();
extern void CAT_render_menu();
extern void CAT_render_stats();
extern void CAT_render_bag();
extern void CAT_render_vending();
extern void CAT_render_arcade();
extern void CAT_render_manual();

typedef struct CAT_room
{
	CAT_rect bounds;
	CAT_ivec2 cursor;

	int props[CAT_MAX_PROP_COUNT];
	CAT_ivec2 places[CAT_MAX_PROP_COUNT];
	int overrides[CAT_MAX_PROP_COUNT];
	int prop_count;

	CAT_machine_state buttons[5];
	int selector;
} CAT_room;
extern CAT_room room;

void CAT_room_init();
int CAT_room_find(int item_id);
bool CAT_room_fits(CAT_rect rect);
void CAT_room_place(int item_id, CAT_ivec2 place);
void CAT_room_remove(int idx);
void CAT_room_flip(int idx);
void CAT_room_move_cursor();

#endif