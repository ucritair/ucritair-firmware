#pragma once

#include "cat_machine.h"
#include <stdbool.h>

void CAT_MS_wifi(CAT_FSM_signal signal);
void CAT_draw_wifi();

bool CAT_is_on_wifi();
char* CAT_get_wifi_SSID();