#include "cat_wifi.h"

#include "cat_gui.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "cat_input.h"
#include "rp2350_ipc.h"

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

static wifi_ap_record_t network;
static char password[MAX_SSID_LEN];
static enum {CONNECT_NONE, CONNECT_ATTEMPT, CONNECT_SUCCESS, CONNECT_FAILURE} connection_status = CONNECT_NONE;

void MS_select_network(CAT_FSM_signal signal);
void MS_enter_password(CAT_FSM_signal signal);
void MS_connect(CAT_FSM_signal signal);
CAT_FSM fsm;

void MS_select_network(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			poll_network_list();
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_gui_begin_menu("WI-FI"))
			{
				for(int i = 0; i < scan_results.count; i++)
				{
					if(strlen(scan_results.aps[i].ssid) == 0)
						continue;
					if(strcmp(scan_results.aps[i].ssid, network.ssid) == 0 && connection_status == CONNECT_SUCCESS)
					{
						CAT_gui_menu_toggle(network.ssid, true, CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON);
						continue;
					}
					if(CAT_gui_menu_item(scan_results.aps[i].ssid))
					{
						network = scan_results.aps[i];
						CAT_FSM_transition(&fsm, MS_enter_password);
					}
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
					CAT_FSM_transition(&fsm, MS_connect);
			}
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		{
			
		}
		break;
	}
}

void MS_connect(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(connection_status == CONNECT_NONE)
			{
				connection_status = CONNECT_ATTEMPT;
			}
			else if(connection_status == CONNECT_ATTEMPT)
			{
				connection_status =
				rp2350_wifi_connect
				(
					network.ssid,
					password,
					network.auth_mode,
					CAT_MINUTE_SECONDS * 1000
				) ? CONNECT_SUCCESS : CONNECT_FAILURE;
			}
			else
			{
				if(CAT_input_pressed(CAT_BUTTON_A) || CAT_input_pressed(CAT_BUTTON_B))
					CAT_FSM_transition(&fsm, NULL);
			}
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
	if(fsm.state == MS_enter_password)
	{
		CAT_frameberry(CAT_WHITE);
		CAT_set_text_mask(12, -1, CAT_LCD_SCREEN_W-12, -1);
		CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
		CAT_draw_textf
		(
			12, 12,
			"Please enter password for network %s\n",
			network.ssid
		);
	}
	else if(fsm.state == MS_connect)
	{
		CAT_frameberry(CAT_WHITE);
		if(connection_status == CONNECT_ATTEMPT)
			CAT_draw_text(12, 12, "Connection in progress...\n");
		else if(connection_status == CONNECT_SUCCESS)
			CAT_draw_text(12, 12, "Connection succeeded!\n");
		else
			CAT_draw_text(12, 12, "Connection failed!\n");
	}
	else
	{
		CAT_frameberry(CAT_WHITE);
	}
}

bool CAT_is_on_wifi()
{
	return connection_status == CONNECT_SUCCESS;
}

char* CAT_get_wifi_SSID()
{
	return network.ssid;
}