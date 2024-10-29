#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>

#include "sensor_hal.h"

static const struct device* dev_i2c = DEVICE_DT_GET(DT_NODELABEL(arduino_i2c));

static const struct sensor_t* sensors[] = {
    &sensor_sen5x,
    &sensor_sunrise,
    &sensor_lps22hh,
};


void sensor_read_once(void) {
    bool is_ready = false;
    printf("Reading sensors...\n");
    ARRAY_FOR_EACH(sensors, i) {
        const struct sensor_t* const s = sensors[i];
        if (s->is_faulted()) {
            printf("[sensor:%s] Faulted\n", s->name);
            continue;
        } else if (s->is_ready(&is_ready) != 0) {
            printf("[sensor:%s] Ready check failed\n", s->name);
        } else if (!is_ready) {
            /* not ready, take no action */
        } else if (s->read() != 0) {
            printf("[sensor:%s] Read failed\n", s->name);
        } else {
            printf("[sensor:%s] success!\n", s->name);
        }
    }
}


int sensor_init(void)
{
    printf("henlo, friendo!\n");

    while (!device_is_ready(dev_i2c)) {
        printf("waiting for i2c to be ready...\n");
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

    printf("Initializing sensors...\n");
    ARRAY_FOR_EACH(sensors, i) {
        const struct sensor_t* const s = sensors[i];
        if (s->init() != 0) {
            printf("[sensor:%s] Init failed :<\n", s->name);
        }
    }

    sensor_read_once();

    return 0;
}

//eof
