#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

#include "lps22hh.h"

#include "sensor.h"

LOG_MODULE_REGISTER(lps22hh);

static const struct device* dev_i2c = DEVICE_DT_GET(DT_NODELABEL(arduino_i2c));

static int32_t io_init() { return 0; }
static int32_t io_deinit() { return 0; }
static int32_t io_get_tick() {
    LOG_ERR("%s not implemented", __PRETTY_FUNCTION__);
    k_panic();
    return 0;
}
static void io_delay(uint32_t) {
    LOG_ERR("%s not implemented", __PRETTY_FUNCTION__);
    k_panic();
}
static int32_t io_write_reg(uint16_t addr, uint16_t reg, uint8_t* data, uint16_t data_len) {
    return i2c_burst_write(dev_i2c, addr, (uint8_t)reg, data, data_len);
}
static int32_t io_read_reg(uint16_t addr, uint16_t reg, uint8_t* buf, uint16_t buf_len) {
    return i2c_burst_read(dev_i2c, addr, (uint8_t)reg, buf, buf_len);
}

static LPS22HH_Object_t obj = {0,};
static LPS22HH_IO_t io = {
    .Init = io_init,
    .DeInit = io_deinit,
    .BusType = LPS22HH_I2C_BUS,
    .Address = 0x5d,
    .WriteReg = io_write_reg,
    .ReadReg = io_read_reg,
    .GetTick = io_get_tick,
    .Delay = io_delay,
};

struct lps22hh_state_t {
    bool is_faulted;
};
static struct lps22hh_state_t state = {0,};

#define CHK(_ACT) do { \
    const int result = _ACT; \
    if (result != 0) { \
        LOG_ERR("Action %s failed with %d (%s)\n", #_ACT, result, strerror(result)); \
        state.is_faulted = true; \
        return result; \
    } \
} while(0)

int lps22hh_init()
{
    CHK(LPS22HH_RegisterBusIO(&obj, &io));
    CHK(LPS22HH_Init(&obj));
    CHK(LPS22HH_TEMP_Enable(&obj));
    CHK(LPS22HH_PRESS_Enable(&obj));
    state.is_faulted = false;
    return 0;
}

int lps22hh_is_ready(bool* is_ready)
{
    lps22hh_status_t status;
    CHK(lps22hh_status_reg_get(&(obj.Ctx), &status));
    *is_ready = status.t_da || status.p_da;
    return 0;
}

int lps22hh_read()
{
    float temp, pressure;
    CHK(LPS22HH_TEMP_GetTemperature(&obj, &temp));
    CHK(LPS22HH_PRESS_GetPressure(&obj, &pressure));
    LOG_INF("temp=%.2f  pressure=%.2f", temp, pressure);
    sunrise_update_pressure(pressure);
    return 0;
}

bool lps22hh_is_faulted()
{
    return state.is_faulted;
}

SENSOR_DEFINE(lps22hh);

//eof
