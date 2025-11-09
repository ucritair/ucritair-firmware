#include "cat_wifi.h"

#include "rp2350_ipc.h"
#include <string.h>

void rp2350_ipc_init(void)
{
	return;
}

bool rp2350_query_firmware_version(uint8_t *major, uint8_t *minor, uint16_t *patch, uint32_t timeout_ms)
{
	*major = 0;
	*minor = 0;
	*patch = 0;
	return true;
}

bool rp2350_query_protocol_version(uint8_t *major, uint8_t *minor, uint16_t *patch, uint32_t timeout_ms)
{
	*major = 0;
	*minor = 0;
	*patch = 0;
	return true;
}

bool rp2350_wifi_connect(const char *ssid, const char *password, uint8_t auth_mode, uint32_t timeout_ms)
{
	return true;
}

bool rp2350_send_sensor_data(uint32_t sensor_value, uint32_t timeout_ms)
{
	return true;
}

bool rp2350_wifi_scan(msg_payload_wifi_scan_response_t *results, uint32_t timeout_ms)
{
	results->count = 1;
	results->aps[0] = (wifi_ap_record_t)
	{
		.auth_mode = WIFI_AUTH_WPA2,
		.bssid = {0},
		.channel = 0,
		.rssi = 0
	};
	strncpy(&results->aps[0].ssid, "Test Network", sizeof(results->aps[0].ssid));
	return true;
}

bool rp2350_reboot_to_bootloader(uint32_t timeout_ms)
{
	return true;
}

void rp2350_ipc_process(void)
{
	return;
}