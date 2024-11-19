#pragma once

void CAT_fresh_gamestate();
#ifdef CAT_DESKTOP
void CAT_load_sleep(int* seconds);
void CAT_save_sleep();
#endif
void CAT_force_load();
void CAT_force_save();

void CAT_init(int seconds_slept);
void CAT_tick_logic();
void CAT_tick_render(int cycle);