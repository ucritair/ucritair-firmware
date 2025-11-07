#include "cat_wifi.h"

#include "rp2350_ipc.h"
#include "cat_gui.h"

typedef struct
{
	uint8_t byte;
} msg_payload_wifi_scan_response_t;

msg_payload_wifi_scan_response_t scan_results;

void poll_network_list()
{
	/*if (rp2350_wifi_scan(&scan_results, 30000))
	{
		LOG_INF("WiFi scan successful! Found %d unique APs:\n", scan_results.count);
		for (int i = 0; i < scan_results.count; i++)
		{
			const char *auth_str[] = {"Open", "WEP", "WPA", "WPA2", "WPA/WPA2"};
			LOG_INF("  [%d] %s [%02X:%02X:%02X:%02X:%02X:%02X] RSSI: %d dBm, Ch: %d, Auth: %s\n",
			        i + 1,
			        scan_results.aps[i].ssid,
			        scan_results.aps[i].bssid[0], scan_results.aps[i].bssid[1],
			        scan_results.aps[i].bssid[2], scan_results.aps[i].bssid[3],
			        scan_results.aps[i].bssid[4], scan_results.aps[i].bssid[5],
			        scan_results.aps[i].rssi,
			        scan_results.aps[i].channel,
			        auth_str[scan_results.aps[i].auth_mode]);
		}
	}
	else
	{
		LOG_ERR("WiFi scan failed or timed out\n");
	}*/
}

void CAT_MS_wifi(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_draw_wifi);
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_gui_begin_menu("WI-FI"))
			{
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