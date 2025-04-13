#pragma once

#include <stdint.h>
#include <stdbool.h>

extern uint8_t saved_version_major;
extern uint8_t saved_version_minor;
extern uint8_t saved_version_patch;
extern uint8_t saved_version_push;

extern float time_since_eink_update;
extern const int eink_update_time_threshold;

void CAT_load_reset();
void CAT_force_load();
void CAT_force_save();

void CAT_init();
void CAT_tick_logic();
void CAT_tick_render();
