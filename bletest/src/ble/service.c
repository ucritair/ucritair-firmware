#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>

#include "service.h"

LOG_MODULE_REGISTER(ble_cat);

static K_SEM_DEFINE(bt_init_ok, 0, 1);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME)-1)

// Advertising data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

// Scan response data
static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CAT_SERVICE),
};


/* Callbacks */

void bt_ready(int err) {
    if (err) {
        LOG_ERR("bt_enable returned %d", err);
    }
    k_sem_give(&bt_init_ok);
}

/* etc... */

int bluetooth_init(struct bt_conn_cb* cbs) {
    int err;
    LOG_INF("Initializing bluetooth...");

    if (cbs == NULL) {
        LOG_ERR("callbacks null :<");
        return -NRFX_ERROR_NULL;
    }

    bt_conn_cb_register(cbs);

    err = bt_enable(bt_ready);
    if (err) {
        LOG_ERR("bt_enable returned %d", err);
        return err;
    }

    // Wait for bt to be ready
    err = k_sem_take(&bt_init_ok, K_MSEC(5000));
    if (err) {
        LOG_ERR("k_sem_take returned %d", err);
        return err;
    }

    // Start advertising
    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("failed to start advertising: %d", err);
        return err;
    }

    return err;
}

//eof
