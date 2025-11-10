#pragma once

#include "cat_machine.h"
#include <stdbool.h>

typedef enum
{
	CAT_WIFI_CONNECTION_NONE,
	CAT_WIFI_CONNECTION_SUCCESS,
	CAT_WIFI_CONNECTION_FAILURE
} CAT_wifi_connection_status;

void CAT_MS_wifi(CAT_FSM_signal signal);
void CAT_draw_wifi();

void CAT_wifi_autoconnect(int timeout_ms);
bool CAT_is_wifi_connected();
char* CAT_get_wifi_SSID();