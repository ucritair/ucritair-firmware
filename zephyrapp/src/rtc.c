#include <zephyr/init.h>
#include <hal/nrf_rtc.h>
#include <nrfx_rtc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(rtc, LOG_LEVEL_DBG);

#include <zephyr/sys/timeutil.h>

#include "rtc.h"

#include "flash.h"
#include "lcd_driver.h"
#include "power_control.h"

#include <hal/nrf_gpio.h>
#include <soc/nrfx_coredep.h>

nrfx_rtc_t rtc_inst = NRFX_RTC_INSTANCE(0);

void rtc_irq_handler(nrfx_rtc_int_type_t int_type)
{
	if (int_type == NRFX_RTC_INT_TICK)
	{
		// LOG_DBG("Tick!!!");
	}
	else if (int_type == NRFX_RTC_INT_COMPARE3)
	{
		on_rtc_compare3();
	}
}

static int board_cat_init_rtc(void)
{
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 2));

    nrfx_rtc_config_t config = NRFX_RTC_DEFAULT_CONFIG;
    config.prescaler = NRF_RTC_FREQ_TO_PRESCALER(8); // 8Hz
    config.reliable = true;

    IRQ_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_RTC_INST_GET(0)), IRQ_PRIO_LOWEST,
              NRFX_RTC_INST_HANDLER_GET(0), 0, 0);

    nrfx_rtc_init(&rtc_inst, &config, &rtc_irq_handler);
    nrfx_rtc_tick_enable(&rtc_inst, true);

    nrfx_coredep_delay_us(101000); // as of SEA-1 we are 1.8945s - so 16 ticks (2.0) forward and then 105.5ms delay to get on time

    nrf_rtc_task_trigger(HW_RTC_CHOSEN, NRF_RTC_TASK_STOP);
    nrf_rtc_task_trigger(HW_RTC_CHOSEN, NRF_RTC_TASK_CLEAR);
    nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(0, 2));
    nrf_rtc_task_trigger(HW_RTC_CHOSEN, NRF_RTC_TASK_START);

    return 0;
}

SYS_INIT(board_cat_init_rtc, PRE_KERNEL_1,
     CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

void configure_rtc_timer3(int for_ms)
{
	nrfx_rtc_cc_set(&rtc_inst, 3, nrf_rtc_counter_get(HW_RTC_CHOSEN) + (for_ms/125), true);
}


// epoch time seconds of RTC=0
PERSIST_RAM uint64_t rtc_offset;

// 0xdeadbeef if retained ram is valid
PERSIST_RAM uint32_t rtc_init_check;

// seconds between sample
PERSIST_RAM uint16_t sensor_wakeup_rate;
PERSIST_RAM uint8_t nox_every_n_samples;
PERSIST_RAM uint8_t nox_every_n_samples_counter;

// true if we woke to sample
PERSIST_RAM uint8_t wakeup_is_from_timer;

PERSIST_RAM uint64_t went_to_sleep_at;

PERSIST_RAM uint8_t guy_happiness;
PERSIST_RAM bool guy_is_wearing_mask;
PERSIST_RAM char guy_name[64];
PERSIST_RAM uint16_t guy_level; // POTENTIAL DISASTER

PERSIST_RAM uint8_t screen_brightness;

PERSIST_RAM uint16_t dim_after_seconds;
PERSIST_RAM uint16_t sleep_after_seconds;

#define RTC_INIT_CHECK_MAGIC 0xb8870005

bool is_first_init = false;

_Static_assert(sizeof(time_t) == sizeof(uint64_t));

time_t get_current_rtc_time()
{
	return ((uint64_t)nrf_rtc_counter_get(HW_RTC_CHOSEN) + rtc_offset) / 8;
}

void set_rtc_counter_raw(uint64_t t)
{
	rtc_offset = t;
	rtc_offset *= 8;
	rtc_offset -= nrf_rtc_counter_get(HW_RTC_CHOSEN);
}

void zero_rtc_counter()
{
	LOG_DBG("zero_rtc_counter");
	struct tm t = {
		.tm_year = 2024 - TIME_UTILS_BASE_YEAR + (2024 - 124),
		// WHAT THE FUCK is going on here. I don't care.
		.tm_mon = 11-1,
		.tm_mday = 7
	};
	set_rtc_counter(&t);
}

void set_rtc_counter(struct tm* t)
{
	set_rtc_counter_raw(timeutil_timegm64(t));
}

void snapshot_rtc_for_reboot()
{
	// wait to synchronize to tick
	int c = nrf_rtc_counter_get(HW_RTC_CHOSEN);
	while (nrf_rtc_counter_get(HW_RTC_CHOSEN) == c) {}
	rtc_offset = rtc_offset + (uint64_t)nrf_rtc_counter_get(HW_RTC_CHOSEN) + 16; // as of SEA-1 we are 1.8945s - so 16 ticks (2.0) forward and then 105.5ms delay to get on time
} 

void check_rtc_init()
{
	if (rtc_init_check != RTC_INIT_CHECK_MAGIC)
	{
		is_first_init = true;
		rtc_init_check = RTC_INIT_CHECK_MAGIC;
		sensor_wakeup_rate = 3*60;
		nox_every_n_samples = 0;
		nox_every_n_samples_counter = 0;
		wakeup_is_from_timer = false;
		zero_rtc_counter();
		went_to_sleep_at = get_current_rtc_time();
		guy_happiness = 1;
		guy_is_wearing_mask = false;
		guy_level = 0;
		const char* waldo_str = "Waldo";
		for(int i = 0; waldo_str[i] < 5; i++)
			guy_name[i] = waldo_str[i];
		guy_name[5] = '\0';
		screen_brightness = BACKLIGHT_FULL;
		dim_after_seconds = 45;
		sleep_after_seconds = 120;
	}
}

void update_rtc()
{
	int c = nrf_rtc_counter_get(HW_RTC_CHOSEN);
	if (c >= (1<<10))
	{
		// Theory: wait until counter ticks, then reset it and add the
		// value we just cleared to the offset

		int new;
		while ((new = nrf_rtc_counter_get(HW_RTC_CHOSEN)) == c) {}

		nrf_rtc_task_trigger(HW_RTC_CHOSEN, NRF_RTC_TASK_CLEAR);
		rtc_offset += new;

		LOG_DBG("rtc prevent wrap");
	}
}

void continue_rtc_from_log()
{
	if (!is_first_init) return;
	if (next_log_cell_nr <= 0) return;

	CAT_log_cell cell;
	flash_get_cell_by_nr(next_log_cell_nr-1, &cell);
	set_rtc_counter_raw(cell.timestamp);
	went_to_sleep_at = get_current_rtc_time() - 10;
}

char* month_names[12] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};