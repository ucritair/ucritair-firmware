#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cat_math.h"
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// PLATFORM

void CAT_platform_init();
void CAT_platform_tick();
void CAT_platform_cleanup();


////////////////////////////////////////////////////////////////////////////////////////////////////
// LCD SCREEN

#define CAT_LCD_SCREEN_W 240
#define CAT_LCD_SCREEN_H 320

#define CAT_LCD_FRAMEBUFFER_SEGMENTS 2
#define CAT_LCD_FRAMEBUFFER_W CAT_LCD_SCREEN_W
#define CAT_LCD_FRAMEBUFFER_H (CAT_LCD_SCREEN_H / CAT_LCD_FRAMEBUFFER_SEGMENTS)
#define CAT_LCD_FRAMEBUFFER_PIXELS (CAT_LCD_FRAMEBUFFER_W * CAT_LCD_FRAMEBUFFER_H)

#define CAT_LCD_MIN_BRIGHTNESS 10
#define CAT_LCD_MAX_BRIGHTNESS 75

uint16_t* CAT_LCD_get_framebuffer();
void CAT_LCD_post();
bool CAT_LCD_is_posted();
void CAT_LCD_flip();

void CAT_set_render_cycle(int cycle);
int CAT_get_render_cycle();
bool CAT_is_first_render_cycle();
bool CAT_is_last_render_cycle();

uint8_t CAT_LCD_get_brightness();
void CAT_LCD_set_brightness(uint8_t percent);
int* CAT_LCD_brightness_pointer();


////////////////////////////////////////////////////////////////////////////////////////////////////
// EINK SCREEN

#define EINK_SCREEN_W 212
#define EINK_SCREEN_H 104

void CAT_eink_post(uint8_t* buffer);
bool CAT_eink_is_posted();
void CAT_eink_update();

void CAT_set_eink_update_flag(bool flag);
bool CAT_eink_needs_update();


////////////////////////////////////////////////////////////////////////////////////////////////////
// SCREEN MANAGEMENT

typedef enum
{
	CAT_SCREEN_ORIENTATION_UP,
	CAT_SCREEN_ORIENTATION_DOWN
} CAT_screen_orientation;

void CAT_set_screen_orientation(CAT_screen_orientation orientation);
CAT_screen_orientation CAT_get_screen_orientation();


////////////////////////////////////////////////////////////////////////////////////////////////////
// LEDs

void CAT_set_LEDs(uint8_t r, uint8_t g, uint8_t b);
void CAT_LED_set_brightness(uint8_t percent);
uint8_t CAT_LED_get_brightness();
int* CAT_LED_brightness_pointer();


////////////////////////////////////////////////////////////////////////////////////////////////////
// SPEAKER

typedef struct CAT_sound
{
	uint8_t* samples;
	size_t size;
} CAT_sound;

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

uint64_t CAT_get_slept_s();
uint64_t CAT_get_uptime_ms();
float CAT_get_delta_time_s();
uint64_t CAT_get_rtc_now();

typedef struct CAT_datetime
{
    int year, month, day, hour, minute, second;
} CAT_datetime;

void CAT_get_datetime(CAT_datetime* datetime);


////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY

void* CAT_malloc(int bytes);
void CAT_free(void* ptr);


////////////////////////////////////////////////////////////////////////////////////////////////////
// SAVE

#define CAT_SAVE_MAGIC 0xaabbccde

typedef enum
{
	CAT_SAVE_FLAG_NONE = 0,
	CAT_SAVE_FLAG_DEVELOPER_MODE = 1,
	CAT_SAVE_FLAG_AQ_FIRST = 2
} CAT_save_flag;

typedef enum
{
	CAT_LOAD_FLAG_NONE = 0,
	CAT_LOAD_FLAG_DIRTY = 1,
	CAT_LOAD_FLAG_RESET = 2,
	CAT_LOAD_FLAG_OVERRIDE = 4
} CAT_load_flag;

typedef struct __attribute__((__packed__))
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

	char name[32];

	uint32_t theme;

	uint16_t level;
	uint32_t xp;

	uint8_t lcd_brightness;
	uint8_t led_brightness;

	uint8_t temperature_unit;

	int save_flags;
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

int CAT_export_save_flags();
void CAT_import_save_flags(int flags);
void CAT_set_save_flag(CAT_save_flag flag);
void CAT_clear_save_flag(CAT_save_flag flag);
bool CAT_check_save_flag(CAT_save_flag flag);
void CAT_clear_save_flags();

void CAT_set_load_flag(CAT_load_flag flag);
void CAT_clear_load_flag(CAT_load_flag flag);
bool CAT_check_load_flag(CAT_load_flag flag);


////////////////////////////////////////////////////////////////////////////////////////////////////
// LOGS

typedef struct __attribute__((__packed__))
{
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

typedef enum
{
	CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES = 1,
	CAT_LOG_CELL_FLAG_HAS_CO2 = 2
} CAT_log_cell_flag;

void CAT_read_log_cell_at_idx(int idx, CAT_log_cell* out);
int CAT_read_log_cell_before_time(int base_idx, uint64_t time, CAT_log_cell* out);


////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

#define CAT_CRITICAL_BATTERY_PCT 20

int CAT_get_battery_pct();
bool CAT_is_charging();

void CAT_sleep();
void CAT_shutdown();
void CAT_factory_reset();


////////////////////////////////////////////////////////////////////////////////////////////////////
// AIR QUALITY

typedef struct
{
	struct {
		uint64_t uptime_last_updated;
		float temp, pressure;
	} lps22hh;
	
	struct {
		uint64_t uptime_last_updated;
		float ppm_filtered_compensated;
		float ppm_filtered_uncompensated;
		float temp;
	} sunrise;
	
	struct {
		uint64_t uptime_last_updated;
		float pm1_0, pm2_5, pm4_0, pm10_0;
		float nc0_5, nc1_0, nc2_5, nc4_0, nc10_0;
		float typ_particle_sz_um;
		float humidity_rhpct, temp_degC, voc_index, nox_index;
	} sen5x;
} CAT_AQ_readings;
extern CAT_AQ_readings readings;

bool CAT_is_AQ_initialized();

typedef enum
{
	CAT_TEMPERATURE_UNIT_DEGREES_CELSIUS,
	CAT_TEMPERATURE_UNIT_DEGREES_FAHRENHEIT
} CAT_temperature_unit;

CAT_temperature_unit CAT_AQ_get_temperature_unit();
void CAT_AQ_set_temperature_unit(CAT_temperature_unit unit);
float CAT_AQ_map_celsius(float temp);
const char* CAT_AQ_get_temperature_unit_string();

typedef struct __attribute__((__packed__))
{
	uint8_t CO2;
	uint8_t VOC;
	uint8_t NOX;
	uint8_t PM2_5;
	uint8_t temp;
	uint8_t rh;
	uint8_t aggregate;
} CAT_AQ_score_block;

typedef struct __attribute__((__packed__))
{
	float CO2;
	float VOC;
	float NOX;
	float PM2_5;
	float temp;
	float rh;
	float aggregate;
	uint64_t sample_count;
} CAT_AQ_moving_scores;

void CAT_AQ_move_scores();
void CAT_AQ_store_moving_scores(CAT_AQ_score_block* block);

int CAT_AQ_get_stored_scores_count();
void CAT_AQ_read_stored_scores(int idx, CAT_AQ_score_block* out);

void CAT_AQ_clear_stored_scores();


////////////////////////////////////////////////////////////////////////////////////////////////////
// IMU

void CAT_IMU_tick();
bool CAT_IMU_is_upside_down();


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
