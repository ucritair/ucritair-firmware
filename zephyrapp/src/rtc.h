#include <time.h>
#include <stdbool.h>
#include "cat_aqi.h"
#include "cat_crisis.h"
#include "cat_pet.h"

extern bool is_first_init;

extern volatile bool guy_is_wearing_mask;
extern volatile char guy_name[64]; // CHANGE THIS, RISK DESTROYING LOGS
extern volatile uint8_t guy_happiness;
extern volatile uint16_t guy_level;

extern volatile uint64_t rtc_offset;
extern volatile uint8_t nox_every_n_samples;
extern volatile uint8_t nox_every_n_samples_counter;
extern volatile uint16_t sensor_wakeup_rate;
extern volatile uint8_t wakeup_is_from_timer;
extern volatile uint64_t went_to_sleep_at;
extern volatile uint8_t screen_brightness;
extern volatile uint16_t dim_after_seconds;
extern volatile uint16_t sleep_after_seconds;

//////////////////////////////////////////////////////////
// AQ SPARKLINE STORE

extern volatile CAT_AQ_score_block aq_moving_scores;
extern volatile uint64_t aq_last_moving_score_time;

extern volatile CAT_AQ_score_block aq_score_buffer[7];
extern volatile uint8_t aq_score_head;
extern volatile uint64_t aq_last_buffered_score_time;

//////////////////////////////////////////////////////////
// MISC. STATE CHUNKS

extern volatile CAT_AQ_crisis_state aq_crisis_state;
extern volatile CAT_pet_timing_state pet_timing_state;
extern volatile uint64_t persist_flags;

#include "cat_pet.h"
_Static_assert(sizeof(guy_name) == sizeof(pet.name));

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
