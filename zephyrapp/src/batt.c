#include <stdio.h>
#include <string.h>
#include <zephyr/drivers/adc.h>
#include <hal/nrf_saadc.h>

/* ─────────────────────────── ADC configuration ─────────────────────────── */
#define ADC_RESOLUTION          10
#define ADC_GAIN                ADC_GAIN_1_4
#define ADC_REFERENCE           ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME    ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40)

#define ADC_1ST_CHANNEL_ID      0
#define ADC_1ST_CHANNEL_INPUT   NRF_SAADC_INPUT_AIN0

#define VREF_MV                 600U   /* Internal reference */
#define ADC_MAX_VALUE           1023U  /* 10‑bit full‑scale */
#define DIVIDER_RATIO_NUM       2U     /* undo 1∕2 divider  */
#define DIVIDER_RATIO_DEN       1U
#define GAIN_NUM                4U     /* undo 1∕4 gain     */
#define GAIN_DEN                1U

/* Low‑battery annunciation threshold */
#define BATTERY_LOW_PCT         20

const struct device *adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));

static const struct adc_channel_cfg m_1st_channel_cfg = {
    .gain             = ADC_GAIN,
    .reference        = ADC_REFERENCE,
    .acquisition_time = ADC_ACQUISITION_TIME,
    .channel_id       = ADC_1ST_CHANNEL_ID,
#if defined(CONFIG_ADC_CONFIGURABLE_INPUTS)
    .input_positive   = ADC_1ST_CHANNEL_INPUT,
#endif
};

#define BUFFER_SIZE 1
static int16_t m_sample_buffer[BUFFER_SIZE];

/* ── Voltage → SoC lookup (5 % … 95 % + explicit 20 %) ───────────────────── */
typedef struct {
    uint16_t mv;  /* cell voltage in mV */
    uint8_t  soc; /* associated SoC %  */
} soc_point_t;

static const soc_point_t soc_lut[] = {
    {3300,  0},           // empty cutoff 
    {3414,  5}, {3503, 15}, {3535, 20}, {3560, 25}, {3602, 35},
    {3642, 45}, {3698, 55}, {3783, 65}, {3871, 75}, {3963, 85},
    {4065, 95},
    {4200, 100}           //Pretty full
};
#define LUT_LEN (sizeof soc_lut / sizeof soc_lut[0])

/* ────────────────────────── Helper functions ───────────────────────────── */
static uint16_t adc_raw_to_batt_mv(int16_t raw)
{
    uint32_t mv = (uint32_t)raw * VREF_MV * GAIN_NUM * DIVIDER_RATIO_NUM;
    mv /= (ADC_MAX_VALUE * GAIN_DEN * DIVIDER_RATIO_DEN);
    return (uint16_t)mv;
}

static uint8_t mv_to_soc(uint16_t mv)
{
    const size_t last = LUT_LEN - 1;

    if (mv < soc_lut[0].mv)   return 0;     // below first anchor
    if (mv > soc_lut[last].mv) return 100;  // above last anchor

    for (size_t i = 1; i <= last; ++i) {
        if (mv <= soc_lut[i].mv) {
            uint16_t mv_lo  = soc_lut[i - 1].mv;
            uint16_t mv_hi  = soc_lut[i].mv;
            uint8_t  soc_lo = soc_lut[i - 1].soc;
            uint8_t  soc_hi = soc_lut[i].soc;

            uint16_t delta_mv = (uint16_t)(mv_hi - mv_lo);
            if (delta_mv == 0) return soc_hi;  // should not happen if LUT is sane

            // Rounded linear interpolation
            uint32_t num = (uint32_t)(mv - mv_lo) * (uint32_t)(soc_hi - soc_lo);
            num += delta_mv / 2; // for rounding instead of truncation
            return (uint8_t)(soc_lo + (num / delta_mv));
        }
    }

    return 100; // shouldn't hit
}

/* ────────────────────────── SAADC wrappers ─────────────────────────────── */
int adc_sample(void)
{
    const struct adc_sequence sequence = {
        .channels    = BIT(ADC_1ST_CHANNEL_ID),
        .buffer      = m_sample_buffer,
        .buffer_size = sizeof(m_sample_buffer),
        .resolution  = ADC_RESOLUTION,
    };

    int ret = adc_read(adc_dev, &sequence);
    if (ret) {
        printk("ADC read err: %d\n", ret);
        return -1;
    }
    return m_sample_buffer[0];
}

/* Restored to match original interface. */
float adc_get_voltage(void)
{
    return adc_raw_to_batt_mv(adc_sample()) / 1000.0f;
}

uint16_t adc_get_battery_mv(void)
{
    int16_t raw = adc_sample();
    if (raw < 0) {
        return 0;
    }
    return adc_raw_to_batt_mv(raw);
}

/* ────────────────────────── Public API ─────────────────────────────────── */
int get_battery_pct(void)
{
    return (int)mv_to_soc(adc_get_battery_mv());
}

int init_adc(void)
{
    printk("nRF53 SAADC battery monitor init\n");

    if (!device_is_ready(adc_dev)) {
        printk("ADC device not ready\n");
        return -ENODEV;
    }

    int err = adc_channel_setup(adc_dev, &m_1st_channel_cfg);
    if (err) {
        printk("ADC channel setup failed (%d)\n", err);
        return err;
    }

    /* Offset calibration; discard first sample */
    NRF_SAADC_S->TASKS_CALIBRATEOFFSET = 1;
    k_msleep(2);
    (void)adc_sample();

    return 0;
}