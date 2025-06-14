#include "cat_core.h"

#include "cat_aqi.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// LCD

// THIS IS AN INT BECAUSE IT MAY BE ACCESSED VIA AN INT POINTER AND LEAVING IT AS A UINT8 CAUSES MEMORY OVERWRITE ISSUES
static int lcd_brightness = CAT_LCD_MAX_BRIGHTNESS;

void CAT_LCD_set_brightness(uint8_t percent)
{
	lcd_brightness = clamp(percent, CAT_LCD_MIN_BRIGHTNESS, CAT_LCD_MAX_BRIGHTNESS);
}

uint8_t CAT_LCD_get_brightness()
{
	return lcd_brightness;
}

int* CAT_LCD_brightness_pointer()
{
	return &lcd_brightness;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// EINK SCREEN

static bool eink_needs_update = false;

void CAT_set_eink_update_flag(bool flag)
{
	eink_needs_update = flag;
}

bool CAT_eink_needs_update()
{
	return eink_needs_update;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// SCREEN MANAGEMENT

static CAT_screen_orientation screen_orientation = CAT_SCREEN_ORIENTATION_UP;

void CAT_set_screen_orientation(CAT_screen_orientation orientation)
{
	screen_orientation = orientation;
}

CAT_screen_orientation CAT_get_screen_orientation()
{
	return screen_orientation;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// LEDs

// THIS IS AN INT BECAUSE IT MAY BE ACCESSED VIA AN INT POINTER AND LEAVING IT AS A UINT8 CAUSES MEMORY OVERWRITE ISSUES
static int led_brightness = 100;

void CAT_LED_set_brightness(uint8_t percent)
{
	led_brightness = clamp(percent, 0, 100);
}

uint8_t CAT_LED_get_brightness()
{
	return led_brightness;
}

int* CAT_LED_brightness_pointer()
{
	return &led_brightness;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// AIR QUALITY

CAT_AQ_readings readings = {0};

bool CAT_is_AQ_initialized()
{
	return
	readings.sunrise.ppm_filtered_uncompensated > 0 &&
	(readings.sen5x.temp_degC != 0 ||
	readings.sen5x.humidity_rhpct > 0 ||
	readings.sen5x.pm2_5 > 0);
}

static CAT_temperature_unit temperature_unit = CAT_TEMPERATURE_UNIT_DEGREES_CELSIUS;

CAT_temperature_unit CAT_AQ_get_temperature_unit()
{
	return temperature_unit;
}

void CAT_AQ_set_temperature_unit(CAT_temperature_unit unit)
{
	temperature_unit = unit;
}

float CAT_AQ_map_celsius(float temp)
{
	return
	temperature_unit == CAT_TEMPERATURE_UNIT_DEGREES_CELSIUS ?
	temp :
	temp * (9.0f / 5.0f) + 32;
}

const char* CAT_AQ_get_temperature_unit_string()
{
	return
	temperature_unit == CAT_TEMPERATURE_UNIT_DEGREES_CELSIUS ?
	"\3C" :
	"\3F";
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// SAVE

static int save_flags = 0;

int CAT_export_save_flags()
{
	return save_flags;
}

void CAT_import_save_flags(int flags)
{
	save_flags = flags;
}

void CAT_set_save_flag(CAT_save_flag flag)
{
	save_flags |= (1 << flag);
}

void CAT_clear_save_flag(CAT_save_flag flag)
{
	save_flags &= ~(1 << flag);
}

bool CAT_check_save_flag(CAT_save_flag flag)
{
	return save_flags & (1 << flag);
}

void CAT_clear_save_flags()
{
	save_flags = 0;
}

static int load_flags = 0;

void CAT_set_load_flag(CAT_load_flag flag)
{
	load_flags |= (1 << flag);
}

void CAT_clear_load_flag(CAT_load_flag flag)
{
	load_flags &= ~(1 << flag);
}

bool CAT_check_load_flag(CAT_load_flag flag)
{
	return load_flags & (1 << flag);
}


