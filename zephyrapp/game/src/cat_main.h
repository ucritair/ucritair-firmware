#pragma once

extern int logged_sleep;

#ifdef CAT_DESKTOP
int CAT_load_sleep();
void CAT_save_sleep();
#endif

void CAT_fresh_gamestate();
void CAT_force_load();
void CAT_force_save();

void CAT_init(int seconds_slept);
void CAT_tick_logic();
void CAT_tick_render(int cycle);