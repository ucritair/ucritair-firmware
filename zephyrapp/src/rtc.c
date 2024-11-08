#include <zephyr/init.h>
#include <hal/nrf_rtc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(rtc, LOG_LEVEL_DBG);

#include "rtc.h"

static int board_cat_init_rtc(void)
{
    nrf_rtc_prescaler_set(NRF_RTC0, 4095); // 125ms/tick
	nrf_rtc_task_trigger(NRF_RTC0, NRF_RTC_TASK_START);

    return 0;
}

SYS_INIT(board_cat_init_rtc, PRE_KERNEL_1,
     CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);


// epoch time seconds of RTC=0
PERSIST_RAM uint32_t rtc_offset;

// 0xdeadbeef if retained ram is valid
PERSIST_RAM uint16_t rtc_init_check;

#define RTC_INIT_CHECK_MAGIC 0xb887

bool is_first_init = false;

time_t get_current_rtc_time()
{
	return (nrf_rtc_counter_get(NRF_RTC0) + rtc_offset) / 8;
}

void zero_rtc_counter()
{
	LOG_DBG("zero_rtc_counter");
	rtc_offset = -nrf_rtc_counter_get(NRF_RTC0);
}

void snapshot_rtc_for_reboot()
{
	rtc_offset = rtc_offset + nrf_rtc_counter_get(NRF_RTC0) + 1;
}

void check_rtc_init()
{
	if (rtc_init_check != RTC_INIT_CHECK_MAGIC)
	{
		is_first_init = true;
		rtc_offset = 0;
		rtc_init_check = RTC_INIT_CHECK_MAGIC;
	}
}

void update_rtc()
{
	if (nrf_rtc_counter_get(NRF_RTC0) >= (1<<10))
	{
		// Theory: wait until counter ticks, then reset it and add the
		// value we just cleared to the offset

		int new;
		while ((new = nrf_rtc_counter_get(NRF_RTC0)) == c) {}

		nrf_rtc_task_trigger(NRF_RTC0, NRF_RTC_TASK_CLEAR);
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