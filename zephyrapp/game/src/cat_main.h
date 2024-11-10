#ifndef CAT_MAIN_H
#define CAT_MAIN_H

void CAT_init(bool is_first_boot, int seconds_slept);
void CAT_tick_logic();
void CAT_tick_render(int cycle);
void CAT_force_load();
void CAT_force_save();

#endif
