#pragma once

#include "cat_machine.h"
#include <stdbool.h>

typedef enum
{
	CAT_WIFI_CONNECTION_NONE,
	CAT_WIFI_CONNECTION_SUCCESS,
	CAT_WIFI_CONNECTION_FAILURE
} CAT_wifi_connection_status;

bool CAT_wifi_init();
bool CAT_wifi_connect(const char* ssid, const char* password, uint8_t auth_mode, uint32_t timeout_ms);
void CAT_wifi_autoconnect(int timeout_ms);
bool CAT_is_wifi_connected();
char* CAT_get_wifi_SSID();

void CAT_MS_wifi(CAT_FSM_signal signal);
void CAT_draw_wifi();
