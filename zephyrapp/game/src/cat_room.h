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

typedef struct CAT_pet
{
	float vigour;
	float focus;
	float spirit;
	bool critical;

	CAT_vec2 pos;
	CAT_vec2 dir;
	bool left;
	
	int stat_timer_id;
	int walk_timer_id;
	int react_timer_id;
	int action_timer_id;
} CAT_pet;
extern CAT_pet pet;

extern CAT_ASM_state* pet_asm;
extern CAT_ASM_state AS_idle;
extern CAT_ASM_state AS_walk;
extern CAT_ASM_state AS_adjust_in;
extern CAT_ASM_state AS_walk_action;
extern CAT_ASM_state AS_eat;
extern CAT_ASM_state AS_study;
extern CAT_ASM_state AS_play;
extern CAT_ASM_state AS_adjust_out;
extern CAT_ASM_state AS_vig_up;
extern CAT_ASM_state AS_foc_up;
extern CAT_ASM_state AS_spi_up;

extern CAT_ASM_state* bubl_asm;
extern CAT_ASM_state AS_react;

void CAT_pet_anim_init();
void CAT_pet_stat();
bool CAT_pet_seek(CAT_vec2 targ);
void CAT_pet_init();