#include "cat_wifi.h"

#include "cat_gui.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "cat_input.h"
#include "cat_persist.h"

#define MODULE_NAME "CAT_wifi"
#define MODULE_PREFIX "["MODULE_NAME"] "

static bool auth_success = false;

bool CAT_wifi_bootloader(uint32_t timeout_ms)
{
	bool result = false;
#if CAT_WIFI_ENABLED
	CAT_printf(MODULE_PREFIX "Putting RP2350 into bootloader mode\n");
	result = rp2350_reboot_to_bootloader(timeout_ms);
#endif
	return result;
}

bool CAT_wifi_init()
{
	CAT_printf(MODULE_PREFIX "Initializing\n");

	auth_success = false;

#if CAT_WIFI_ENABLED
	rp2350_ipc_init();
	CAT_msleep(10000);

	uint8_t fw_major, fw_minor;
	uint16_t fw_patch;
	if(rp2350_query_firmware_version(&fw_major, &fw_minor, &fw_patch, 5000))
	{
		CAT_printf("RP2350 Firmware: v%u.%u.%u\n", fw_major, fw_minor, fw_patch);
		return true;
	}
#endif

	CAT_printf("Initialization failed\n");	
	return false;
}

bool CAT_wifi_scan(msg_payload_wifi_scan_response_t *results, uint32_t timeout_ms)
{
	bool result = false;
	results->count = 0;
#if CAT_WIFI_ENABLED
	CAT_printf(MODULE_PREFIX "Scanning for networks\n");
	result = rp2350_wifi_scan(results, timeout_ms);
#endif
	return result;
}

bool CAT_wifi_connect(const char* ssid, const char* password, uint8_t auth_mode, uint32_t timeout_ms)
{
	CAT_printf
	(
		MODULE_PREFIX
		"Connecting to network:\n"
		"SSID: %s\n"
		"Auth mode: %d\n"
		"Timeout: %d ms\n",
		ssid, auth_mode, timeout_ms
	);

	bool result = false;
#if CAT_WIFI_ENABLED
	result = rp2350_wifi_connect
	(
		ssid,
		password,
		auth_mode,
		timeout_ms
	);
#endif
	CAT_printf(MODULE_PREFIX "Connection %s!\n", result ? "succeeded" : "failed");
	return result;
}

bool CAT_wifi_autoconnect(int timeout_ms)
{
	bool result = false;
	if(wifi_status == CAT_WIFI_CONNECTION_SUCCESS)
	{
		result = CAT_wifi_connect(wifi_details.ssid, wifi_password, wifi_details.auth_mode, timeout_ms);
		wifi_status = result ? CAT_WIFI_CONNECTION_SUCCESS : CAT_WIFI_CONNECTION_FAILURE;
	}
	return result;
}

bool CAT_wifi_ZK_authenticate(msg_payload_zkp_authenticate_response_t *response, uint32_t timeout_ms)
{
	bool result = false;
#if CAT_WIFI_ENABLED
	CAT_printf(MODULE_PREFIX "Starting ZKP authentication phase\n");
	result = rp2350_zkp_authenticate(response, timeout_ms);
#endif
	auth_success = result;
	return result;
}

bool CAT_wifi_send_data(uint32_t *data, uint8_t count, uint32_t timeout_ms)
{
	bool result = false;
#if CAT_WIFI_ENABLED
	CAT_printf(MODULE_PREFIX "Sending %d data to the RP2350\n", count);
	result = rp2350_send_sensor_data_array(data, count, timeout_ms);
#endif
	return result;
}

bool CAT_is_wifi_connected()
{
	return wifi_status == CAT_WIFI_CONNECTION_SUCCESS;
}

char* CAT_get_wifi_SSID()
{
	return wifi_details.ssid;
}

bool CAT_wifi_is_ZK_authenticated()
{
	return auth_success;
}

static msg_payload_wifi_scan_response_t scan_results;

void print_network_details(wifi_ap_record_t details)
{
	const char *auth_str[] = {"Open", "WEP", "WPA", "WPA2", "WPA/WPA2"};
	CAT_printf
	(
		"%s [%02X:%02X:%02X:%02X:%02X:%02X] RSSI: %d dBm, Ch: %d, Auth: %s\n",
		details.ssid,
		details.bssid[0], details.bssid[1],
		details.bssid[2], details.bssid[3],
		details.bssid[4], details.bssid[5],
		details.rssi,
		details.channel,
		auth_str[details.auth_mode]
	);
}

void poll_network_list()
{
	if (CAT_wifi_scan(&scan_results, 30000))
	{
		CAT_printf("WiFi scan successful! Found %d unique APs:\n", scan_results.count);
		for (int i = 0; i < scan_results.count; i++)
		{
			print_network_details(scan_results.aps[i]);
		}
	}
	else
	{
		CAT_printf("WiFi scan failed or timed out\n");
	}
}

static wifi_ap_record_t network;
static char password[MAX_PASSWORD_LEN];
static int attempt_status = CAT_WIFI_CONNECTION_NONE;

void MS_select_network(CAT_FSM_signal signal);
void MS_enter_password(CAT_FSM_signal signal);
void MS_connect_attempt(CAT_FSM_signal signal);
void MS_connect_result(CAT_FSM_signal);
CAT_FSM fsm;

void MS_select_network(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_gui_begin_menu("WI-FI"))
			{
				for(int i = 0; i < scan_results.count; i++)
				{
					wifi_ap_record_t result = scan_results.aps[i];
					if(strlen(result.ssid) == 0)
						continue;
					if(wifi_status == CAT_WIFI_CONNECTION_SUCCESS && strcmp(result.ssid, wifi_details.ssid) == 0)
					{
						CAT_gui_menu_toggle(result.ssid, true, CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON);
						continue;
					}

					if(CAT_gui_menu_item(result.ssid))
					{
						network = result;
						CAT_FSM_transition(&fsm, MS_enter_password);
					}
				}

				if(CAT_gui_menu_item("REFRESH NETWORKS"))
					poll_network_list();

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

void MS_enter_password(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			password[0] = '\0';
			CAT_gui_open_keyboard(password, MAX_PASSWORD_LEN);
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(!CAT_gui_keyboard_is_open())
			{
				if(strlen(password) == 0)
					CAT_FSM_transition(&fsm, MS_select_network);
				else
					CAT_FSM_transition(&fsm, MS_connect_attempt);
			}
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		{
			
		}
		break;
	}
}

void MS_connect_attempt(CAT_FSM_signal signal)
{
	static bool wait = true;

	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			attempt_status = CAT_WIFI_CONNECTION_NONE;
			wait = true;
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(wait)
			{
				wait = false;
				break;
			}

			attempt_status =
			CAT_wifi_connect
			(
				network.ssid,
				password,
				network.auth_mode,
				CAT_MINUTE_SECONDS * 1000
			) ? CAT_WIFI_CONNECTION_SUCCESS : CAT_WIFI_CONNECTION_FAILURE;

			CAT_FSM_transition(&fsm, MS_connect_result);
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		{
			
		}
		break;
	}
}

void MS_connect_result(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{

		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(attempt_status == CAT_WIFI_CONNECTION_SUCCESS)
			{
				wifi_details = network;
				strncpy(wifi_password, password, MAX_PASSWORD_LEN);
				wifi_status = attempt_status;
			}

			if(CAT_input_pressed(CAT_BUTTON_A) || CAT_input_pressed(CAT_BUTTON_B))
				CAT_FSM_transition(&fsm, NULL);
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		{
			
		}
		break;
	}
}

void CAT_MS_wifi(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_draw_wifi);
			network.ssid[0] = '\0';
			password[0] = '\0';
			poll_network_list();		
			CAT_FSM_transition(&fsm, MS_select_network);
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			CAT_FSM_tick(&fsm);
			if(fsm.state == NULL)
				CAT_pushdown_pop();
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
	CAT_frameberry(CAT_WHITE);

	if(fsm.state == MS_enter_password)
	{
		CAT_set_text_mask(12, -1, CAT_LCD_SCREEN_W-12, -1);
		CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
		CAT_draw_textf
		(
			12, 12,
			"Please enter password for network %s\n",
			network.ssid
		);
	}
	else if(fsm.state == MS_connect_attempt)
	{
		CAT_draw_text(12, 12, "Connection in progress...\n");
	}
	else if(fsm.state == MS_connect_result)
	{
		if(attempt_status == CAT_WIFI_CONNECTION_SUCCESS)
			CAT_draw_text(12, 12, "Connection succeeded!\n");
		else
			CAT_draw_text(12, 12, "Connection failed!\n");
	}
}