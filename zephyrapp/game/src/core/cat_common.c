#include "cat_core.h"

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

static uint8_t led_brightness = 100;

void CAT_LED_set_brightness(uint8_t percent)
{
	led_brightness = clamp(percent, 0, 100);
}

uint8_t CAT_LED_get_brightness()
{
	return led_brightness;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// AIR QUALITY

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

static uint16_t session_flags = 0;

uint16_t CAT_get_save_flags()
{
	return session_flags;
}

void CAT_set_save_flags(uint16_t flags)
{
	session_flags = flags;
}

void CAT_enable_save_flag(CAT_save_flag flag)
{
	session_flags |= (1 << flag);
}

void CAT_disable_save_flag(CAT_save_flag flag)
{
	session_flags &= ~(1 << flag);
}

bool CAT_is_save_flag_enabled(CAT_save_flag flag)
{
	return (session_flags & (1 << flag)) > 0;
}

static uint16_t load_flags = 0;

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
	return (load_flags & (1 << flag)) > 0;
}
