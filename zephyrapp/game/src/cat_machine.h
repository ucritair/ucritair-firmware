#pragma once

#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_TIMETABLE_MAX_LENGTH 32


//////////////////////////////////////////////////////////////////////////
// TIMETABLE

typedef struct CAT_timetable
{
	bool active[CAT_TIMETABLE_MAX_LENGTH];
	float duration[CAT_TIMETABLE_MAX_LENGTH];
	float timer[CAT_TIMETABLE_MAX_LENGTH];
} CAT_timetable;
extern CAT_timetable timetable;

void CAT_timetable_init();
int CAT_timer_init(float duration);
void CAT_timer_reinit(int* timer_id, float duration);
void CAT_timer_delete(int timer_id);

float CAT_timer_get(int timer_id);
void CAT_timer_set(int timer_id, float t);
void CAT_timer_add(int timer_id, float t);

bool CAT_timer_tick(int timer_id);
void CAT_timer_reset(int timer_id);

bool CAT_timer_done(int timer_id);
float CAT_timer_progress(int timer_id);

bool CAT_pulse(float period);


//////////////////////////////////////////////////////////////////////////
// MACHINE

/* PROTOTYPE
void CAT_MS_example(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}
*/

typedef enum CAT_machine_signal
{
	CAT_MACHINE_SIGNAL_ENTER,
	CAT_MACHINE_SIGNAL_TICK,
	CAT_MACHINE_SIGNAL_EXIT
} CAT_machine_signal;

typedef void (*CAT_machine_state)(CAT_machine_signal);
typedef void (*CAT_render_callback)(void);

void CAT_machine_transition(CAT_machine_state state);
void CAT_machine_tick();
void CAT_machine_back();
CAT_machine_state CAT_get_machine_state();

void CAT_set_render_callback(CAT_render_callback callback);
CAT_render_callback CAT_get_render_callback();


