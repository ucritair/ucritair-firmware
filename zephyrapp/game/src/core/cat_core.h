#pragma once

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
bool CAT_first_frame_complete();


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

	uint16_t snake_high_score;

	float stat_timer;
	float life_timer;
	float earn_timer;
	uint8_t times_pet;
	float petting_timer;
	uint8_t times_milked;

	int8_t name[32];

	uint32_t theme;

	uint16_t level;
	uint32_t xp;
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

typedef enum CAT_log_flags
{
	CAT_LOG_TEMP_RH_PARTICLES_BIT = 1,
	CAT_LOG_CO2_BIT = 2
} CAT_log_flags;

typedef struct __attribute__((__packed__)) CAT_log_cell {
	uint8_t flags;
	uint8_t unused[3];

	uint64_t timestamp; // timestamp in ????
	int32_t temp_Cx1000; // C * 1000
	uint16_t pressure_hPax10; // hPa * 10

	uint16_t rh_pctx100; // % * 100
	uint16_t co2_ppmx1; // ppm * 1
	uint16_t pm_ugmx100[4]; //PM 1.0, 2.5, 4.0, 10.0 x100
	uint16_t pn_ugmx100[5]; //PN 0.5, 1.0, 2.5, 4.0, 10.0 x100

	uint8_t voc_index, nox_index; //x1

	uint16_t co2_uncomp_ppmx1;

	uint8_t pad[20];
} CAT_log_cell;

int CAT_get_flash_size(int* size);
int CAT_load_flash(uint8_t* target, uint8_t** start, uint8_t** end);
bool CAT_did_post_flash();

void CAT_clear_log();
int CAT_next_log_cell_idx();
void CAT_get_log_cell(int idx, CAT_log_cell* out);
void CAT_populate_log_cell(CAT_log_cell* cell);

bool CAT_log_is_ready();
void CAT_write_log();


////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

#define CAT_CRITICAL_BATTERY_PCT 10

int CAT_get_battery_pct();
bool CAT_is_charging();


////////////////////////////////////////////////////////////////////////////////////////////////////
// AIR QUALITY

typedef struct CAT_AQ_readings {
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
} CAT_AQ_readings;

void CAT_get_AQ_readings(CAT_AQ_readings* readings);


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
