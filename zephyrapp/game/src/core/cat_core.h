#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cat_math.h"
#include <stdlib.h>
#include <string.h>
#include "cat_time.h"
#include "cat_leaderboard.h"

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

#define CAT_EINK_SCREEN_W 248
#define CAT_EINK_SCREEN_H 128

#define CAT_TILE_SIZE 16
#define CAT_GLYPH_WIDTH 8
#define CAT_GLYPH_HEIGHT 12
#define CAT_LEADING 2

#define CAT_SCREEN_GRID_W (CAT_LCD_SCREEN_W / CAT_TILE_SIZE)
#define CAT_SCREEN_GRID_H (CAT_LCD_SCREEN_H / CAT_TILE_SIZE)

#define CAT_DEFAULT_PET_NAME "Aura"


////////////////////////////////////////////////////////////////////////////////////////////////////
// PLATFORM

void CAT_platform_init();
void CAT_platform_tick();
void CAT_platform_cleanup();
void CAT_platform_capture_frame();


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

#define CAT_EINK_PIXEL_COUNT (CAT_EINK_SCREEN_W*CAT_EINK_SCREEN_H)
#define CAT_EINK_FRAMEBUFFER_SIZE (CAT_EINK_PIXEL_COUNT/8)

#define EINK_UPDATE_PERIOD (CAT_MINUTE_SECONDS * 2)

uint8_t* CAT_eink_get_framebuffer();

bool CAT_eink_is_boot_update();
void CAT_set_eink_update_flag(bool flag);
bool CAT_poll_eink_update_flag();

void CAT_eink_clear();
void CAT_eink_draw_default();
void CAT_eink_draw_power_off();
void CAT_eink_draw_research();
void CAT_eink_update(bool force_full_write);

bool CAT_eink_should_update();
void CAT_eink_execute_update();

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
void CAT_set_date(CAT_datetime date);


////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY

void* CAT_malloc(int bytes);
void CAT_free(void* ptr);


////////////////////////////////////////////////////////////////////////////////////////////////////
// LOGS

#ifdef CAT_EMBEDDED
_Static_assert(sizeof(float) == sizeof(uint32_t));
#endif

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
	
	CAT_stroop_data stroop_data;

	uint8_t pad[11];
} CAT_log_cell;

#ifdef CAT_EMBEDDED
_Static_assert(sizeof(CAT_log_cell) == 64);
#endif

typedef enum
{
	CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES = (1 << 0),
	CAT_LOG_CELL_FLAG_HAS_CO2 = (1 << 1),
	CAT_LOG_CELL_FLAG_HAS_COG_PERF = (1 << 2)
} CAT_log_cell_flag;

void CAT_read_log_cell_at_idx(int idx, CAT_log_cell* out);
int CAT_read_log_cell_before_time(int bookmark, uint64_t time, CAT_log_cell* out);
int CAT_read_log_cell_after_time(int bookmark, uint64_t time, CAT_log_cell* out);
int CAT_read_first_calendar_cell(CAT_log_cell* cell);
int CAT_get_log_cell_count();
bool CAT_logs_initialized();
void CAT_force_log_cell_write();
void CAT_erase_log_cells();


////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

#define CAT_CRITICAL_BATTERY_PCT 20
#define CAT_SAFE_BATTERY_PCT 50

int CAT_get_battery_pct();
bool CAT_is_charging();
bool CAT_is_on();

void CAT_msleep(int ms);
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

bool CAT_AQ_sensors_initialized();
bool CAT_AQ_NOX_VOC_initialized();


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
// COMMS

static inline uint32_t CAT_bonus_get()
{
	return 0;
}

static inline void CAT_bonus_set(uint32_t value)
{
	;
}