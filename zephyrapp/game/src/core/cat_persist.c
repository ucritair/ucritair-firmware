#include "cat_persist.h"

// epoch time seconds of RTC=0
PERSIST_RAM uint64_t rtc_offset;

// 0xdeadbeef if retained ram is valid
PERSIST_RAM uint32_t rtc_init_check;

// seconds between sample
PERSIST_RAM uint16_t sensor_wakeup_period;
PERSIST_RAM uint8_t nox_sample_period;
PERSIST_RAM uint8_t nox_sample_counter;

// true if we woke to sample
PERSIST_RAM uint8_t wakeup_is_from_timer;
PERSIST_RAM uint64_t sleep_timestamp;

PERSIST_RAM uint8_t pet_mood;
PERSIST_RAM uint8_t pet_mask;
PERSIST_RAM char pet_name[64];
PERSIST_RAM uint16_t pet_level;

PERSIST_RAM uint8_t screen_brightness;
PERSIST_RAM uint8_t led_brightness;
PERSIST_RAM uint16_t dim_after_seconds;
PERSIST_RAM uint16_t sleep_after_seconds;

PERSIST_RAM CAT_AQ_score_block aq_moving_scores;
PERSIST_RAM uint64_t aq_moving_score_samples;
PERSIST_RAM uint64_t aq_moving_score_time;

PERSIST_RAM CAT_AQ_score_block aq_weekly_scores[7];
PERSIST_RAM uint8_t aq_weekly_score_head;
PERSIST_RAM uint64_t aq_weekly_score_time;

PERSIST_RAM CAT_AQ_crisis_state aq_crisis_state;
PERSIST_RAM CAT_pet_timing_state pet_timing_state;
PERSIST_RAM wifi_ap_record_t wifi_details;
PERSIST_RAM char wifi_password[MAX_PASSWORD_LEN];
PERSIST_RAM uint8_t wifi_status;

PERSIST_RAM uint64_t last_sensor_timestamp;
PERSIST_RAM uint64_t last_log_timestamp;

PERSIST_RAM uint64_t persist_flags;

bool is_persist_fresh;