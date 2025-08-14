#pragma once

#include "stdbool.h"

void CAT_monitor_gate_init(const char* title);
bool CAT_monitor_gate_is_locked();
bool CAT_monitor_gate_lock();
void CAT_monitor_gate_logic();
void CAT_monitor_gate_render();

