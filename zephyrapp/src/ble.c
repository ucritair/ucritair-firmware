/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/bluetooth/services/cts.h>
#include <zephyr/bluetooth/services/hrs.h>
#include <zephyr/bluetooth/services/ias.h>

#include "rtc.h"
#include "flash.h"
#include "lcd_rendering.h"

#define VND_UUID_PFX(x) BT_UUID_128_ENCODE(0xfc7d4395, 0x1019, 0x49c4, 0xa91b, (0x7491ecc4ull<<16) | (unsigned long long)x)

/* Custom Service Variables */
#define BT_UUID_CUSTOM_SERVICE_VAL VND_UUID_PFX(0x0000)

#define VND_MAX_LEN 20

bool ble_ok = false;

static uint8_t vnd_value[VND_MAX_LEN + 1] = { 'u', 'c', 'r', 'i', 't', 'r'};

static ssize_t read_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	const char *value = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 strlen(value));
}

static ssize_t write_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (offset + len > VND_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	value[offset + len] = 0;

	return len;
}

static ssize_t read_time(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	uint32_t t = RTC_TIME_TO_EPOCH_TIME(get_current_rtc_time());

	lcd_keep_awake();
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &t, sizeof(t));
}

static ssize_t write_time(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint32_t t = 0;

	if (offset != 0) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (len != sizeof(t))
	{
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	memcpy(&t, buf, sizeof(t));

	set_rtc_counter_raw(EPOCH_TIME_TO_RTC_TIME(t));

	lcd_keep_awake();
	return len;
}

static ssize_t read_cell_count(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	uint32_t t = next_log_cell_nr-1;

	lcd_keep_awake();
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &t, sizeof(t));
}

uint32_t cell_selector = 0;

static ssize_t read_cell_selector(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	lcd_keep_awake();
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &cell_selector, sizeof(cell_selector));
}

static ssize_t write_cell_selector(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint32_t t = 0;

	if (offset != 0) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (len != sizeof(t))
	{
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	memcpy(&t, buf, sizeof(t));

	cell_selector = t;

	lcd_keep_awake();
	return len;
}

struct __attribute__((__packed__))
{
	uint32_t defacto_cell_nr;
	CAT_log_cell cell;
} current_ble_cell = {
	.defacto_cell_nr = 0xffffffff
};

static ssize_t read_cells(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	lcd_keep_awake();
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &current_ble_cell, sizeof(current_ble_cell) - sizeof(current_ble_cell.cell.pad));
}

#include "cat_pet.h"
#include "cat_room.h"
#include "cat_item.h"
#include "cat_bag.h"
#include "cat_core.h"

static ssize_t read_stats(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	lcd_keep_awake();

	struct __attribute__((__packed__))
	{
		uint8_t vigour, focus, spirit;
		uint16_t age;
		uint8_t interventions;
	} readout = {
		.vigour = pet.vigour,
		.focus = pet.focus,
		.spirit = pet.spirit,

		.age = pet.lifetime,

		.interventions = ((CAT_item_list_find(&bag, mask_item) != -1) << 0) | \
		                 ((CAT_room_find(prop_purifier_item) != -1) << 1) | \
		                 ((CAT_room_find(prop_uv_lamp_item) != -1) << 2)
	};

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &readout, sizeof(readout));
}

static ssize_t read_items(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	lcd_keep_awake();

	bool looking_for_placed = attr->user_data;

	uint8_t bytes[CAT_ITEM_TABLE_MAX_LENGTH>>3] = {0};

	for (int i = 0; i < item_table.length; i++)
	{
		bool found = false;

		if (looking_for_placed)
		{
			found = CAT_room_find(i) != -1;
		}
		else
		{
			found = CAT_item_list_find(&bag, i) != -1;
		}

		if (found)
		{
			bytes[i>>3] |= 1<<(i&0b111);
		}
	}

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &bytes, sizeof(bytes));
}

static ssize_t read_bonus(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	lcd_keep_awake();

	uint32_t bonus = CAT_bonus_get();

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &bonus, sizeof(bonus));
}

static ssize_t write_bonus(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint32_t t = 0;

	if (offset != 0) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (len != sizeof(t))
	{
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	memcpy(&t, buf, sizeof(t));

	CAT_bonus_set(t);

	lcd_keep_awake();
	return len;
}

static ssize_t read_name(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	lcd_keep_awake();
	return bt_gatt_attr_read(conn, attr, buf, len, offset, pet.name, sizeof(pet.name));
}

static ssize_t write_name(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	memcpy(pet.name+offset, buf, len);

	lcd_keep_awake();
	return len;
}

/* Vendor Primary Service Declaration */
BT_GATT_SERVICE_DEFINE(vnd_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_DECLARE_128(BT_UUID_CUSTOM_SERVICE_VAL)),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_DECLARE_128(VND_UUID_PFX(0x0001)),
		BT_GATT_CHRC_READ|BT_GATT_CHRC_WRITE, BT_GATT_PERM_READ|BT_GATT_PERM_WRITE,
		read_vnd, write_vnd, vnd_value),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_DECLARE_128(VND_UUID_PFX(0x0002)),
		BT_GATT_CHRC_READ|BT_GATT_CHRC_WRITE, BT_GATT_PERM_READ|BT_GATT_PERM_WRITE,
		read_time, write_time, NULL),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_DECLARE_128(VND_UUID_PFX(0x0003)),
		BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
		read_cell_count, NULL, NULL),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_DECLARE_128(VND_UUID_PFX(0x0004)),
		BT_GATT_CHRC_READ|BT_GATT_CHRC_WRITE, BT_GATT_PERM_READ|BT_GATT_PERM_WRITE,
		read_cell_selector, write_cell_selector, NULL),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_DECLARE_128(VND_UUID_PFX(0x0005)),
		BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
		read_cells, NULL, NULL),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_DECLARE_128(VND_UUID_PFX(0x0010)),
		BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
		read_stats, NULL, NULL),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_DECLARE_128(VND_UUID_PFX(0x0011)),
		BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
		read_items, NULL, 0),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_DECLARE_128(VND_UUID_PFX(0x0012)),
		BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
		read_items, NULL, 1),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_DECLARE_128(VND_UUID_PFX(0x0013)),
		BT_GATT_CHRC_READ|BT_GATT_CHRC_WRITE, BT_GATT_PERM_READ|BT_GATT_PERM_WRITE,
		read_bonus, write_bonus, NULL),
	BT_GATT_CHARACTERISTIC(
		BT_UUID_DECLARE_128(VND_UUID_PFX(0x0014)),
		BT_GATT_CHRC_READ|BT_GATT_CHRC_WRITE, BT_GATT_PERM_READ|BT_GATT_PERM_WRITE,
		read_name, write_name, NULL),
	// player attributes and age
);

static ssize_t ess_read_float_u16_x100(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf, uint16_t len, uint16_t offset)
{
    int16_t value = (*(float*)attr->user_data) * 100;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

static ssize_t ess_read_float_u32_x1000(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf, uint16_t len, uint16_t offset)
{
    int32_t value = (*(float*)attr->user_data) * 1000;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

static ssize_t ess_read_float_u16_x1(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf, uint16_t len, uint16_t offset)
{
    int16_t value = (*(float*)attr->user_data) * 1;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

static ssize_t ess_read_float_fp16_x1(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf, uint16_t len, uint16_t offset)
{
    __fp16 value = (*(float*)attr->user_data) * 1;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

BT_GATT_SERVICE_DEFINE(ess_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),
	BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
        BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
        ess_read_float_u16_x100, NULL, &readings.lps22hh.temp),
	BT_GATT_CHARACTERISTIC(BT_UUID_PRESSURE,
        BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
        ess_read_float_u32_x1000, NULL, &readings.lps22hh.pressure),
	BT_GATT_CHARACTERISTIC(BT_UUID_HUMIDITY,
        BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
        ess_read_float_u16_x100, NULL, &readings.sen5x.humidity_rhpct),
	BT_GATT_CHARACTERISTIC(BT_UUID_GATT_CO2CONC,
        BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
        ess_read_float_u16_x1, NULL, &readings.sunrise.ppm_filtered_compensated),
	BT_GATT_CHARACTERISTIC(BT_UUID_GATT_PM1CONC,
        BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
        ess_read_float_fp16_x1, NULL, &readings.sen5x.pm1_0),
	BT_GATT_CHARACTERISTIC(BT_UUID_GATT_PM25CONC,
        BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
        ess_read_float_fp16_x1, NULL, &readings.sen5x.pm2_5),
	BT_GATT_CHARACTERISTIC(BT_UUID_GATT_PM10CONC,
        BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
        ess_read_float_fp16_x1, NULL, &readings.sen5x.pm10_0),
);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE, (CONFIG_BT_DEVICE_APPEARANCE >> 0) & 0xff, (CONFIG_BT_DEVICE_APPEARANCE >> 8) & 0xff),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_ESS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_SERVICE_VAL),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
	printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

static struct bt_gatt_cb gatt_callbacks = {
	.att_mtu_updated = mtu_updated
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
	} else {
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));
}

static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type, struct net_buf_simple *buf)
{
	//TODO
	// printk("Scan: type=%02x addr=%02x:%02x:%02x:%02x:%02x:%02x adv_type=%02x rssi=%d data len=%u\n",
	// 	addr->type,
	// 	addr->a.val[0],
	// 	addr->a.val[1],
	// 	addr->a.val[2],
	// 	addr->a.val[3],
	// 	addr->a.val[4],
	// 	addr->a.val[5],
	// 	adv_type, rssi, buf->len);
	// printk("      data");
	// for (uint16_t i = 0; i < buf->len; i++) {
	// 	printk(" %02x", buf->data[i]);
	// }
	// printk("\n");
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(void)
{
	struct bt_le_scan_param scan_param = {
		.type     = BT_LE_SCAN_TYPE_PASSIVE,
		.options  = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
		.interval = 0x0300,
		.window   = 0x0300,
	};
	int err;

	printk("Bluetooth initialized\n");

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");

	err = bt_le_scan_start(&scan_param, scan_cb);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
		return;
	}

	printk("Scanning successfully started\n");
}

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Passkey for %s: %06u\n", addr, passkey);
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
	.passkey_display = auth_passkey_display,
	.passkey_entry = NULL,
	.cancel = auth_cancel,
};

static void bas_notify(void)
{
	uint8_t battery_level = bt_bas_get_battery_level();

	battery_level--;

	if (!battery_level) {
		battery_level = 100U;
	}

	bt_bas_set_battery_level(battery_level);
}

int ble_main(void)
{
	int err;

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	bt_ready();
	bt_gatt_cb_register(&gatt_callbacks);
	bt_conn_auth_cb_register(&auth_cb_display);

	ble_ok = true;

	return 0;
}

void ble_update()
{
	if (cell_selector == 0xffffffff)
	{
		populate_log_cell(&current_ble_cell.cell);
		current_ble_cell.defacto_cell_nr = cell_selector;
	}
	else if (cell_selector != current_ble_cell.defacto_cell_nr)
	{
		printk("Reading cell %d into current_ble_cell\n", cell_selector);
		flash_get_cell_by_nr(cell_selector, &current_ble_cell.cell);
		current_ble_cell.defacto_cell_nr = cell_selector;
	}
}
