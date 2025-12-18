#pragma once

#include "stdbool.h"

bool CAT_monitor_gate_is_locked();
bool CAT_monitor_gate_lock();
void CAT_monitor_gate_logic();
void CAT_monitor_gate_draw(const char* title);

