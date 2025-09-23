#pragma once

#include <stdint.h>
#include <stdbool.h>

void CAT_load_default();
void CAT_load_turnkey();
void CAT_force_load();
void CAT_force_save();

void CAT_init();
void CAT_tick_logic();
void CAT_tick_render();
