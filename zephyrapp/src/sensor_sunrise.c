#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>

#include "sensor_hal.h"
#include "cat_core.h"

LOG_MODULE_REGISTER(sunrise, SENSOR_LOG_LEVEL);

/*------------- default CO2 value -----------------------------------*/
#define SUNRISE_DEFAULT_TARGET_PPM  427 // Mauna Loa Average June 2025. 

/* -------------------- byte-order helpers --------------------------- */
#define FROM_BE16(v) sys_be16_to_cpu(v)
#define TO_BE16(v)   sys_cpu_to_be16(v)
#define FROM_BE32(v) sys_be32_to_cpu(v)
#define TO_BE32(v)   sys_cpu_to_be32(v)
#define ID(x)        (x)                 /* identity transform         */

/* -------------------------- constants ------------------------------ */
#define TIMEOUT_MS              15       /* sensor auto-sleep window   */
#define EEPROM_UPDATE_DELAY_MS  25       /* Sunrise EE write time      */
static const size_t   MAX_RETRIES = 5;
static const uint16_t ADDR        = 0x68;

static const struct device *dev_i2c =
        DEVICE_DT_GET(DT_NODELABEL(arduino_i2c));

/* ------------------------- driver state ---------------------------- */
struct sunrise_state_t {
    uint8_t count;
    bool    is_faulted;
};
static struct sunrise_state_t state = { 0 };

/* ======================= register structs ========================= */
#pragma pack(push, 1)
#if !CONFIG_LITTLE_ENDIAN
#error "This driver assumes a little-endian MCU"
#endif

struct error_status_t {
    uint8_t fatal_error              :1;
    uint8_t i2c_error                :1;
    uint8_t algorithm_error          :1;
    uint8_t calibration_error        :1;
    uint8_t self_diag_error          :1;
    uint8_t out_of_range             :1;
    uint8_t memory_error             :1;
    uint8_t no_measurement_completed :1;

    uint8_t low_internal_regulator_voltage :1;
    uint8_t measurement_timeout            :1;
    uint8_t abnormal_signal_level          :1;
    uint8_t reserved                       :4;
    uint8_t scale_factor_error             :1;
};
NRF_STATIC_ASSERT(sizeof(struct error_status_t) == 2, "error_status_t");

struct fw_rev_t          { uint8_t main, sub; };
struct product_code_t    { char code[16];     };
struct calibration_status_t{
    uint8_t _r0:1,_r1:1,factory_calibration_restored:1,abc_calibration:1,
            target_calibration:1,background_calibration:1,
            zero_calibration:1,_r7:1;
};
struct abc_parameters_t   { uint16_t time; uint8_t par[4]; };
struct filter_parameters_t{ uint16_t par[7]; };
struct meter_control_t{
    uint8_t nrdy_disable:1,abc_disabled:1,static_iir_filter_dis:1,
            dynamic_iir_filter_dis:1,pressure_compensation_dis:1,
            nrdy_invert_disabled:1,reserved_6:1,reserved_7:1;
};
#pragma pack(pop)

/* ======================= Sunrise commands ========================= */
enum calibration_cmd_t{
    CAL_RestoreFactoryCalibration = 0x7c02,
    CAL_ForcedAbcCalibration      = 0x7c03,
    CAL_TargetCalibration         = 0x7c05,
    CAL_BackgroundCalibration     = 0x7c06,
    CAL_ZeroCalibration           = 0x7c07,
};
enum measurement_mode_t { MODE_Continuous=0, MODE_Single=1 };
enum scr_t              { RESTART_SENSOR  = 0xFF          };

/* ---------------- sleep helper (handles tick wrap) ---------------- */
static inline void msleep_for_sure(int32_t ms)
{
    while (ms > 0) ms = k_msleep(ms);
}

/* ========================= reg helpers ============================ */
#define REG_RO(REGADDR,NAME,CTYPE,XFRM_RD)                                   \
    static inline int Read_##NAME(CTYPE *out){                               \
        int rc = i2c_burst_read(dev_i2c, ADDR, REGADDR,                      \
                                (uint8_t *)out, sizeof(*out));               \
        *out = XFRM_RD(*out);                                                \
        return rc; }

#define REG_RW(REGADDR,NAME,CTYPE,XFRM_RD,XFRM_WR)                           \
    REG_RO(REGADDR,NAME,CTYPE,XFRM_RD)                                       \
    static inline int Write_##NAME(CTYPE v){                                 \
        v = XFRM_WR(v);                                                      \
        return i2c_burst_write(dev_i2c, ADDR, REGADDR,                       \
                               (uint8_t *)&v, sizeof(v)); }

#define REG_EE(REGADDR,NAME,CTYPE,XFRM_RD,XFRM_WR)                           \
    REG_RO(REGADDR,NAME,CTYPE,XFRM_RD)                                       \
    static inline int Write_##NAME(CTYPE v){                                 \
        v = XFRM_WR(v);                                                      \
        int rc = i2c_burst_write(dev_i2c, ADDR, REGADDR,                     \
                                 (uint8_t *)&v, sizeof(v));                  \
        if (!rc) msleep_for_sure(EEPROM_UPDATE_DELAY_MS);                    \
        return rc; }

/* =========================== register map ========================== */
REG_RW(0x00, ErrorStatus,                     struct error_status_t,      ID, ID)
/* -------- measurement data (RO) ----------------------------------- */
REG_RO(0x06, MeasuredConcentration_Filtered_PressureCompensated, int16_t, FROM_BE16)
REG_RO(0x08, Temperature,                    int16_t,                    FROM_BE16)
REG_RO(0x0d, MeasurementCount,               uint8_t,                    ID)
REG_RO(0x0e, MeasurementCycleTime,           uint16_t,                   FROM_BE16)
REG_RO(0x10, MeasuredConcentration_Unfiltered_PressureCompensated, int16_t, FROM_BE16)
REG_RO(0x12, MeasuredConcentration_Filtered, int16_t,                    FROM_BE16)
REG_RO(0x14, MeasuredConcentration_Unfiltered, int16_t,                  FROM_BE16)
REG_RO(0x1a, ScaledMeasuredConcentration,    int16_t,                    FROM_BE16)
REG_RO(0x2a, ElapsedTimeCounter,             uint32_t,                   FROM_BE32)
REG_RO(0x2f, FirmwareType,                   uint8_t,                    ID)
REG_RO(0x38, FirmwareRevision,               struct fw_rev_t,            ID)
REG_RO(0x3a, SensorID,                       uint32_t,                   FROM_BE32)
REG_RO(0x70, ProductCode,                    struct product_code_t,      ID)
/* -------- configuration / control --------------------------------- */
REG_RW(0x81, CalibrationStatus,              struct calibration_status_t, ID, ID)
/* note: command is 16-bit so write must be BE                                  */
REG_RW(0x82, CalibrationCommand,             uint16_t,      FROM_BE16, TO_BE16)
REG_RW(0x84, CalibrationTarget,              int16_t,                    FROM_BE16, TO_BE16)
REG_RW(0x86, MeasuredConcentrationOverride,  int16_t,                    FROM_BE16, TO_BE16)
REG_RW(0x88, AbcParameters,                  struct abc_parameters_t,    ID, ID)
REG_RW(0x93, StartSingleMeasurement,         uint8_t,                    ID, ID)

REG_EE(0x95, MeasurementMode,                uint16_t,    FROM_BE16, TO_BE16)
REG_EE(0x96, MeasurementPeriod,              uint16_t,                   FROM_BE16, TO_BE16)
REG_EE(0x98, NumberOfSamples,                uint16_t,                   FROM_BE16, TO_BE16)
REG_EE(0x9a, AbcPeriod,                      uint16_t,                   FROM_BE16, TO_BE16)
REG_RW(0x9d, ClearErrorStatus,               uint8_t,                    ID, ID)
REG_EE(0x9e, AbcTarget,                      uint16_t,                   FROM_BE16, TO_BE16)
REG_EE(0xa1, StaticIirFilterParameter,       uint8_t,                    ID, ID)
REG_RW(0xa3, SCR,                            enum scr_t,                 ID, ID)
REG_EE(0xa5, MeterControl,                   struct meter_control_t,     ID, ID)
REG_EE(0xa7, SensorAddress,                  uint8_t,                    ID, ID)
REG_EE(0xa8, ConcentrationScaleFactor_Numerator,   uint16_t, FROM_BE16, TO_BE16)
REG_EE(0xaa, ConcentrationScaleFactor_Denominator, uint16_t, FROM_BE16, TO_BE16)
REG_RW(0xac, ScaledCalibrationTarget,        uint16_t,                   FROM_BE16, TO_BE16)
REG_RW(0xae, ScaledMeasuredConcentrationOverride, uint16_t,              FROM_BE16, TO_BE16)
REG_EE(0xb0, ScaledAbcTarget,                uint16_t,                   FROM_BE16, TO_BE16)
REG_RW(0xc1, Mirror_CalibrationStatus,       struct calibration_status_t, ID, ID)
REG_RW(0xc3, Mirror_StartSingleMeasurement,  uint8_t,                    ID, ID)
REG_RW(0xc4, Mirror_AbcParameters,           struct abc_parameters_t,    ID, ID)
REG_RW(0xce, FilterParameters,               struct filter_parameters_t, ID, ID)
REG_RW(0xdc, BarometricAirPressure,          int16_t,                    FROM_BE16, TO_BE16)
REG_RW(0xde, AbcBarometricPressure,          int16_t,                    FROM_BE16, TO_BE16)

/* ===================== I²C wake-up helper ========================= */
static inline int _wakeup(void)
{
    uint8_t dummy = 0;
    struct i2c_msg msg = {
        .buf   = &dummy,
        .len   = 0,
        .flags = I2C_MSG_WRITE | I2C_MSG_STOP
    };
    for (size_t i = 0; i < MAX_RETRIES; i++)
        if (!i2c_transfer(dev_i2c, &msg, 1, ADDR))
            return 0;
    return -EIO;
}

/* ========================= error wrapper =========================== */
#define CHK(ACT) do { int rc = (ACT);                                      \
    if (rc) {                                                              \
        LOG_ERR("%s failed (%d)", #ACT, rc);                               \
        state.is_faulted = true;                                           \
        return rc;                                                         \
    } } while (0)

/* =========================== public API ============================ */
int sunrise_init(void)
{
    CHK(_wakeup());

    uint8_t fw_type;            CHK(Read_FirmwareType(&fw_type));
    struct fw_rev_t fw_rev;     CHK(Read_FirmwareRevision(&fw_rev));
    struct product_code_t pc;   CHK(Read_ProductCode(&pc));
    struct meter_control_t mc;
    CHK(Read_MeterControl(&mc));

    /*Senseair says to consider turning on ABC and set this to 6-9 months and hope we see 420 ppm at least once */
    if (mc.abc_disabled == 0) {       /* only touch EEPROM if we must   */
        mc.abc_disabled = 1;
        CHK(Write_MeterControl(mc));  /* EE write → 25 ms delay inside  */
        LOG_INF("Sunrise ABC set to disabled");
    }

    LOG_INF("Sunrise FW %u.%u  ProdCode=\"%s\"  ABC=%s",
        fw_rev.main, fw_rev.sub, pc.code,
        mc.abc_disabled ? "DISABLED" : "ENABLED");
    return 0;
}

int sunrise_is_ready(bool *ready)
{
    uint8_t c; *ready = false;
    CHK(Read_MeasurementCount(&c));
    if (c != state.count) { *ready = true; state.count = c; }
    return 0;
}

static void sunrise_dump_debug(void)
{
    uint16_t tgt;           CHK(Read_CalibrationTarget(&tgt));
    uint16_t abc_period;    CHK(Read_AbcPeriod(&abc_period));

    struct error_status_t es;

    int16_t  filt, filt_pc; CHK(Read_MeasuredConcentration_Filtered(&filt));
                            CHK(Read_MeasuredConcentration_Filtered_PressureCompensated(&filt_pc));

    struct calibration_status_t cs; CHK(Read_CalibrationStatus(&cs));

    CHK(Read_ErrorStatus(&es));


    LOG_INF("target_calibration bit = %d", cs.target_calibration);        /* 1 = OK            */
    LOG_INF("AbcPeriod = %u h", abc_period);                              /* 0 or 0xFFFF = off */
    LOG_INF("CalTarget = %u ppm", tgt);                                   /* 427 ppm expected  */
    LOG_INF("Raw        = %d ppm", filt);                                 /* ~400 ppm          */
    LOG_INF("Compensated = %d ppm", filt_pc);                             /* ~427 ppm          */

    if (*(uint16_t*)&es != 0) {
        LOG_WRN("Sunrise Error Status: fatal=%d, i2c=%d, alg=%d, cal=%d, diag=%d, range=%d, mem=%d, no_meas=%d",
                es.fatal_error, es.i2c_error, es.algorithm_error, es.calibration_error,
                es.self_diag_error, es.out_of_range, es.memory_error, es.no_measurement_completed);
        LOG_WRN("Sunrise Error Status (cont): vreg_low=%d, timeout=%d, signal_abnormal=%d, scale_err=%d",
                es.low_internal_regulator_voltage, es.measurement_timeout, es.abnormal_signal_level,
                es.scale_factor_error);
    }   
    return 0;
}

int sunrise_read(void)
{
    int16_t filt, filt_pc, temp;
    
    CHK(Read_MeasuredConcentration_Filtered(&filt));
    CHK(Read_MeasuredConcentration_Filtered_PressureCompensated(&filt_pc));
    CHK(Read_Temperature(&temp));

    readings.sunrise.temp                       = temp / 100.0f;
    readings.sunrise.ppm_filtered_uncompensated = filt;
    readings.sunrise.ppm_filtered_compensated   = filt_pc;
    readings.sunrise.uptime_last_updated        = k_uptime_get();

    LOG_INF("CO2 %d ppm (filt)", filt);
    LOG_INF("CO2 %d ppm (filt+comp)", filt_pc);
    //sunrise_dump_debug(); // Disabled in normal read unless we need this. 
    return 0;
}

bool sunrise_is_faulted(void)
{
    return state.is_faulted;
}


/* ----------------- helper: write barometric pressure --------------- */
int update_pressure_sunrise(float hPa)
{
    int16_t raw = (int16_t)(hPa * 10.0f + 0.5f);
    CHK(Write_BarometricAirPressure(raw));  /* macro swaps on write   */
    // LOG_DBG("Pressure %.1f hPa pushed to Sunrise", hPa);
    return 0;
}

/******************************************************************************
 *  Target / zero calibration helper
 ******************************************************************************/
static int force_abc_sunrise_target(uint16_t target_ppm)
{
    LOG_INF("Beginning calibration (%u ppm requested)", target_ppm);

    /* push latest pressure (if valid) */
    float p = readings.lps22hh.pressure;
    if (p > 300.f && p < 1300.f) CHK(update_pressure_sunrise(p));

    /* snapshot current MeterControl */
    struct meter_control_t mc_orig; CHK(Read_MeterControl(&mc_orig));

    /* clear previous status / errors */
    struct calibration_status_t cs_zero = {0};
    struct error_status_t       es_zero = {0};
    CHK(Write_CalibrationStatus(cs_zero));
    CHK(Write_ErrorStatus(es_zero));

    /* decide which command to send */
    if (target_ppm == 0) {
        /* ---- ZERO CALIBRATION ---------------------------------------- */
        CHK(Write_CalibrationCommand((uint16_t)CAL_ZeroCalibration));
        LOG_INF("Issued Zero-Calibration (0 ppm)");
    }
    else if (target_ppm == 69) {
        CHK(Write_CalibrationCommand((uint16_t)CAL_RestoreFactoryCalibration));
        LOG_INF("Restored Factory Cal"); // Dirty hack to do factory cal
    }
    else {
        /* ---- TARGET CALIBRATION -------------------------------------- */
        CHK(Write_CalibrationTarget(target_ppm));        /* BE16 helper */
        CHK(Write_CalibrationCommand((uint16_t)CAL_TargetCalibration));
        LOG_INF("Issued Target-Calibration to %u ppm", target_ppm);
    }

    /* poll completion flag */
    struct calibration_status_t cs;

    do {
        k_msleep(200);
        CHK(Read_CalibrationStatus(&cs));
    }
    while ( (target_ppm == 0 && !cs.zero_calibration) ||
            (target_ppm == 69 && !cs.factory_calibration_restored) || // Dirty hack to do factory cal
            (target_ppm > 0 && target_ppm != 69 && !cs.target_calibration) ); // Dirty hack to do factory cal

    LOG_INF("Calibration finished ✓");

    /* restore original MeterControl & clear status */
    CHK(Write_MeterControl(mc_orig));
    CHK(Write_CalibrationStatus(cs_zero));

    /*final diagnostics */
    sunrise_dump_debug();
    return 0;
}


/* --- legacy wrapper: keeps existing call-sites working ------------- */
int force_abc_sunrise(void)
{
    return force_abc_sunrise_target(SUNRISE_DEFAULT_TARGET_PPM);
}

/* --- new, explicit-parameter API ----------------------------------- */
int force_abc_sunrise_target_ppm(uint16_t ppm)
{
    return force_abc_sunrise_target(ppm);
}

/* ------------------------------------------------------------------ */
/*  Define Sensor :)*/
/* ------------------------------------------------------------------ */
SENSOR_DEFINE(sunrise);