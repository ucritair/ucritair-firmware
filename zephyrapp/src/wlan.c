	
#include <zephyr/net/net_if.h>
#include <zephyr/net/ethernet_mgmt.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/wifi_utils.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(wlan, LOG_LEVEL_DBG);

bool did_post_wifi = false;

static struct net_mgmt_event_callback wifi_shell_mgmt_cb;

static void handle_wifi_scan_result(struct net_mgmt_event_callback *cb)
{
	const struct wifi_scan_result *entry =
		(const struct wifi_scan_result *)cb->info;

	if (strstr(entry->ssid, "Entropic") != NULL)
	{
		did_post_wifi = true;
	}
}

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
				    uint32_t mgmt_event, struct net_if *iface)
{
	switch (mgmt_event) {
	case NET_EVENT_WIFI_SCAN_RESULT:
		handle_wifi_scan_result(cb);
		break;
	default:
		break;
	}
}


void set_mac()
{

#ifdef CONFIG_WIFI
	struct net_if *iface = net_if_get_default();

	// k_msleep(100);
	// net_if_up(iface);
	// k_msleep(100);
	// net_if_down(iface);


	char mac_addr_change[]={0x12, 0x34, 0x56, 0x78, 0x90, 0x12};
	struct ethernet_req_params params;
	int ret;

	// if (net_if_is_up(iface))
	// {
	// 	sys_reboot(SYS_REBOOT_WARM);
	// 	while (1)
	// 	{
	// 		LOG_ERR("iface is up");
	// 		k_msleep(1000);
	// 	}
	// }

	memcpy(params.mac_address.addr, mac_addr_change, 6);
	// net_if_down(iface);

	ret = net_mgmt(NET_REQUEST_ETHERNET_SET_MAC_ADDRESS, iface,
			&params, sizeof(struct ethernet_req_params));
	if(ret != 0) {
		while (1)
		{
			LOG_ERR("unable to change mac address");
			k_msleep(1000);
		}
	}
	ret = memcmp(net_if_get_link_addr(iface)->addr, mac_addr_change,
			sizeof(mac_addr_change));
	if(ret != 0) {
		while (1)
		{
			LOG_ERR("mac address change failed");
			k_msleep(1000);
		}
	}
	LOG_DBG("MAC changed to %x:%x:%x:%x:%x:%x\n", \
	mac_addr_change[0], mac_addr_change[1], mac_addr_change[2], \
	mac_addr_change[3], mac_addr_change[4], mac_addr_change[5]);
	int ifup = net_if_up(iface);

	LOG_INF("ifup result: %d", ifup);

	k_msleep(50);

	// struct wifi_mode_info mode_info = {0};
	// int modeget = net_mgmt(NET_REQUEST_WIFI_MODE, iface, &mode_info, sizeof(mode_info));

	// if (modeget != 0)
	// {
	// 	while (1)
	// 	{
	// 		LOG_ERR("Wifi failed to come up, modeget -> %d", modeget);
	// 		k_msleep(1000);
	// 	}
	// }

	LOG_INF("Done changing MAC");

	net_mgmt_init_event_callback(&wifi_shell_mgmt_cb,
	     wifi_mgmt_event_handler,
	     (NET_EVENT_WIFI_SCAN_DONE   |
				NET_EVENT_WIFI_CONNECT_RESULT     |
				NET_EVENT_WIFI_DISCONNECT_RESULT  |
				NET_EVENT_WIFI_TWT                |
				NET_EVENT_WIFI_RAW_SCAN_RESULT    |
				NET_EVENT_WIFI_AP_ENABLE_RESULT   |
				NET_EVENT_WIFI_AP_DISABLE_RESULT  |
				NET_EVENT_WIFI_AP_STA_CONNECTED   |
				NET_EVENT_WIFI_AP_STA_DISCONNECTED|
				NET_EVENT_WIFI_SCAN_RESULT));

	net_mgmt_add_event_callback(&wifi_shell_mgmt_cb);

	struct wifi_scan_params scanparams = { 0 };

	if (net_mgmt(NET_REQUEST_WIFI_SCAN, iface, &scanparams, sizeof(scanparams))) {
		LOG_WRN("Scan request failed\n");
		return -ENOEXEC;
	}

#else
	LOG_WRN("CONFIG_WIFI disabled");
#endif
}

