#pragma once

#include "cat_machine.h"
#include <stdbool.h>
#include <stdint.h>
#include "rp2350_ipc.h"

#define CAT_WIFI_TRANSMISSION_PERIOD 30
#define CAT_WIFI_DATUM_COUNT 6

typedef enum
{
	CAT_WIFI_CONNECTION_NONE,
	CAT_WIFI_CONNECTION_SUCCESS,
	CAT_WIFI_CONNECTION_FAILURE
} CAT_wifi_connection_status;

bool CAT_wifi_bootloader(uint32_t timeout_ms);
bool CAT_wifi_init();
bool CAT_wifi_scan(msg_payload_wifi_scan_response_t *results, uint32_t timeout_ms);
bool CAT_wifi_connect(const char* ssid, const char* password, uint8_t auth_mode, uint32_t timeout_ms);
bool CAT_wifi_autoconnect(int timeout_ms);
bool CAT_wifi_ZK_authenticate(msg_payload_zkp_authenticate_response_t *response, uint32_t timeout_ms);
bool CAT_wifi_send_data(uint32_t *data, uint8_t count, uint32_t timeout_ms);

bool CAT_is_wifi_connected();
char* CAT_get_wifi_SSID();
bool CAT_wifi_is_ZK_authenticated();

void CAT_MS_wifi(CAT_FSM_signal signal);
void CAT_draw_wifi();
