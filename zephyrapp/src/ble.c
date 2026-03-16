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
#include <zephyr/sys/util.h>
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
#include "item_assets.h"
#include "cat_pet.h"
#include "batt.h"
#include "cat_persist.h"

/* ── BTHome v2 ──────────────────────────────────────────────── */
#define BTHOME_UUID16_LO 0xD2
#define BTHOME_UUID16_HI 0xFC
#define BTHOME_DEVICE_INFO 0x40 /* v2, no encryption */

/* BTHome v2 object IDs */
#define BTHOME_OBJ_TEMPERATURE 0x02 /* sint16, factor 0.01 °C */
#define BTHOME_OBJ_HUMIDITY    0x03 /* uint16, factor 0.01 %  */
#define BTHOME_OBJ_CO2         0x12 /* uint16, ppm            */
#define BTHOME_OBJ_PM25        0x0D /* uint16, µg/m³          */
#define BTHOME_OBJ_PM10        0x0E /* uint16, µg/m³          */
#define BTHOME_OBJ_BATTERY     0x01 /* uint8, %               */

/*
 * Mutable buffer: [UUID16-LE][device_info][obj,val16]×5 + [obj,val8]×1
 * Total: 2 + 1 + 5×3 + 1×2 = 20 bytes
 */
static uint8_t bthome_svc_data[20] = {
	BTHOME_UUID16_LO, BTHOME_UUID16_HI,
	BTHOME_DEVICE_INFO,
	/* BTHome v2 requires object IDs in ascending order */
	BTHOME_OBJ_BATTERY,     0,         /* 0x01 */
	BTHOME_OBJ_TEMPERATURE, 0, 0,     /* 0x02 */
	BTHOME_OBJ_HUMIDITY,    0, 0,     /* 0x03 */
	BTHOME_OBJ_PM25,        0, 0,     /* 0x0D */
	BTHOME_OBJ_PM10,        0, 0,     /* 0x0E */
	BTHOME_OBJ_CO2,         0, 0,     /* 0x12 */
};

bool ble_connected = false;

/* Active connection reference (refcounted). Protected by notify_lock so
 * worker threads can atomically read + ref before disconnect can free it.
 * Using a tracked reference avoids bt_gatt_notify(NULL, ...) which
 * iterates all connections — including zombie ones mid-teardown. */
static struct bt_conn *notify_conn;
static struct k_spinlock notify_lock;

#define VND_UUID_PFX(x) BT_UUID_128_ENCODE(0xfc7d4395, 0x1019, 0x49c4, 0xa91b, (0x7491ecc4ull<<16) | (unsigned long long)x)

/* Custom Service Variables */
#define BT_UUID_CUSTOM_SERVICE_VAL VND_UUID_PFX(0x0000)

#define VND_MAX_LEN 20

bool ble_ok = false;

static uint8_t vnd_value[VND_MAX_LEN + 1];
char device_id[7]; /* 6-char base-36 ID + NUL */
static char adv_name[VND_MAX_LEN + 1] = CONFIG_BT_DEVICE_NAME;

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
#include "cat_item.h"
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

		.interventions = ((item_table.counts[mask_item] > 0) << 0) | \
		                 ((CAT_room_prop_lookup(prop_purifier_item) != -1) << 1) | \
		                 ((CAT_room_prop_lookup(prop_uv_lamp_item) != -1) << 2)
	};

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &readout, sizeof(readout));
}

static ssize_t read_items(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	lcd_keep_awake();

	bool looking_for_placed = attr->user_data;

	uint8_t bytes[CAT_ITEM_TABLE_CAPACITY>>3] = {0};

	for (int i = 0; i < item_table.length; i++)
	{
		bool found = false;

		if (looking_for_placed)
		{
			found = CAT_room_prop_lookup(i) != -1;
		}
		else
		{
			found = item_table.counts[i] > 0;
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
	if (offset + len > sizeof(pet.name))
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

	memcpy(pet.name+offset, buf, len);

	lcd_keep_awake();
	return len;
}

/* ── BTHome advertising data update ──────────────────────────── */
void update_bthome_adv_data(void)
{
	uint8_t batt = (uint8_t)get_battery_pct();
	int16_t temp = (int16_t)(readings.lps22hh.temp * 100);
	uint16_t hum = (uint16_t)(readings.sen5x.humidity_rhpct * 100);
	uint16_t co2 = (uint16_t)(readings.sunrise.ppm_filtered_compensated);
	uint16_t pm25 = (uint16_t)(readings.sen5x.pm2_5);
	uint16_t pm10 = (uint16_t)(readings.sen5x.pm10_0);

	/* Indices must match bthome_svc_data layout (ascending obj ID order) */
	bthome_svc_data[4] = batt;                              /* [4]    battery  0x01 */
	bthome_svc_data[6] = (uint8_t)(temp & 0xFF);            /* [6:7]  temp     0x02 */
	bthome_svc_data[7] = (uint8_t)((temp >> 8) & 0xFF);
	bthome_svc_data[9] = (uint8_t)(hum & 0xFF);             /* [9:10] humidity 0x03 */
	bthome_svc_data[10] = (uint8_t)((hum >> 8) & 0xFF);
	bthome_svc_data[12] = (uint8_t)(pm25 & 0xFF);           /* [12:13] PM2.5   0x0D */
	bthome_svc_data[13] = (uint8_t)((pm25 >> 8) & 0xFF);
	bthome_svc_data[15] = (uint8_t)(pm10 & 0xFF);           /* [15:16] PM10    0x0E */
	bthome_svc_data[16] = (uint8_t)((pm10 >> 8) & 0xFF);
	bthome_svc_data[18] = (uint8_t)(co2 & 0xFF);            /* [18:19] CO2     0x12 */
	bthome_svc_data[19] = (uint8_t)((co2 >> 8) & 0xFF);
}

/* ── Device config characteristic (0x0015) ──────────────────── */
struct __attribute__((__packed__)) ble_device_config {
	uint16_t sensor_wakeup_period;
	uint16_t sleep_after_seconds;
	uint16_t dim_after_seconds;
	uint8_t  nox_sample_period;
	uint8_t  screen_brightness;
	uint64_t persist_flags;         /* matches cat_persist.h uint64_t */
};

static ssize_t read_config(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	lcd_keep_awake();
	struct ble_device_config cfg = {
		.sensor_wakeup_period = sensor_wakeup_period,
		.sleep_after_seconds = sleep_after_seconds,
		.dim_after_seconds = dim_after_seconds,
		.nox_sample_period = nox_sample_period,
		.screen_brightness = screen_brightness,
		.persist_flags = persist_flags,
	};
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &cfg, sizeof(cfg));
}

static ssize_t write_config(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	struct ble_device_config cfg;

	if (offset != 0)
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	if (len != sizeof(cfg))
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);

	memcpy(&cfg, buf, sizeof(cfg));

	sensor_wakeup_period = cfg.sensor_wakeup_period;
	sleep_after_seconds = cfg.sleep_after_seconds;
	dim_after_seconds = cfg.dim_after_seconds;
	persist_flags = cfg.persist_flags;
	nox_sample_period = cfg.nox_sample_period;
	screen_brightness = cfg.screen_brightness;

	lcd_keep_awake();
	return len;
}

/* ── Log stream characteristic (0x0006) ─────────────────────── */
static uint32_t log_stream_start = 0;
static uint32_t log_stream_count = 0;
static uint32_t log_stream_cursor = 0;
static bool log_stream_active = false;

static void log_stream_work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(log_stream_work, log_stream_work_handler);

static ssize_t write_log_stream(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	if (offset != 0)
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	if (len != 8)
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);

	memcpy(&log_stream_start, buf, 4);
	memcpy(&log_stream_count, (uint8_t *)buf + 4, 4);

	log_stream_cursor = log_stream_start;
	log_stream_active = true;

	lcd_keep_awake();

	k_work_schedule(&log_stream_work, K_MSEC(10));
	return len;
}

/* Resolved in ble_main() after service registration */
static const struct bt_gatt_attr *log_stream_notify_attr = NULL;

static void log_stream_work_handler(struct k_work *work)
{
	/* pkt is static to reduce stack pressure — this handler is
	 * single-threaded (system work queue) so no reentrancy risk. */
	static struct __attribute__((__packed__)) {
		uint32_t cell_nr;
		CAT_log_cell cell;
	} pkt;

	k_spinlock_key_t key = k_spin_lock(&notify_lock);
	struct bt_conn *conn = notify_conn;
	if (conn) bt_conn_ref(conn);
	k_spin_unlock(&notify_lock, key);

	if (!conn || !log_stream_active || !ble_ok || !log_stream_notify_attr) {
		goto out;
	}

	uint32_t cells_sent = log_stream_cursor - log_stream_start;
	if (cells_sent >= log_stream_count)
	{
		/* Send end marker */
		struct __attribute__((__packed__)) {
			uint32_t cell_nr;
		} end_marker = { .cell_nr = 0xFFFFFFFF };

		int err = bt_gatt_notify(conn, log_stream_notify_attr,
					 &end_marker, sizeof(end_marker));
		if (err == -ENOMEM) {
			bt_conn_unref(conn);
			k_work_schedule(&log_stream_work, K_MSEC(50));
			return;
		}
		log_stream_active = false;
		goto out;
	}

	pkt.cell_nr = log_stream_cursor;
	flash_get_cell_by_nr(log_stream_cursor, &pkt.cell);

	/* Bail out if disconnect fired during flash read */
	if (!ble_connected) {
		log_stream_active = false;
		bt_conn_unref(conn);
		return;
	}

	int err = bt_gatt_notify(conn, log_stream_notify_attr,
				 &pkt, sizeof(pkt) - sizeof(pkt.cell.pad));
	if (err) {
		bt_conn_unref(conn);
		if (err == -ENOMEM) {
			k_work_schedule(&log_stream_work, K_MSEC(50));
		} else {
			log_stream_active = false;
		}
		return;
	}

	log_stream_cursor++;
	lcd_keep_awake();
	bt_conn_unref(conn);
	k_work_schedule(&log_stream_work, K_MSEC(10));
	return;

out:
	if (conn) bt_conn_unref(conn);
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
	/* Log stream: write {start_cell, count} to begin streaming via notify */
	BT_GATT_CHARACTERISTIC(
		BT_UUID_DECLARE_128(VND_UUID_PFX(0x0006)),
		BT_GATT_CHRC_WRITE | BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_WRITE,
		NULL, write_log_stream, NULL),
	BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	/* Device config: sensor_wakeup_period, sleep/dim, flags, etc. */
	BT_GATT_CHARACTERISTIC(
		BT_UUID_DECLARE_128(VND_UUID_PFX(0x0015)),
		BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
		read_config, write_config, NULL),
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
	/* Temperature — NOTIFY */
	BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_READ,
        ess_read_float_u16_x100, NULL, &readings.lps22hh.temp),
	BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	/* Pressure */
	BT_GATT_CHARACTERISTIC(BT_UUID_PRESSURE,
        BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
        ess_read_float_u32_x1000, NULL, &readings.lps22hh.pressure),
	/* Humidity — NOTIFY */
	BT_GATT_CHARACTERISTIC(BT_UUID_HUMIDITY,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_READ,
        ess_read_float_u16_x100, NULL, &readings.sen5x.humidity_rhpct),
	BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	/* CO2 — NOTIFY */
	BT_GATT_CHARACTERISTIC(BT_UUID_GATT_CO2CONC,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_READ,
        ess_read_float_u16_x1, NULL, &readings.sunrise.ppm_filtered_compensated),
	BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	/* PM1.0 */
	BT_GATT_CHARACTERISTIC(BT_UUID_GATT_PM1CONC,
        BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
        ess_read_float_fp16_x1, NULL, &readings.sen5x.pm1_0),
	/* PM2.5 — NOTIFY */
	BT_GATT_CHARACTERISTIC(BT_UUID_GATT_PM25CONC,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_READ,
        ess_read_float_fp16_x1, NULL, &readings.sen5x.pm2_5),
	BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	/* PM10 */
	BT_GATT_CHARACTERISTIC(BT_UUID_GATT_PM10CONC,
        BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
        ess_read_float_fp16_x1, NULL, &readings.sen5x.pm10_0),
);

/* AD packet: Flags + BTHome service data + Appearance */
static struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_SVC_DATA16, bthome_svc_data, sizeof(bthome_svc_data)),
	BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE,
		      (CONFIG_BT_DEVICE_APPEARANCE >> 0) & 0xff,
		      (CONFIG_BT_DEVICE_APPEARANCE >> 8) & 0xff),
};

/* Scan response: runtime name + 16-bit service UUIDs. We drop the custom
 * 128-bit UUID here so the full per-device name still fits in the legacy
 * 31-byte payload. */
static struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, adv_name, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_ESS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
};

#define BLE_LEGACY_ADV_MAX_PAYLOAD_LEN 31
#define BLE_AD_ELEM_LEN(_data_len) (1 + 1 + (_data_len))
#define BLE_FLAGS_DATA_LEN 1
#define BLE_APPEARANCE_DATA_LEN 2
#define BLE_UUID16_LIST_DATA_LEN 4
#define UCRIT_NAME_PREFIX_LEN 6
#define DEVICE_ID_LEN 6

enum {
	BLE_BTHOME_ADV_PAYLOAD_LEN =
		BLE_AD_ELEM_LEN(BLE_FLAGS_DATA_LEN) +
		BLE_AD_ELEM_LEN(sizeof(bthome_svc_data)) +
		BLE_AD_ELEM_LEN(BLE_APPEARANCE_DATA_LEN),
	BLE_SCAN_RSP_MAX_PAYLOAD_LEN =
		BLE_AD_ELEM_LEN(UCRIT_NAME_PREFIX_LEN + DEVICE_ID_LEN) +
		BLE_AD_ELEM_LEN(BLE_UUID16_LIST_DATA_LEN),
};

BUILD_ASSERT(UCRIT_NAME_PREFIX_LEN + DEVICE_ID_LEN <= VND_MAX_LEN,
	     "uCrit BLE name exceeds adv_name buffer");
BUILD_ASSERT(BLE_BTHOME_ADV_PAYLOAD_LEN <= BLE_LEGACY_ADV_MAX_PAYLOAD_LEN,
	     "BTHome advertising payload exceeds legacy 31-byte limit");
BUILD_ASSERT(BLE_SCAN_RSP_MAX_PAYLOAD_LEN <= BLE_LEGACY_ADV_MAX_PAYLOAD_LEN,
	     "BLE scan response exceeds legacy 31-byte limit");

#define BT_LE_ADV_CONN_FAST_1_IDENTITY \
	BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONN | BT_LE_ADV_OPT_USE_IDENTITY, \
			BT_GAP_ADV_FAST_INT_MIN_1, \
			BT_GAP_ADV_FAST_INT_MAX_1, NULL)

static void ble_prepare_identity(bool log_identity)
{
	bt_addr_le_t addrs[CONFIG_BT_ID_MAX];
	size_t id_count = CONFIG_BT_ID_MAX;

	bt_id_get(addrs, &id_count);
	if (id_count == 0) {
		return;
	}

	if (log_identity) {
		char addr_str[BT_ADDR_LE_STR_LEN];
		bt_addr_le_to_str(&addrs[0], addr_str, sizeof(addr_str));
		printk("BLE_ADDR: %s\n", addr_str);
	}

	/* FNV-1a hash of 6-byte address -> 32 bits */
	const uint8_t *a = addrs[0].a.val;
	uint32_t h = 2166136261u;
	static const char b36[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for (int i = 0; i < 6; i++) {
		h ^= a[i];
		h *= 16777619u;
	}

	/* Encode as 6 base-36 chars (A-Z 0-9, ~31 bits) */
	for (int i = 0; i < 6; i++) {
		device_id[i] = b36[h % 36];
		h /= 36;
	}
	device_id[6] = '\0';

	snprintf(adv_name, sizeof(adv_name), "uCrit-%s", device_id);
	sd[0].data_len = strlen(adv_name);

	if (bt_set_name(adv_name)) {
		printk("Failed to set BLE name to %s\n", adv_name);
	}

	/* Populate device name characteristic (0x0001) */
	memcpy(vnd_value, adv_name, strlen(adv_name) + 1);

	if (log_identity) {
		printk("Device ID: %s (name: %s)\n", device_id, adv_name);
	}
}

static int start_connectable_advertising(void)
{
	update_bthome_adv_data();
	return bt_le_adv_start(BT_LE_ADV_CONN_FAST_1_IDENTITY,
			       ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
}

/* Timer-wake BLE broadcast: briefly advertise BTHome data for HA */
void ble_broadcast_bthome(void)
{
	int err = bt_enable(NULL);
	if (err) {
		printk("bt_enable for broadcast failed: %d\n", err);
		return;
	}

	ble_prepare_identity(false);
	update_bthome_adv_data();

	/* Use the stable identity address so Home Assistant keeps mapping this
	 * short timer-wake broadcast to the same device. */
	err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY,
			      ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		printk("BTHome broadcast adv start failed: %d\n", err);
		bt_disable();
		return;
	}

	k_msleep(3000); /* Advertise for 3 seconds */

	bt_le_adv_stop();
	bt_disable();
}

void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
	printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

static struct bt_gatt_cb gatt_callbacks = {
	.att_mtu_updated = mtu_updated
};

static void sensor_notify_work_handler(struct k_work *work);
static K_WORK_DEFINE(sensor_notify_work, sensor_notify_work_handler);

static void adv_restart_work_handler(struct k_work *work)
{
	start_connectable_advertising();
}

static K_WORK_DEFINE(adv_restart_work, adv_restart_work_handler);

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
	} else {
		printk("Connected\n");
		k_spinlock_key_t key = k_spin_lock(&notify_lock);
		struct bt_conn *old = notify_conn;
		notify_conn = bt_conn_ref(conn);
		ble_connected = true;
		k_spin_unlock(&notify_lock, key);
		if (old) {
			bt_conn_unref(old);
		}
		lcd_keep_awake();
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);

	/* Clear under lock so workers can't grab a ref between our
	 * NULL-write and unref (which would be a use-after-free). */
	k_spinlock_key_t key = k_spin_lock(&notify_lock);
	struct bt_conn *old = notify_conn;
	notify_conn = NULL;
	ble_connected = false;
	k_spin_unlock(&notify_lock, key);

	if (old) {
		bt_conn_unref(old);
	}

	/* Workers self-terminate: they check notify_conn == NULL under
	 * spinlock at the top of each iteration and exit cleanly. */
	log_stream_active = false;
	log_stream_cursor = 0;
	log_stream_start = 0;
	log_stream_count = 0;
	k_work_submit(&adv_restart_work);
}


BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

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

	printk("Bluetooth initialized\n");

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	bt_gatt_cb_register(&gatt_callbacks);
	bt_conn_auth_cb_register(&auth_cb_display);

	/* Resolve log stream notify attribute by UUID */
	log_stream_notify_attr = bt_gatt_find_by_uuid(
		vnd_svc.attrs, vnd_svc.attr_count,
		BT_UUID_DECLARE_128(VND_UUID_PFX(0x0006)));

	ble_prepare_identity(true);

	err = start_connectable_advertising();
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return 0;
	}

	printk("Advertising successfully started\n");

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

/*
 * ESS attrs layout (with CCC on temp, humidity, CO2, PM2.5):
 * [0]  primary service
 * [1]  char decl: temperature
 * [2]  char value: temperature  ← notify
 * [3]  CCC
 * [4]  char decl: pressure
 * [5]  char value: pressure
 * [6]  char decl: humidity
 * [7]  char value: humidity     ← notify
 * [8]  CCC
 * [9]  char decl: CO2
 * [10] char value: CO2          ← notify
 * [11] CCC
 * [12] char decl: PM1.0
 * [13] char value: PM1.0
 * [14] char decl: PM2.5
 * [15] char value: PM2.5        ← notify
 * [16] CCC
 * [17] char decl: PM10
 * [18] char value: PM10
 */
static void sensor_notify_work_handler(struct k_work *work)
{
	k_spinlock_key_t key = k_spin_lock(&notify_lock);
	struct bt_conn *conn = notify_conn;
	if (conn) bt_conn_ref(conn);
	k_spin_unlock(&notify_lock, key);

	if (!conn || !ble_ok) {
		if (conn) bt_conn_unref(conn);
		return;
	}

	int err;

	int16_t temp_val = (int16_t)(readings.lps22hh.temp * 100);
	err = bt_gatt_notify(conn, &attr_ess_svc[2], &temp_val, sizeof(temp_val));
	if (err) goto done;

	int16_t hum_val = (int16_t)(readings.sen5x.humidity_rhpct * 100);
	err = bt_gatt_notify(conn, &attr_ess_svc[7], &hum_val, sizeof(hum_val));
	if (err) goto done;

	int16_t co2_val = (int16_t)(readings.sunrise.ppm_filtered_compensated);
	err = bt_gatt_notify(conn, &attr_ess_svc[10], &co2_val, sizeof(co2_val));
	if (err) goto done;

	__fp16 pm25_val = (__fp16)(readings.sen5x.pm2_5);
	err = bt_gatt_notify(conn, &attr_ess_svc[15], &pm25_val, sizeof(pm25_val));
	if (err) goto done;
done:
	bt_conn_unref(conn);
}

void ble_notify_sensors(void)
{
	if (!ble_ok || !ble_connected)
		return;

	k_work_submit(&sensor_notify_work);
}

void ble_print_addr(void)
{
	if (!ble_ok)
		return;

	bt_addr_le_t addrs[CONFIG_BT_ID_MAX];
	size_t id_count = CONFIG_BT_ID_MAX;
	bt_id_get(addrs, &id_count);
	if (id_count > 0) {
		char addr_str[BT_ADDR_LE_STR_LEN];
		bt_addr_le_to_str(&addrs[0], addr_str, sizeof(addr_str));
		printk("BLE_ADDR: %s\n", addr_str);
	}
}

void ble_refresh_adv(void)
{
	if (!ble_ok || ble_connected)
		return;

	update_bthome_adv_data();
	bt_le_adv_update_data(ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
}
