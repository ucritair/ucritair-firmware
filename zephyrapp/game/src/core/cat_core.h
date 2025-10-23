#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cat_math.h"
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// MISCELLANY

#define CAT_MINUTE_SECONDS 60
#define CAT_HOUR_SECONDS (60 * CAT_MINUTE_SECONDS)
#define CAT_DAY_SECONDS (24 * CAT_HOUR_SECONDS)
#define CAT_MONTH_SECONDS (30 * CAT_DAY_SECONDS)
#define CAT_YEAR_SECONDS (365 * CAT_DAY_SECONDS + 6 * CAT_HOUR_SECONDS)
#define CAT_DAY_ZERO 60904915200 // (2000/1/1 00:00:00)

#define CAT_LCD_SCREEN_W 240
#define CAT_LCD_SCREEN_H 320

#define EINK_SCREEN_W 212
#define EINK_SCREEN_H 104

#define CAT_TILE_SIZE 16
#define CAT_GLYPH_WIDTH 8
#define CAT_GLYPH_HEIGHT 12
#define CAT_LEADING 2

#define CAT_SCREEN_GRID_W (CAT_LCD_SCREEN_W / CAT_TILE_SIZE)
#define CAT_SCREEN_GRID_H (CAT_LCD_SCREEN_H / CAT_TILE_SIZE)

#define CAT_DEFAULT_PET_NAME "Aura"

#define CAT_TEXT_INPUT_MAX_LENGTH 24


////////////////////////////////////////////////////////////////////////////////////////////////////
// PLATFORM

void CAT_platform_init();
void CAT_platform_tick();
void CAT_platform_cleanup();


////////////////////////////////////////////////////////////////////////////////////////////////////
// LCD SCREEN

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


////////////////////////////////////////////////////////////////////////////////////////////////////
// EINK SCREEN

void CAT_eink_post(uint8_t* buffer);
bool CAT_eink_is_posted();

void CAT_set_eink_update_flag(bool flag);
bool CAT_poll_eink_update_flag();
void CAT_eink_update();

bool CAT_eink_should_update();
void CAT_eink_execute_update();
void CAT_eink_low_power();

////////////////////////////////////////////////////////////////////////////////////////////////////
// SCREEN MANAGEMENT

typedef enum
{
	CAT_SCREEN_ORIENTATION_UP,
	CAT_SCREEN_ORIENTATION_DOWN
} CAT_screen_orientation;

void CAT_set_screen_orientation(int orientation);
CAT_screen_orientation CAT_get_screen_orientation();

void CAT_poll_screen_flip();
bool CAT_should_flip_screen();
void CAT_flip_screen();

void CAT_orientation_tick();


////////////////////////////////////////////////////////////////////////////////////////////////////
// LEDs

void CAT_set_LEDs(uint8_t r, uint8_t g, uint8_t b);


////////////////////////////////////////////////////////////////////////////////////////////////////
// SPEAKER

typedef struct CAT_sound
{
	uint8_t* samples;
	size_t size;
} CAT_sound;

void CAT_sound_power(bool value);
void CAT_play_sound(CAT_sound* sound);
void CAT_beep();


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
uint64_t CAT_get_RTC_offset();
uint64_t CAT_get_RTC_now();


////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY

void* CAT_malloc(int bytes);
void CAT_free(void* ptr);


////////////////////////////////////////////////////////////////////////////////////////////////////
// SAVE

#define CAT_SAVE_MAGIC 0xaabbccde

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
} CAT_save_legacy;

typedef enum
{
	CAT_SAVE_CONFIG_FLAG_NONE = 0,
	CAT_SAVE_CONFIG_FLAG_DEVELOPER = (1 << 0),
	CAT_SAVE_CONFIG_FLAG_KALI_YUGA = (1 << 1)
} CAT_save_config_flag;

typedef enum
{
	CAT_SAVE_SECTOR_PET,
	CAT_SAVE_SECTOR_INVENTORY,
	CAT_SAVE_SECTOR_DECO,
	CAT_SAVE_SECTOR_HIGHSCORES,
	CAT_SAVE_SECTOR_CONFIG,
	CAT_SAVE_SECTOR_FOOTER,
} CAT_save_sector;

typedef struct __attribute__((__packed__))
{
	uint8_t label;
	uint16_t size;
} CAT_save_sector_header;

typedef struct __attribute__((__packed__))
{
	// HEADER : MAGIC NUMBER
	uint32_t magic_number;

	// HEADER : VERSION
	uint8_t version_major;
	uint8_t version_minor;
	uint8_t version_patch;
	uint8_t version_push;

	// SECTOR : PET
	struct __attribute__((__packed__))
	{
		CAT_save_sector_header header;
		char name[24];
		uint8_t lifespan;
		uint8_t lifetime;
		uint16_t incarnations;
		uint8_t level;
		uint32_t xp;
		uint8_t vigour;
		uint8_t focus;
		uint8_t spirit;
	} pet;

	// SECTOR : INVENTORY
	struct __attribute__((__packed__))
	{
		CAT_save_sector_header header;
		uint16_t counts[256];
	} inventory;

	// SECTOR : DECO
	struct __attribute__((__packed__))
	{
		CAT_save_sector_header header;
		uint8_t props[150];
		uint8_t positions[150*2];
		uint8_t overrides[150];
		uint8_t children[150];
	} deco;
	
	// SECTOR : HIGHSCORES
	struct __attribute__((__packed__))
	{
		CAT_save_sector_header header;
		uint16_t snake;
		uint16_t mine;
		uint16_t foursquares;
	} highscores;

	// SECTOR : CONFIG
	struct __attribute__((__packed__))
	{
		CAT_save_sector_header header;
		uint64_t flags;
		uint8_t theme;
	} config;

	// FOOTER
	CAT_save_sector_header footer;
} CAT_save;

typedef enum
{
	CAT_SAVE_ERROR_NONE,
	CAT_SAVE_ERROR_MAGIC,
	CAT_SAVE_ERROR_SECTOR_CORRUPT,
	CAT_SAVE_ERROR_SECTOR_MISSING
} CAT_save_error;

void CAT_initialize_save(CAT_save* save);
void CAT_migrate_legacy_save(void* save);
void CAT_extend_save(CAT_save* save);
CAT_save_error CAT_verify_save_structure(CAT_save* save);

CAT_save* CAT_start_save();
void CAT_finish_save(CAT_save* save);
CAT_save* CAT_start_load();

uint64_t CAT_get_config_flags();
void CAT_set_config_flags(uint64_t flags);

void CAT_raise_config_flags(uint64_t flags);
void CAT_lower_config_flags(uint64_t flags);
bool CAT_check_config_flags(uint64_t flags);

typedef enum
{
	CAT_LOAD_FLAG_NONE = 0,
	CAT_LOAD_FLAG_DIRTY = 1,
	CAT_LOAD_FLAG_DEFAULT = 2,
	CAT_LOAD_FLAG_TURNKEY = 4
} CAT_load_flag;

void CAT_set_load_flags(uint64_t flags);
void CAT_unset_load_flags(uint64_t flags);
bool CAT_check_load_flags(uint64_t flags);


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
int CAT_read_log_cell_before_time(int bookmark, uint64_t time, CAT_log_cell* out);
int CAT_read_log_cell_after_time(int bookmark, uint64_t time, CAT_log_cell* out);
int CAT_read_first_calendar_cell(CAT_log_cell* cell);
int CAT_get_log_cell_count();


////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

#define CAT_CRITICAL_BATTERY_PCT 20
#define CAT_SAFE_BATTERY_PCT 50

int CAT_get_battery_pct();
bool CAT_is_charging();
bool CAT_is_on();

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

bool CAT_AQ_logs_initialized();
bool CAT_AQ_sensors_initialized();
bool CAT_AQ_NOX_VOC_initialized();

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
	uint16_t CO2; // ppm x1
	uint8_t VOC; // index x1
	uint8_t NOX; // index x1
	uint16_t PM2_5; // ug/m3 x100
	int32_t temp; // degC x1000
	uint16_t rh; // % x100
	uint8_t aggregate; // score x1
} CAT_AQ_score_block;

bool CAT_AQ_moving_scores_initialized();
void CAT_AQ_update_moving_scores();

bool CAT_AQ_weekly_scores_initialized();
void CAT_AQ_push_weekly_scores(CAT_AQ_score_block* in);
CAT_AQ_score_block* CAT_AQ_get_weekly_scores(int idx);


////////////////////////////////////////////////////////////////////////////////////////////////////
// IMU

typedef union 
{
	struct
	{
		float x;
		float y;
		float z;
	};

	float data[3];
} CAT_IMU_values;

void CAT_IMU_get_raw(CAT_IMU_values* out);
void CAT_IMU_get_normalized(CAT_IMU_values* out);
bool CAT_IMU_is_upside_down();


////////////////////////////////////////////////////////////////////////////////////////////////////
// DEBUG

void CAT_printf(const char* fmt, ...);
int CAT_get_debug_number();
void CAT_set_debug_number(int x);


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