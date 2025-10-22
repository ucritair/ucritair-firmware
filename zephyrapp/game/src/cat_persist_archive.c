#ifdef CAT_DESKTOP

#include "cat_persist_archive.h"
#include "cat_persist.h"
#include <unistd.h>

typedef struct
{
	uint64_t rtc_offset;
	uint32_t rtc_init_check;
	uint16_t sensor_wakeup_period;
	uint8_t nox_sample_period;
	uint8_t nox_sample_counter;
	uint8_t wakeup_is_from_timer;
	uint64_t sleep_timestamp;
	uint8_t pet_mood;
	uint8_t pet_mask;
	char pet_name[64];
	uint16_t pet_level;
	uint8_t screen_brightness;
	uint8_t led_brightness;
	uint16_t dim_after_seconds;
	uint16_t sleep_after_seconds;
	CAT_AQ_score_block aq_moving_scores;
	uint64_t aq_moving_score_samples;
	uint64_t aq_moving_score_time;
	CAT_AQ_score_block aq_weekly_scores[7];
	uint8_t aq_weekly_score_head;
	uint64_t aq_weekly_score_time;
	CAT_AQ_crisis_state aq_crisis_state;
	CAT_pet_timing_state pet_timing_state;
	uint64_t persist_flags;
} CAT_persist_archive;

void CAT_write_persist_archive(int fd)
{
	CAT_persist_archive archive = {};
	archive.rtc_offset = rtc_offset;
	archive.rtc_init_check = rtc_init_check;
	archive.sensor_wakeup_period = sensor_wakeup_period;
	archive.nox_sample_period = nox_sample_period;
	archive.nox_sample_counter = nox_sample_counter;
	archive.wakeup_is_from_timer = wakeup_is_from_timer;
	archive.sleep_timestamp = sleep_timestamp;
	archive.pet_mood = pet_mood;
	archive.pet_mask = pet_mask;
	memcpy(archive.pet_name, pet_name, sizeof(archive.pet_name));
	archive.pet_level = pet_level;
	archive.screen_brightness = screen_brightness;
	archive.led_brightness = led_brightness;
	archive.dim_after_seconds = dim_after_seconds;
	archive.sleep_after_seconds = sleep_after_seconds;
	archive.aq_moving_scores = aq_moving_scores;
	archive.aq_moving_score_samples = aq_moving_score_samples;
	archive.aq_moving_score_time = aq_moving_score_time;
	memcpy(archive.aq_weekly_scores, aq_weekly_scores, sizeof(archive.aq_weekly_scores));
	archive.aq_weekly_score_head = aq_weekly_score_head;
	archive.aq_weekly_score_time = aq_weekly_score_time;
	archive.aq_crisis_state = aq_crisis_state;
	archive.pet_timing_state = pet_timing_state;
	archive.persist_flags = persist_flags;
	write(fd, &archive, sizeof(archive));
}

void CAT_read_persist_archive(int fd)
{
	CAT_persist_archive archive = {};
	read(fd, &archive, sizeof(archive));
	rtc_offset = archive.rtc_offset;
	rtc_init_check = archive.rtc_init_check;
	sensor_wakeup_period = archive.sensor_wakeup_period;
	nox_sample_period = archive.nox_sample_period;
	nox_sample_counter = archive.nox_sample_counter;
	wakeup_is_from_timer = archive.wakeup_is_from_timer;
	sleep_timestamp = archive.sleep_timestamp;
	pet_mood = archive.pet_mood;
	pet_mask = archive.pet_mask;
	memcpy(pet_name, archive.pet_name, sizeof(pet_name));
	pet_level = archive.pet_level;
	screen_brightness = archive.screen_brightness;
	led_brightness = archive.led_brightness;
	dim_after_seconds = archive.dim_after_seconds;
	sleep_after_seconds = archive.sleep_after_seconds;
	aq_moving_scores = archive.aq_moving_scores;
	aq_moving_score_samples = archive.aq_moving_score_samples;
	aq_moving_score_time = archive.aq_moving_score_time;
	memcpy(aq_weekly_scores, archive.aq_weekly_scores, sizeof(aq_weekly_scores));
	aq_weekly_score_head = archive.aq_weekly_score_head;
	aq_weekly_score_time = archive.aq_weekly_score_time;
	aq_crisis_state = archive.aq_crisis_state;
	pet_timing_state = archive.pet_timing_state;
	persist_flags = archive.persist_flags;
}

#endif

