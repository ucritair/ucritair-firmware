#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>

#include "ble/service.h"

LOG_MODULE_REGISTER(main);

void on_connected(struct bt_conn* conn, uint8_t err);
void on_disconnected(struct bt_conn* conn, uint8_t reason);

struct bt_conn_cb bt_callbacks = {
    .connected = on_connected,
    .disconnected = on_disconnected,
};

static struct bt_conn* current_conn = NULL;

void on_connected(struct bt_conn* conn, uint8_t err) {
    if (err) {
        LOG_ERR("connection error: %d", err);
        return;
    }

    LOG_INF("connected!");
    current_conn = bt_conn_ref(conn);
}

void on_disconnected(struct bt_conn* conn, uint8_t reason) {
    LOG_INF("disconnected (reason: %d)", reason);
    if (current_conn) {
        bt_conn_unref(current_conn);
        current_conn = NULL;
    }
}


int main(void)
{
    printf("henlo, friendo!\n");

    int err;
    err = bluetooth_init(&bt_callbacks);
    if (err) {
        LOG_ERR("bluetooth_init() failed with %d", err);
        while(true) {
            k_msleep(1000);
        }
    }
    LOG_INF("Running...");

    while (true) {
        k_msleep(1000);
    }

    return 0;
}

//eof
