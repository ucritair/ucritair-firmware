#pragma once

#include <stdbool.h>

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
float CAT_timer_progress(int timer_id);


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
// MACHINE AND STATE DECLARATIONS

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
