#pragma once

#include <zephyr/bluetooth/uuid.h>

#define BT_UUID_CAT_SERVICE \
    BT_UUID_128_ENCODE(0x22340001, 0xf0ee, 0x46d1, 0x9b13, 0x4db98657b673)

int bluetooth_init(struct bt_conn_cb* cbs);

//eof
