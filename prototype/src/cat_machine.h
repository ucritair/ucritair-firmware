#ifndef CAT_MACHINE_H
#define CAT_MACHINE_H

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


//////////////////////////////////////////////////////////////////////////
// MACHINE

typedef enum CAT_machine_signal
{
	CAT_MACHINE_SIGNAL_ENTER,
	CAT_MACHINE_SIGNAL_TICK,
	CAT_MACHINE_SIGNAL_EXIT
} CAT_machine_signal;

typedef void (*CAT_machine_state)(CAT_machine_signal);
extern CAT_machine_state machine;

void CAT_machine_transition(CAT_machine_state state);
void CAT_machine_tick();

#endif
