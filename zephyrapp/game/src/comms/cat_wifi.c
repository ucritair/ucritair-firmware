#include "cat_wifi.h"

#include "cat_gui.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define MAX_SSID_LEN 32
#define MAX_SCAN_RESULTS 10

typedef struct __attribute__((__packed__))
{
    char ssid[MAX_SSID_LEN];
    uint8_t bssid[6];
    int8_t rssi;           // Signal strength in dBm
    uint8_t channel;
    uint8_t auth_mode;     // 0=Open, 1=WEP, 2=WPA, 3=WPA2, 4=WPA/WPA2
} wifi_ap_record_t;

typedef struct __attribute__((__packed__))
{
    uint8_t count;         // Number of APs found (up to MAX_SCAN_RESULTS)
    wifi_ap_record_t aps[MAX_SCAN_RESULTS];
} msg_payload_wifi_scan_response_t;

static bool rp2350_wifi_scan(msg_payload_wifi_scan_response_t *results, uint32_t timeout_ms)
{
	results->count = 3;
	for(int i = 0; i < results->count; i++)
	{
		wifi_ap_record_t* record = &results->aps[i];
		snprintf(record->ssid, sizeof(record->ssid), "Network %d", i);
		memset(record->bssid, 0, sizeof(record->bssid));
		record->rssi = i;
		record->channel = i;
		record->auth_mode = 3;
	}
	return true;
}

static bool rp2350_wifi_connect(const char *ssid, const char *password, uint8_t auth_mode, uint32_t timeout_ms)
{
	return true;
}

static msg_payload_wifi_scan_response_t scan_results;

void poll_network_list()
{
	if (rp2350_wifi_scan(&scan_results, 30000))
	{
		CAT_printf("WiFi scan successful! Found %d unique APs:\n", scan_results.count);
		for (int i = 0; i < scan_results.count; i++)
		{
			const char *auth_str[] = {"Open", "WEP", "WPA", "WPA2", "WPA/WPA2"};
			CAT_printf
			(
				"  [%d] %s [%02X:%02X:%02X:%02X:%02X:%02X] RSSI: %d dBm, Ch: %d, Auth: %s\n",
				i + 1,
				scan_results.aps[i].ssid,
				scan_results.aps[i].bssid[0], scan_results.aps[i].bssid[1],
				scan_results.aps[i].bssid[2], scan_results.aps[i].bssid[3],
				scan_results.aps[i].bssid[4], scan_results.aps[i].bssid[5],
				scan_results.aps[i].rssi,
				scan_results.aps[i].channel,
				auth_str[scan_results.aps[i].auth_mode]
			);
		}
	}
	else
	{
		CAT_printf("WiFi scan failed or timed out\n");
	}
}

static int network_idx = -1;

void CAT_MS_wifi(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_draw_wifi);
			poll_network_list();
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_gui_begin_menu("WI-FI"))
			{
				for(int i = 0; i < scan_results.count; i++)
				{
					CAT_gui_menu_item(scan_results.aps[i].ssid);
				}
				CAT_gui_end_menu();
			}
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		{

		}
		break;
	}
}

void CAT_draw_wifi()
{
	
}