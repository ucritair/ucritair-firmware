#pragma once

#include <stdint.h>
#include "cat_aqi.h"
#include "cat_crisis.h"
#include "cat_pet.h"

#ifdef CAT_DESKTOP
#define PERSIST_RAM volatile
#else
#define PERSIST_RAM __attribute__((__section__(".endmap_presist_region"))) volatile
#endif

typedef enum
{
	CAT_PERSIST_CONFIG_FLAG_NONE = 0,
	CAT_PERSIST_CONFIG_FLAG_BATTERY_ALERT = (1 << 0),
	CAT_PERSIST_CONFIG_FLAG_MANUAL_ORIENT = (1 << 1),
	CAT_PERSIST_CONFIG_FLAG_USE_FAHRENHEIT = (1 << 2),
	CAT_PERSIST_CONFIG_FLAG_AQ_FIRST = (1 << 3),
	CAT_PERSIST_CONFIG_FLAG_PAUSE_CARE = (1 << 4),
} CAT_persist_config_flag;

extern volatile uint64_t rtc_offset;
extern volatile uint32_t rtc_init_check;

extern volatile uint16_t sensor_wakeup_period;
extern volatile uint8_t nox_sample_period;
extern volatile uint8_t nox_sample_counter;

extern volatile uint8_t wakeup_is_from_timer;
extern volatile uint64_t sleep_timestamp;

extern volatile uint8_t pet_mood;
extern volatile uint8_t pet_mask;
extern volatile char pet_name[64];
extern volatile uint16_t pet_level;

extern volatile uint8_t screen_brightness;
extern volatile uint8_t led_brightness;
extern volatile uint16_t dim_after_seconds;
extern volatile uint16_t sleep_after_seconds;

extern volatile CAT_AQ_score_block aq_moving_scores;
extern volatile uint64_t aq_moving_score_samples;
extern volatile uint64_t aq_moving_score_time;

extern volatile CAT_AQ_score_block aq_weekly_scores[7];
extern volatile uint8_t aq_weekly_score_head;
extern volatile uint64_t aq_weekly_score_time;

extern volatile CAT_AQ_crisis_state aq_crisis_state;
extern volatile CAT_pet_timing_state pet_timing_state;

extern volatile uint64_t persist_flags;

extern bool is_persist_fresh;