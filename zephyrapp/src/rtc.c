#include <zephyr/init.h>
#include <hal/nrf_rtc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(rtc, LOG_LEVEL_DBG);

#include <zephyr/sys/timeutil.h>

#include "rtc.h"

static int board_cat_init_rtc(void)
{
    nrf_rtc_prescaler_set(HW_RTC_CHOSEN, 4095); // 125ms/tick
	nrf_rtc_task_trigger(HW_RTC_CHOSEN, NRF_RTC_TASK_START);

    return 0;
}

SYS_INIT(board_cat_init_rtc, PRE_KERNEL_1,
     CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);


// epoch time seconds of RTC=0
PERSIST_RAM uint64_t rtc_offset;

// 0xdeadbeef if retained ram is valid
PERSIST_RAM uint16_t rtc_init_check;

// seconds between sample
PERSIST_RAM uint16_t sensor_wakeup_rate;

// true if we woke to sample
PERSIST_RAM uint8_t wakeup_is_from_timer;

#define RTC_INIT_CHECK_MAGIC 0xb887

bool is_first_init = false;

_Static_assert(sizeof(time_t) == sizeof(uint64_t));

time_t get_current_rtc_time()
{
	return ((uint64_t)nrf_rtc_counter_get(HW_RTC_CHOSEN) + rtc_offset) / 8;
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
	LOG_DBG("set_rtc_counter");
	rtc_offset = timeutil_timegm64(t);
	rtc_offset *= 8;
	rtc_offset -= nrf_rtc_counter_get(HW_RTC_CHOSEN);
}

void snapshot_rtc_for_reboot()
{
	rtc_offset = rtc_offset + (uint64_t)nrf_rtc_counter_get(HW_RTC_CHOSEN) + 1;
}

void check_rtc_init()
{
	if (rtc_init_check != RTC_INIT_CHECK_MAGIC)
	{
		is_first_init = true;
		rtc_init_check = RTC_INIT_CHECK_MAGIC;
		sensor_wakeup_rate = 10;
		wakeup_is_from_timer = false;
		zero_rtc_counter();
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