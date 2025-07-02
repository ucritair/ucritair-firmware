#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

#include "sensirion_i2c_hal.h"
#include "sen5x_i2c.h"

#include "sensor_hal.h"
#include "cat_core.h"

LOG_MODULE_REGISTER(sen5x, SENSOR_LOG_LEVEL);

static const struct device* dev_i2c = DEVICE_DT_GET(DT_NODELABEL(arduino_i2c));

struct sen5x_state_t {
    char name[32];
    char serial[32];
    uint8_t fw_maj, fw_min, hw_maj, hw_min, proto_maj, proto_min;
    bool fw_debug;

    double mc_pm1p0, mc_pm2p5, mc_pm4p0, mc_pm10p0;
    double nc_pm0p5, nc_pm1p0, nc_pm2p5, nc_pm4p0, nc_pm10p0;
    double typical_particle_size_um;
    double humidity, temp_c;
    double voc_index, nox_index;
    uint32_t status;
    bool is_faulted;
};

static struct sen5x_state_t state = {0,};

#define CHK(_ACT) do { \
    const int result = _ACT; \
    if (result != 0) { \
        LOG_ERR("Action %s failed with %d (%s)", #_ACT, result, strerror(result)); \
        state.is_faulted = true; \
        return result; \
    } \
} while(0)

int sen5x_init()
{
    sensirion_i2c_hal_init(dev_i2c);
    CHK(sen5x_device_reset());
    CHK(sen5x_get_product_name(state.name, sizeof(state.name)));
    LOG_INF("name=%s", state.name);
    CHK(sen5x_get_serial_number(state.serial, sizeof(state.serial)));
    LOG_INF("serial=%s", state.serial);
    CHK(sen5x_get_version(
        &state.fw_maj, &state.fw_min, &state.fw_debug,
        &state.hw_maj, &state.hw_min,
        &state.proto_maj, &state.proto_min));
    LOG_INF("Firmware v%d.%d (debug=%s)", state.fw_maj, state.fw_min, (state.fw_debug ? "y":"n"));
    LOG_INF("Hardware v%d.%d", state.hw_maj, state.hw_min);
    LOG_INF("Protocol v%d.%d", state.proto_maj, state.proto_min);
    CHK(sen5x_set_rht_acceleration_mode(2));
    CHK(sen5x_start_measurement());
    return 0;
}

int sen5x_is_ready(bool* is_ready)
{
    CHK(sen5x_read_data_ready(is_ready));
    return 0;
}

int sen5x_read()
{
    uint32_t status;
    uint16_t mc_pm1p0, mc_pm2p5, mc_pm4p0, mc_pm10p0;
    uint16_t nc_pm0p5, nc_pm1p0, nc_pm2p5, nc_pm4p0, nc_pm10p0, typical_particle_size;
    int16_t humidity, temp;
    int16_t voc_index, nox_index;

    CHK(sen5x_read_and_clear_device_status(&status));
    //TODO: parse the status!!
    LOG_INF("status=%08x", status);

    CHK(sen5x_read_measured_pm_values(
        &mc_pm1p0, &mc_pm2p5, &mc_pm4p0, &mc_pm10p0,
        &nc_pm0p5, &nc_pm1p0, &nc_pm2p5, &nc_pm4p0, &nc_pm10p0,
        &typical_particle_size));
    if (mc_pm1p0  == 0xFFFF ||
        mc_pm2p5  == 0xFFFF ||
        mc_pm4p0  == 0xFFFF ||
        mc_pm10p0 == 0xFFFF ||
        nc_pm0p5  == 0xFFFF ||
        nc_pm1p0  == 0xFFFF ||
        nc_pm2p5  == 0xFFFF ||
        nc_pm4p0  == 0xFFFF ||
        nc_pm10p0 == 0xFFFF ||
        typical_particle_size == 0xFFFF)
    {
        LOG_WRN("not ready yet");
        return -EBUSY;
    }

    CHK(sen5x_read_measured_values(
        &mc_pm1p0, &mc_pm2p5, &mc_pm4p0, &mc_pm10p0,
        &humidity, &temp, &voc_index, &nox_index));
    if (mc_pm1p0  == 0xFFFF ||
        mc_pm2p5  == 0xFFFF ||
        mc_pm4p0  == 0xFFFF ||
        mc_pm10p0 == 0xFFFF ||
        humidity  == 0x7FFF ||
        temp      == 0x7FFF ||
        voc_index == 0x7FFF ||
        nox_index == 0x7FFF)
    {
        LOG_WRN("not ready yet");
        return -EBUSY;
    }

    state.status = status;
    state.mc_pm1p0 = mc_pm1p0 / 10.0;
    state.mc_pm2p5 = mc_pm2p5 / 10.0;
    state.mc_pm4p0 = mc_pm4p0 / 10.0;
    state.mc_pm10p0 = mc_pm10p0 / 10.0;
    state.nc_pm0p5 = nc_pm0p5 / 10.0;
    state.nc_pm1p0 = nc_pm1p0 / 10.0;
    state.nc_pm2p5 = nc_pm2p5 / 10.0;
    state.nc_pm4p0 = nc_pm4p0 / 10.0;
    state.nc_pm10p0 = nc_pm10p0 / 10.0;
    state.typical_particle_size_um = typical_particle_size / 1000.0;
    state.humidity = humidity / 100.0;
    state.temp_c = temp / 200.0;
    state.temp_c -= 2.5; // per MP 10/11/24 21:38:43
    state.voc_index = voc_index / 10.0;
    state.nox_index = nox_index / 10.0;

    LOG_INF("PM(ug/m3): 1.0=%.1f  2.5=%.1f  4.0=%.1f  10.0=%.1f",
        state.mc_pm1p0, state.mc_pm2p5, state.mc_pm4p0, state.mc_pm10p0);
    LOG_INF("PM(#/m3): 0.5=%.1f  1.0=%.1f  2.5=%.1f  4.0=%.1f  10.0=%.1f",
        state.nc_pm0p5, state.nc_pm1p0, state.nc_pm2p5, state.nc_pm4p0, state.nc_pm10p0);
    LOG_INF("typical_particle_size=%.3f um", state.typical_particle_size_um);
    LOG_INF("humidity=%.2f %%RH", state.humidity);
    LOG_INF("temp=%.3f C", state.temp_c);
    LOG_INF("voc_index=%.1f", state.voc_index);
    LOG_INF("nox_index=%.1f", state.nox_index);

    readings.sen5x.pm1_0 = state.mc_pm1p0;
    readings.sen5x.pm2_5 = state.mc_pm2p5;
    readings.sen5x.pm4_0 = state.mc_pm4p0;
    readings.sen5x.pm10_0 = state.mc_pm10p0;
    readings.sen5x.nc0_5 = state.nc_pm0p5;
    readings.sen5x.nc1_0 = state.nc_pm1p0;
    readings.sen5x.nc2_5 = state.nc_pm2p5;
    readings.sen5x.nc4_0 = state.nc_pm4p0;
    readings.sen5x.nc10_0 = state.nc_pm10p0;
    readings.sen5x.typ_particle_sz_um = state.typical_particle_size_um;
    readings.sen5x.humidity_rhpct = state.humidity;
    readings.sen5x.temp_degC = state.temp_c;
    readings.sen5x.voc_index = state.voc_index;
    readings.sen5x.nox_index = state.nox_index;
    readings.sen5x.uptime_last_updated = k_uptime_get();

    return 0;
}

bool sen5x_is_faulted()
{
    return state.is_faulted;
}

SENSOR_DEFINE(sen5x);

//eof
