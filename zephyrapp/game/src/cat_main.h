#pragma once

#include <stdint.h>
#include <stdbool.h>

#define EINK_UPDATE_PERIOD CAT_MINUTE_SECONDS
extern uint64_t last_eink_time;

void CAT_load_default();
void CAT_load_turnkey();
void CAT_force_load();
void CAT_force_save();

void CAT_init();
void CAT_tick_logic();
void CAT_tick_render();
