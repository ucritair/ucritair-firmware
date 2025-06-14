#pragma once

#include <stdint.h>
#include <stdbool.h>

extern float time_since_eink_update;
extern const int eink_update_time_threshold;

void CAT_load_default();
void CAT_load_turnkey();
void CAT_force_load();
void CAT_force_save();

void CAT_init();
void CAT_tick_logic();
void CAT_tick_render();
