#include <time.h>
#include <stdbool.h>

extern bool is_first_init;

extern volatile bool guy_is_wearing_mask;
extern volatile uint8_t guy_happiness;
extern volatile uint64_t rtc_offset;
extern volatile uint16_t sensor_wakeup_rate;
extern volatile uint8_t wakeup_is_from_timer;
extern volatile uint64_t went_to_sleep_at;
extern volatile uint8_t screen_brightness;

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

#define PERSIST_RAM __attribute__((__section__(".endmap_presist_region"))) volatile

extern char* month_names[12];
