#pragma once

#include <stdint.h>
#include <stdbool.h>

extern int logged_sleep;
extern bool needs_load;
extern bool override_load;

extern uint8_t saved_version_major;
extern uint8_t saved_version_minor;
extern uint8_t saved_version_patch;
extern uint8_t saved_version_push;

#ifdef CAT_DESKTOP
int CAT_load_sleep();
void CAT_save_sleep();
#endif

void CAT_load_failsafe();
void CAT_force_load();
void CAT_force_save();

void CAT_init(int seconds_slept);
void CAT_tick_logic();
void CAT_tick_render(int cycle);
