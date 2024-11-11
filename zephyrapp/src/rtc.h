#include <time.h>

extern bool is_first_init;
extern uint64_t rtc_offset;
extern uint16_t sensor_wakeup_rate;
extern uint8_t wakeup_is_from_timer;

#define MIN_WAKEUP_RATE_TO_DEEP_SLEEP 120

#define HW_RTC_CHOSEN NRF_RTC0

#define RTC_TIME_TO_EPOCH_TIME(x) (x - 59958144000)

void zero_rtc_counter();
void check_rtc_init();
void snapshot_rtc_for_reboot();
time_t get_current_rtc_time();
void update_rtc();
void set_rtc_counter(struct tm* t);

#define PERSIST_RAM __attribute__((__section__(".endmap_presist_region")))

extern char* month_names[12];