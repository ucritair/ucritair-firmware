#pragma once

extern int logged_sleep;

extern int saved_version_major;
extern int saved_version_minor;
extern int saved_version_patch;
extern int saved_version_push;

#ifdef CAT_DESKTOP
int CAT_load_sleep();
void CAT_save_sleep();
#endif

void CAT_save_failsafe();
void CAT_force_load();
void CAT_force_save();

void CAT_init(int seconds_slept);
void CAT_tick_logic();
void CAT_tick_render(int cycle);
