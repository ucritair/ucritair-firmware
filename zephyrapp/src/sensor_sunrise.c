#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

#include "sensor_hal.h"
#include <zephyr/sys/byteorder.h>

#include "airquality.h"

LOG_MODULE_REGISTER(sunrise, SENSOR_LOG_LEVEL);

/* Sensor goes to sleep after this many milliseconds of SDA inactivity.
 * After this, needs another wake-up.
 * Note the sensor also goes to sleep immediately after a completed read or write sequence. */
#define TIMEOUT_MS 15
/* EEPROM writes take 25ms */
#define EEPROM_UPDATE_DELAY_MS 25

//TODO: Should we wrap all i2c calls to track last activity
// time so we can run wakeup automatically in case another
// thread interrupts us for longer than 15ms mid operation?

static const struct device* dev_i2c = DEVICE_DT_GET(DT_NODELABEL(arduino_i2c));

/* Maximum number of retries for wake-up attempts */
static const size_t MAX_RETRIES = 5;
/* Senseair Sunrise I2C device address */
static const uint16_t ADDR = 0x68;

struct sunrise_state_t {
    uint8_t count;
    //todo: timestamp for device needing wake-up?
    //todo: timestamp for device reading ready?
    bool is_faulted;
};
static struct sunrise_state_t state = {0,};


#pragma pack(push, 1)
#if !CONFIG_LITTLE_ENDIAN
#error "These structures assume little endian for bit packing"
#endif

/* ErrorStatus register (00h-01h) */
struct error_status_t {
    //TODO: verify this bit and byte ordering is correct
    uint8_t fatal_error                    : 1;
    uint8_t i2c_error                      : 1;
    uint8_t algorithm_error                : 1;
    uint8_t calibration_error              : 1;
    uint8_t self_diag_error                : 1;
    uint8_t out_of_range                   : 1;
    uint8_t memory_error                   : 1;
    uint8_t no_measurement_completed       : 1;

    uint8_t low_internal_regulator_voltage : 1;
    uint8_t measurement_timeout            : 1;
    uint8_t abnormal_signal_level          : 1;
    uint8_t reserved                       : 4;
    uint8_t scale_factor_error             : 1;
};
NRF_STATIC_ASSERT(sizeof(struct error_status_t) == 2, "error_status_t size wrong");

struct fw_rev_t {
    uint8_t main;
    uint8_t sub;
};
NRF_STATIC_ASSERT(sizeof(struct fw_rev_t) == 2, "fw_rev_t size wrong");

struct product_code_t {
    /* Product code (null-terminated ASCII string) */
    char code[16];
};
NRF_STATIC_ASSERT(sizeof(struct product_code_t) == 16, "product_code_t size wrong");

struct calibration_status_t {
    uint8_t _reserved_0 : 1;
    uint8_t _reserved_1 : 1;
    uint8_t factory_calibration_restored : 1;
    uint8_t abc_calibration : 1;
    uint8_t target_calibration : 1;
    uint8_t background_calibration : 1;
    uint8_t zero_calibration : 1;
    uint8_t _reserved_7 : 1;
};
NRF_STATIC_ASSERT(sizeof(struct calibration_status_t) == 1, "calibration_status_t size wrong");

/* If ABC is enabled in single measurement mode and the sensor
 * is powered down between measurements, these registers must
 * be read from the sensor after each measurement and written
 * back to the sensor after each power on (enable) before a new
 * measurement is triggered. */
struct abc_parameters_t {
    /* Time passed since last ABC calibration (hours) */
    uint16_t time;
    /* Parameters */
    uint8_t par[4];
};
NRF_STATIC_ASSERT(sizeof(struct abc_parameters_t) == 6, "abc_parameters_t size wrong");

/* If IIR filter is enabled in single measurement mode and sensor
 * is powered down between measurements, these registers must
 * be read from the sensor after each measurement and written
 * back to the sensor after each power on (enable) before a new
 * measurement is triggered. */
struct filter_parameters_t {
    /* Parameters */
    uint16_t par[7];
};
NRF_STATIC_ASSERT(sizeof(struct filter_parameters_t) == 14, "filter_parameters_t size wrong");

struct meter_control_t {
    uint8_t nrdy_disable : 1;
    uint8_t abc_disabled  : 1;
    uint8_t static_iir_filter_dis : 1;
    uint8_t dynamic_iir_filter_dis : 1;
    uint8_t pressure_compensation_dis : 1;
    uint8_t nrdy_invert_disabled : 1;
    uint8_t reserved_6 : 1;
    uint8_t reserved_7 : 1;
};
NRF_STATIC_ASSERT(sizeof(struct meter_control_t) == 1, "meter_control_t size wrong");

#pragma pack(pop)


enum calibration_cmd_t {
    CAL_RestoreFactoryCalibration = 0x7c02,
    CAL_ForcedAbcCalibration = 0x7c03,
    CAL_TargetCalibration = 0x7c05,
    CAL_BackgroundCalibration = 0x7c06,
    CAL_ZeroCalibration = 0x7c07,
};

enum measurement_mode_t {
    MODE_Continuous = 0,
    MODE_Single = 1,
};

enum scr_t {
    RESTART_SENSOR = 0xFF,
};


static inline void msleep_for_sure(int32_t ms) {
    while (ms > 0) {
        ms -= k_msleep(ms);
    }
}


#define REG_RO(REGADDR,NAME,CTYPE,XFRM) \
    static inline int Read_##NAME(CTYPE* out) { \
        int result = i2c_burst_read(dev_i2c, ADDR, REGADDR, (uint8_t*)out, sizeof(CTYPE)); \
        *out = XFRM(*out); \
        return result; \
    }
#define REG_RW(REGADDR,NAME,CTYPE,XFRM) \
    REG_RO(REGADDR,NAME,CTYPE,XFRM) \
    static inline int Write_##NAME(CTYPE value) { \
        value = XFRM(value); \
        int result = i2c_burst_write(dev_i2c, ADDR, REGADDR, (uint8_t*)&value, sizeof(value)); \
        return result; \
    }
#define REG_EE(REGADDR,NAME,CTYPE,XFRM) \
    REG_RO(REGADDR,NAME,CTYPE,XFRM) \
    static inline int Write_##NAME(CTYPE value) { \
        value = XFRM(value); \
        int result = i2c_burst_write(dev_i2c, ADDR, REGADDR, (uint8_t*)&value, sizeof(value)); \
        if (result == 0) { \
            msleep_for_sure(EEPROM_UPDATE_DELAY_MS); \
        } \
        return result; \
    }

REG_RO(0x00, ErrorStatus, struct error_status_t,)
/* Measured concentration (ppm). Filtered. Pressure compensated. */
REG_RO(0x06, MeasuredConcentration_Filtered_PressureCompensated, int16_t, sys_be16_to_cpu)
/* Device temperature (°C * 100)
 * Example: 2223 means 22.23°C */
REG_RO(0x08, Temperature, int16_t, sys_be16_to_cpu)
/* Counter incremented after each measurement, 0-255. Wraps. */
REG_RO(0x0d, MeasurementCount, uint8_t,)
/* Time in present measurement cycle, in 2 second increments.
 * 0 when new measurement cycle started.
 * Example: value of 3 means 6 seconds passed in current measurement cycle */
REG_RO(0x0e, MeasurementCycleTime, uint16_t, sys_be16_to_cpu)
/* Measured concentration (ppm). Unfiltered. Pressure compensated. */
REG_RO(0x10, MeasuredConcentration_Unfiltered_PressureCompensated, int16_t, sys_be16_to_cpu)
/* Measured concentration (ppm). Filtered. Not pressure compensated. */
REG_RO(0x12, MeasuredConcentration_Filtered, int16_t, sys_be16_to_cpu)
/* Measured concentration (ppm). Unfiltered. Not pressure compensated. */
REG_RO(0x14, MeasuredConcentration_Unfiltered, int16_t, sys_be16_to_cpu)
/* Measured concentration, scaled by (TODO) */
REG_RO(0x1a, ScaledMeasuredConcentration, int16_t, sys_be16_to_cpu)
/* Counts while sensor is powered up. Unit: hours */
REG_RO(0x2a, ElapsedTimeCounter, uint32_t, sys_be32_to_cpu)
REG_RO(0x2f, FirmwareType, uint8_t,)
REG_RO(0x38, FirmwareRevision, struct fw_rev_t,)
REG_RO(0x3a, SensorID, uint32_t, sys_be32_to_cpu)
REG_RO(0x70, ProductCode, struct product_code_t,)

REG_RW(0x81, CalibrationStatus, struct calibration_status_t,)
REG_RW(0x82, CalibrationCommand, enum calibration_cmd_t,)
REG_RW(0x84, CalibrationTarget, int16_t, sys_be16_to_cpu)
REG_RW(0x86, MeasuredConcentrationOverride, uint16_t, sys_be16_to_cpu)
REG_RW(0x88, AbcParameters, struct abc_parameters_t,)
REG_RW(0x93, StartSingleMeasurement, uint8_t,)

REG_EE(0x95, MeasurementMode, enum measurement_mode_t,)
/* Measurement period (seconds). Only used in continuous mode.
 * Sensor reset required after modifying. */
//TODO: Validate the value, range is 2-65534 inclusive
REG_EE(0x96, MeasurementPeriod, uint16_t, sys_be16_to_cpu)
/* Number of samples in one measurement (1-1024, inclusive).
 * Sensor reset required after modifying. */
REG_EE(0x98, NumberOfSamples, uint16_t, sys_be16_to_cpu)
/* Period for ABC cycle (hours) */
REG_EE(0x9a, AbcPeriod, uint16_t, sys_be16_to_cpu)

/* Write any number to clear the ErrorStatus register */
REG_RW(0x9d, ClearErrorStatus, uint8_t,)

/* Target value for background and ABC calibrations (ppm) */
REG_EE(0x9e, AbcTarget, uint16_t, sys_be16_to_cpu)
/* Parameter for static IIR filter (2-10 inclusive).
 * Higher value corresponds to harder filtration. */
REG_EE(0xa1, StaticIirFilterParameter, uint8_t,)

/* Write 0xFF to restart the sensor */
REG_RW(0xa3, SCR, enum scr_t,)

REG_EE(0xa5, MeterControl, struct meter_control_t,)
/* Sensor address (1-127 inclusive) - default is 104 (0x68).
 * Sensor reset required after modifying.
 * Sensor does not validate the I2C address, DO NOT SET RESERVED OR INVALID ADDRESSES */
REG_EE(0xa7, SensorAddress, uint8_t,)
/* Concentration scale factor (numerator).
 * If both numerator and denominator are set to 0xFFFF, scaling is disabled.
 * Sensor reset required after modifying both values. */
REG_EE(0xa8, ConcentrationScaleFactor_Numerator, uint16_t, sys_be16_to_cpu)
/* Concentration scale factor (denominator).
 * If both numerator and denominator are set to 0xFFFF, scaling is disabled.
 * Sensor reset required after modifying both values. */
REG_EE(0xaa, ConcentrationScaleFactor_Denominator, uint16_t, sys_be16_to_cpu)

REG_RW(0xac, ScaledCalibrationTarget, uint16_t, sys_be16_to_cpu)
REG_RW(0xae, ScaledMeasuredConcentrationOverride, uint16_t, sys_be16_to_cpu)

REG_EE(0xb0, ScaledAbcTarget, uint16_t, sys_be16_to_cpu)

REG_RW(0xc1, Mirror_CalibrationStatus, struct calibration_status_t,)
REG_RW(0xc3, Mirror_StartSingleMeasurement, uint8_t,)
REG_RW(0xc4, Mirror_AbcParameters, struct abc_parameters_t,)

REG_RW(0xce, FilterParameters, struct filter_parameters_t,)

/* Barometric air pressure (unit: 0.1 hPa)
 * Range 3,000 - 13,000 (300 - 1300 hPa)
 * 
 * Values outside pressure range will set "out of range" error flag
 * and pressure compensation will proceed with min or max allowed
 * pressure value. */
REG_RW(0xdc, BarometricAirPressure, int16_t, sys_be16_to_cpu)

/* If pressure compensation and ABC are both enabled when the sensor
 * is used in single measurement mode, and powered down between
 * measurements, this register must be read from the sensor after each
 * measurement and written back to the sensor after each power on (enable)
 * before a new measurement is triggered. */
REG_RW(0xde, AbcBarometricPressure, int16_t, sys_be16_to_cpu)

#undef REG_EE
#undef REG_RW
#undef REG_RO


static inline int _wakeup() {
    uint8_t buf = 0;
    struct i2c_msg msg = {
        .buf = &buf,
        .len = 0,
        .flags = I2C_MSG_WRITE | I2C_MSG_STOP,
    };

    int result = 0;
    for (size_t retries = 0; retries < MAX_RETRIES; retries++) {
        result = i2c_transfer(dev_i2c, &msg, 1, ADDR);
        if (result == 0) {
            break;
        } else {
            LOG_WRN("wake-up retry %zu/%zu failed", retries+1, MAX_RETRIES);
        }
    }
    return result;
}

#define CHK(_ACT) do { \
    const int result = _ACT; \
    if (result != 0) { \
        LOG_ERR("Action %s failed with %d (%s)", #_ACT, result, strerror(result)); \
        state.is_faulted = true; \
        return result; \
    } \
} while(0)

int sunrise_init()
{
    CHK(_wakeup());
    
    uint8_t fw_type;
    uint32_t sensor_id;
    uint32_t elapsed_time_hrs;
    struct fw_rev_t fw_rev;
    struct product_code_t product_code;
    struct error_status_t error_status;
    struct calibration_status_t calibration_status;
    struct meter_control_t meter_ctrl;
    enum measurement_mode_t measurement_mode;
    CHK(Read_FirmwareType(&fw_type));
    CHK(Read_SensorID(&sensor_id));
    CHK(Read_FirmwareRevision(&fw_rev));
    CHK(Read_ElapsedTimeCounter(&elapsed_time_hrs));
    CHK(Read_ProductCode(&product_code));
    CHK(Read_ErrorStatus(&error_status));
    CHK(Read_CalibrationStatus(&calibration_status));
    CHK(Read_MeterControl(&meter_ctrl));
    CHK(Read_MeasurementMode(&measurement_mode));

    //TODO: ensure product code contains a null terminator and only printable ascii,
    // else print the hexdump of it instead

    LOG_DBG("FW Type: %u  Rev: %u.%u", fw_type, fw_rev.main, fw_rev.sub);
    LOG_DBG("Sensor ID: %08x", sensor_id);
    LOG_DBG("Product Code: %s", product_code.code);
    LOG_DBG("Elapsed Time: %u hours", elapsed_time_hrs);
    LOG_DBG("Measurement Mode: %02x", measurement_mode);

    LOG_DBG("Error Status: %04x", *(uint16_t*)&error_status);
    LOG_DBG("                     fatal_error: %d", error_status.fatal_error);
    LOG_DBG("                       i2c_error: %d", error_status.i2c_error);
    LOG_DBG("                 algorithm_error: %d", error_status.algorithm_error);
    LOG_DBG("               calibration_error: %d", error_status.calibration_error);
    LOG_DBG("                 self_diag_error: %d", error_status.self_diag_error);
    LOG_DBG("                    out_of_range: %d", error_status.out_of_range);
    LOG_DBG("                    memory_error: %d", error_status.memory_error);
    LOG_DBG("        no_measurement_completed: %d", error_status.no_measurement_completed);
    LOG_DBG("  low_internal_regulator_voltage: %d", error_status.low_internal_regulator_voltage);
    LOG_DBG("             measurement_timeout: %d", error_status.measurement_timeout);
    LOG_DBG("           abnormal_signal_level: %d", error_status.abnormal_signal_level);
    LOG_DBG("                        reserved: %d", error_status.reserved);
    LOG_DBG("              scale_factor_error: %d", error_status.scale_factor_error);

    LOG_DBG("Calibration Status: %02x", *(uint8_t*)&calibration_status);
    LOG_DBG("                   _reserved_0: %d", calibration_status._reserved_0);
    LOG_DBG("                   _reserved_1: %d", calibration_status._reserved_1);
    LOG_DBG("  factory_calibration_restored: %d", calibration_status.factory_calibration_restored);
    LOG_DBG("               abc_calibration: %d", calibration_status.abc_calibration);
    LOG_DBG("            target_calibration: %d", calibration_status.target_calibration);
    LOG_DBG("        background_calibration: %d", calibration_status.background_calibration);
    LOG_DBG("              zero_calibration: %d", calibration_status.zero_calibration);
    LOG_DBG("                   _reserved_7: %d", calibration_status._reserved_7);

    LOG_DBG("Meter Control: %02x", *(uint8_t*)&meter_ctrl);
    LOG_DBG("               nrdy_disable: %d", meter_ctrl.nrdy_disable);
    LOG_DBG("               abc_disabled: %d", meter_ctrl.abc_disabled);
    LOG_DBG("      static_iir_filter_dis: %d", meter_ctrl.static_iir_filter_dis);
    LOG_DBG("     dynamic_iir_filter_dis: %d", meter_ctrl.dynamic_iir_filter_dis);
    LOG_DBG("  pressure_compensation_dis: %d", meter_ctrl.pressure_compensation_dis);
    LOG_DBG("       nrdy_invert_disabled: %d", meter_ctrl.nrdy_invert_disabled);
    LOG_DBG("                 reserved_6: %d", meter_ctrl.reserved_6);
    LOG_DBG("                 reserved_7: %d", meter_ctrl.reserved_7);

    return 0;
}

int sunrise_is_ready(bool* is_ready)
{
    uint8_t count;
    *is_ready = false;
    CHK(Read_MeasurementCount(&count));
    if (state.count != count) {
        *is_ready = true;
        LOG_INF("count: %d -> %d", state.count, count);
        state.count = count;
    }
    return 0;
}

int sunrise_read()
{
    int16_t ppm_filtered;
    int16_t ppm_filtered_compensated;
    CHK(Read_MeasuredConcentration_Filtered(&ppm_filtered));
    CHK(Read_MeasuredConcentration_Filtered_PressureCompensated(&ppm_filtered_compensated));
    LOG_INF("co2: %d ppm (filtered)", ppm_filtered);
    LOG_INF("co2: %d ppm (filtered, pressure compensated)", ppm_filtered_compensated);

    int16_t temp;
    CHK(Read_Temperature(&temp));

    current_readings.sunrise.temp = (float)temp / 100.;
    current_readings.sunrise.ppm_filtered_compensated = ppm_filtered_compensated;
    current_readings.sunrise.uptime_last_updated = k_uptime_get();

    return 0;
}

bool sunrise_is_faulted()
{
    return false;
}

SENSOR_DEFINE(sunrise);

//eof
