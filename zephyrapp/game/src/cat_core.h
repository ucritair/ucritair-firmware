#ifndef CAT_CORE_H
#define CAT_CORE_H

#include <stdint.h>
#include <stdbool.h>

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
// SPEAKER

void CAT_play_tone(float pitch_hz, float time_s);


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

#define PERSISTENCE_PAGE_SIZE 4096

typedef struct CAT_save
{
	struct
	{
		int major;
		int minor;
		int patch;
		int push;
	} version;
} CAT_save;
extern CAT_save save;

void CAT_write_save();
bool CAT_check_save();
void CAT_read_save();


////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

int CAT_get_battery_pct();


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

void CAT_AQI_tick();
float CAT_temp_score();
float CAT_CO2_score();
float CAT_PM_score();
float CAT_VOC_score();
float CAT_NOX_score();


#ifdef CAT_DESKTOP 
#ifdef LOUIS 
#define CAT_BAKED_ASSETS
#endif
#include "cat_desktop.h"
#else
#include "cat_embedded.h"
#define CAT_BAKED_ASSETS
#endif

#endif
