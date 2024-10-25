#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

#include "sensor.h"
#include <zephyr/sys/byteorder.h>

LOG_MODULE_REGISTER(sunrise);

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
    int16_t pressure_0p1_hpa;
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
        ms = k_msleep(ms);
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
        LOG_ERR("Action %s failed with %d (%s)\n", #_ACT, result, strerror(result)); \
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

    printf("FW Type: %u  Rev: %u.%u\n", fw_type, fw_rev.main, fw_rev.sub);
    printf("Sensor ID: %08x\n", sensor_id);
    printf("Product Code: %s\n", product_code.code);
    printf("Elapsed Time: %u hours\n", elapsed_time_hrs);
    printf("Measurement Mode: %02x\n", measurement_mode);

    printf("Error Status: %04x\n", *(uint16_t*)&error_status);
    printf("                     fatal_error: %d\n", error_status.fatal_error);
    printf("                       i2c_error: %d\n", error_status.i2c_error);
    printf("                 algorithm_error: %d\n", error_status.algorithm_error);
    printf("               calibration_error: %d\n", error_status.calibration_error);
    printf("                 self_diag_error: %d\n", error_status.self_diag_error);
    printf("                    out_of_range: %d\n", error_status.out_of_range);
    printf("                    memory_error: %d\n", error_status.memory_error);
    printf("        no_measurement_completed: %d\n", error_status.no_measurement_completed);
    printf("  low_internal_regulator_voltage: %d\n", error_status.low_internal_regulator_voltage);
    printf("             measurement_timeout: %d\n", error_status.measurement_timeout);
    printf("           abnormal_signal_level: %d\n", error_status.abnormal_signal_level);
    printf("                        reserved: %d\n", error_status.reserved);
    printf("              scale_factor_error: %d\n", error_status.scale_factor_error);

    printf("Calibration Status: %02x\n", *(uint8_t*)&calibration_status);
    printf("                   _reserved_0: %d\n", calibration_status._reserved_0);
    printf("                   _reserved_1: %d\n", calibration_status._reserved_1);
    printf("  factory_calibration_restored: %d\n", calibration_status.factory_calibration_restored);
    printf("               abc_calibration: %d\n", calibration_status.abc_calibration);
    printf("            target_calibration: %d\n", calibration_status.target_calibration);
    printf("        background_calibration: %d\n", calibration_status.background_calibration);
    printf("              zero_calibration: %d\n", calibration_status.zero_calibration);
    printf("                   _reserved_7: %d\n", calibration_status._reserved_7);

    printf("Meter Control: %02x\n", *(uint8_t*)&meter_ctrl);
    printf("               nrdy_disable: %d\n", meter_ctrl.nrdy_disable);
    printf("               abc_disabled: %d\n", meter_ctrl.abc_disabled);
    printf("      static_iir_filter_dis: %d\n", meter_ctrl.static_iir_filter_dis);
    printf("     dynamic_iir_filter_dis: %d\n", meter_ctrl.dynamic_iir_filter_dis);
    printf("  pressure_compensation_dis: %d\n", meter_ctrl.pressure_compensation_dis);
    printf("       nrdy_invert_disabled: %d\n", meter_ctrl.nrdy_invert_disabled);
    printf("                 reserved_6: %d\n", meter_ctrl.reserved_6);
    printf("                 reserved_7: %d\n", meter_ctrl.reserved_7);

    if (meter_ctrl.pressure_compensation_dis) {
        LOG_INF("Enabling pressure compensation");
        meter_ctrl.pressure_compensation_dis = 0;
        CHK(Write_MeterControl(meter_ctrl));
    }

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
    return 0;
}

bool sunrise_is_faulted()
{
    return false;
}

int sunrise_update_pressure(const float pressure_hpa)
{
    const int16_t pressure_0p1_hpa = pressure_hpa * 10;
    if (state.pressure_0p1_hpa == pressure_0p1_hpa) {
        return 0;
    }
    CHK(Write_BarometricAirPressure(pressure_0p1_hpa));
    state.pressure_0p1_hpa = pressure_0p1_hpa;
    LOG_INF("pressure update: %d e-1 hPa", pressure_0p1_hpa);
    return 0;
}

SENSOR_DEFINE(sunrise);

//eof
