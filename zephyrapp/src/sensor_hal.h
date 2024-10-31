#pragma once

#include <zephyr/kernel.h>

/* Sensor definition.
 * 
 * For simplicity, state is private to each driver and 
 * only one copy is kept. If we ever need to support more
 * than one instance of a given sensor, can extend this
 * to have a state pointer and some macros to simplify
 * statically creating instances or whatever.
 */
struct sensor_t {
    char const* const name;
    int (*init)();
    int (*is_ready)(bool*);
    int (*read)();
    bool (*is_faulted)();
};

#define SENSOR_DECL(NAME) \
    extern struct sensor_t sensor_##NAME

#define SENSOR_DEFINE(NAME) \
    struct sensor_t sensor_##NAME = { \
        .name = #NAME, \
        .init = NAME##_init, \
        .is_ready = NAME##_is_ready, \
        .read = NAME##_read, \
        .is_faulted = NAME##_is_faulted, \
    }

SENSOR_DECL(sen5x);
SENSOR_DECL(sunrise);
SENSOR_DECL(lps22hh);

int sensor_init();
void sensor_read_once();

//eof
