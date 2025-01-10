#pragma once

#ifdef CAT_DESKTOP
#include "cat_desktop.h"
#else
#include "cat_embedded.h"
#endif

#include <stdint.h>
#include <stdbool.h>
#include "cat_math.h"
#include "sound_assets.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// PLATFORM

void CAT_platform_init();
void CAT_platform_tick();
void CAT_platform_cleanup();


////////////////////////////////////////////////////////////////////////////////////////////////////
// LCD SCREEN

#define LCD_SCREEN_W 240
#define LCD_SCREEN_H 320

void CAT_LCD_post(uint16_t* buffer);
bool CAT_LCD_is_posted();
void CAT_LCD_set_backlight(int percent);


////////////////////////////////////////////////////////////////////////////////////////////////////
// EINK SCREEN

#define EINK_SCREEN_W 212
#define EINK_SCREEN_H 104

void CAT_eink_post(uint8_t* buffer);
bool CAT_eink_is_posted();


////////////////////////////////////////////////////////////////////////////////////////////////////
// LEDs

void CAT_set_LEDs(uint8_t r, uint8_t g, uint8_t b);


////////////////////////////////////////////////////////////////////////////////////////////////////
// SPEAKER

void CAT_sound_power(bool value);
void CAT_play_sound(CAT_sound* sound);


////////////////////////////////////////////////////////////////////////////////////////////////////
// INPUT

uint16_t CAT_get_buttons();

typedef struct CAT_touch
{
    uint16_t x;
    uint16_t y;
    uint16_t pressure;
} CAT_touch;

void CAT_get_touch(CAT_touch* touch);


////////////////////////////////////////////////////////////////////////////////////////////////////
// TIME

uint64_t CAT_get_time_ms();
float CAT_get_delta_time();

typedef struct CAT_datetime
{
    int year, month, day, hour, minute, second;
} CAT_datetime;

void CAT_get_datetime(CAT_datetime* datetime);
void CAT_set_datetime(CAT_datetime* datetime);


////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY

void* CAT_malloc(int bytes);
void CAT_free(void* ptr);


////////////////////////////////////////////////////////////////////////////////////////////////////
// STORAGE

#define CAT_SAVE_MAGIC 0xaabbccde

typedef struct __attribute__((__packed__)) CAT_save
{
	uint32_t magic_number;

	uint8_t version_major;
	uint8_t version_minor;
	uint8_t version_patch;
	uint8_t version_push;

	uint8_t vigour;
	uint8_t focus;
	uint8_t spirit;
	uint32_t lifetime;

	uint8_t prop_ids[150];
	CAT_ivec2 prop_places[150];
	uint8_t prop_overrides[150];
	int16_t prop_children[150];
	uint8_t prop_count;

	uint8_t bag_ids[128];
	uint8_t bag_counts[128];
	uint8_t bag_length;
	uint32_t coins;

	int snake_high_score;

	float stat_timer;
	float life_timer;
	float earn_timer;
	int times_pet;
	float petting_timer;
	int times_milked;

	char name[32];

	unsigned long int theme;

	int level;
	int xp;
} CAT_save;

// Call to start saving, then populate the returned CAT_save*
CAT_save* CAT_start_save();
// then call with the CAT_save* to finish saving
void CAT_finish_save(CAT_save*);

// Call to start loading, then load from the returned CAT_save*
CAT_save* CAT_start_load();
// then call once done loading
void CAT_finish_load();

static inline bool CAT_check_save(CAT_save* save)
{
	return save->magic_number == CAT_SAVE_MAGIC;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

#define CAT_CRITICAL_BATTERY_PCT 10

int CAT_get_battery_pct();
bool CAT_is_charging();


////////////////////////////////////////////////////////////////////////////////////////////////////
// AIR QUALITY

#ifdef CAT_DESKTOP
typedef struct CAT_AQI {
	struct {
		uint64_t uptime_last_updated;
		float temp, pressure;
	} lps22hh;
	
	struct {
		uint64_t uptime_last_updated;
		float ppm_filtered_compensated;
		float temp;
	} sunrise;
	
	struct {
		uint64_t uptime_last_updated;
		float pm1_0, pm2_5, pm4_0, pm10_0;
		float humidity_rhpct, temp_degC, voc_index, nox_index;
	} sen5x;
} CAT_AQI;
extern CAT_AQI aqi;
#else
#include "airquality.h"
#define CAT_AQI struct current_readings
#define aqi current_readings
#endif

float CAT_co2_score(float co2);
float CAT_voc_score(float voc);
float CAT_nox_score(float nox);
float CAT_pm25_score(float pm25);
float CAT_mean_temp();
float CAT_temperature_score(float temp);
float CAT_humidity_score(float rh);
float CAT_iaq_score(float co2, float voc, float nox, float pm25, float temp, float rh);
void CAT_AQI_quantize(int* temp_idx, int* co2_idx, int* pm_idx, int* voc_idx, int* nox_idx);
float CAT_AQI_aggregate();


////////////////////////////////////////////////////////////////////////////////////////////////////
// DEBUG

void CAT_printf(const char* fmt, ...);


////////////////////////////////////////////////////////////////////////////////////////////////////
// BONUS

static inline uint32_t CAT_bonus_get()
{
	return 0;
}

static inline void CAT_bonus_set(uint32_t value)
{
	;
}