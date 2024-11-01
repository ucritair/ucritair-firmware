#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>

#include "sensor_hal.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sensor_hal, SENSOR_LOG_LEVEL);

static const struct device* dev_i2c = DEVICE_DT_GET(DT_NODELABEL(arduino_i2c));

static const struct sensor_t* sensors[] = {
    &sensor_sen5x,
    &sensor_sunrise,
    &sensor_lps22hh,
};


void sensor_read_once(void) {
    bool is_ready = false;
    LOG_DBG("Reading sensors...");
    ARRAY_FOR_EACH(sensors, i) {
        const struct sensor_t* const s = sensors[i];
        for (int retry = 0; retry < 2; retry++)
        {
            if (s->is_faulted()) {
                LOG_ERR("[sensor:%s] Faulted", s->name);
                s->init();
                continue;
            } else if (s->is_ready(&is_ready) != 0) {
                LOG_WRN("[sensor:%s] Ready check failed", s->name);
            } else if (!is_ready) {
                /* not ready, take no action */
            } else if (s->read() != 0) {
                LOG_ERR("[sensor:%s] Read failed", s->name);
            } else {
                LOG_DBG("[sensor:%s] success!", s->name);
            }
        }
    }
}


int sensor_init(void)
{
    LOG_DBG("henlo, friendo!");

    while (!device_is_ready(dev_i2c)) {
        LOG_DBG("waiting for i2c to be ready...");
        k_msleep(100);
    }

    printf("i2c bus scan:\n");
    printf("    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    for (uint8_t y = 0; y <= 7; y++) {
        printf("%02x", y<<4);
        for (uint8_t x = 0; x <= 0xf; x++) {
            uint8_t addr = (y<<4) | x;
            uint8_t buf = 0;
            if (addr < 0x08 || addr > 0x77) {
                printf("   ");
            } else if (i2c_read(dev_i2c, &buf, 0, addr) == 0) {
                printf(" %02x", addr);
            } else {
                printf(" --");
            }
        }
        printf("\n");
    }

    LOG_INF("Initializing sensors...");
    ARRAY_FOR_EACH(sensors, i) {
        const struct sensor_t* const s = sensors[i];
        if (s->init() != 0) {
            LOG_ERR("[sensor:%s] Init failed :<", s->name);
        }
    }

    sensor_read_once();

    return 0;
}

//eof
