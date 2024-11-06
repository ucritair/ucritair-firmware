#ifndef CAT_MACHINE_H
#define CAT_MACHINE_H

#include <stdbool.h>
#include "cat_sprite.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_TIMETABLE_MAX_LENGTH 64


//////////////////////////////////////////////////////////////////////////
// TIMETABLE

typedef struct CAT_timetable
{
	float timers[CAT_TIMETABLE_MAX_LENGTH];
	float durations[CAT_TIMETABLE_MAX_LENGTH];
	int length;
} CAT_timetable;
extern CAT_timetable timetable;

void CAT_timetable_init();
int CAT_timer_init(float duration);
bool CAT_timer_tick(int timer_id);
void CAT_timer_reset(int timer_id);


//////////////////////////////////////////////////////////////////////////
// MACHINE

typedef enum CAT_machine_signal
{
	CAT_MACHINE_SIGNAL_ENTER,
	CAT_MACHINE_SIGNAL_TICK,
	CAT_MACHINE_SIGNAL_EXIT
} CAT_machine_signal;

typedef void (*CAT_machine_state)(CAT_machine_signal);

void CAT_machine_transition(CAT_machine_state* machine, CAT_machine_state state);
void CAT_machine_tick(CAT_machine_state* machine);

//////////////////////////////////////////////////////////////////////////
// ANIMACHINE

typedef struct CAT_ASM_state
{
	int enter_anim_id;
	int tick_anim_id;
	int exit_anim_id;

	enum {ENTER, TICK, EXIT, DONE} signal;
	//struct CAT_ASM_state* next;
} CAT_ASM_state;

void CAT_ASM_init(CAT_ASM_state* state, int enai, int tiai, int exai);
void CAT_ASM_transition(CAT_ASM_state** spp, CAT_ASM_state* next);
void CAT_ASM_kill(CAT_ASM_state** spp);
bool CAT_ASM_is_in(CAT_ASM_state** spp, CAT_ASM_state* state);
bool CAT_ASM_is_entering(CAT_ASM_state** spp);
bool CAT_ASM_is_ticking(CAT_ASM_state** spp);
bool CAT_ASM_is_exiting(CAT_ASM_state** spp);
bool CAT_ASM_is_done(CAT_ASM_state** spp);
int CAT_ASM_tick(CAT_ASM_state** pp);

#endif
