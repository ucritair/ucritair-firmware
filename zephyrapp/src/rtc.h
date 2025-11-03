#include <time.h>
#include <stdbool.h>
#include "cat_air.h"
#include "cat_crisis.h"
#include "cat_pet.h"
#include "cat_persist.h"

_Static_assert(sizeof(time_t) == sizeof(uint64_t));
_Static_assert(sizeof(pet_name) == sizeof(pet.name));

#define MIN_WAKEUP_RATE_TO_DEEP_SLEEP 60

#define HW_RTC_CHOSEN NRF_RTC0

#define RTC_EPOCH_TIME_OFFSET 59958144000
#define RTC_TIME_TO_EPOCH_TIME(x) (x - RTC_EPOCH_TIME_OFFSET)
#define EPOCH_TIME_TO_RTC_TIME(x) (x + RTC_EPOCH_TIME_OFFSET)

void zero_rtc_counter();
void check_rtc_init();
void snapshot_rtc_for_reboot();
time_t get_current_rtc_time();
void update_rtc();
void set_rtc_counter(struct tm* t);
void set_rtc_counter_raw(uint64_t t);
void configure_rtc_timer3(int for_ms);

extern char* month_names[12];
