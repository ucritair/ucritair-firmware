#include "cat_embedded.h"
#include "cat_core.h"

#include <zephyr/kernel.h>

#include "buttons.h"
#include "touch.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
// DEV MODE

float delta_t = 0.1;
uint64_t last_uptime = 0;

void CAT_platform_init()
{
	
}

void CAT_platform_tick()
{
	uint64_t now = k_uptime_get();
	delta_t = ((float)(now - last_uptime))/1000.;
	last_uptime = now;
}

void CAT_AQI_tick()
{
	
}

void CAT_platform_cleanup()
{
	
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// LCD SCREEN

void CAT_LCD_post(uint16_t* buffer)
{
	
}

extern volatile bool write_done;

bool CAT_LCD_is_posted()
{
	return write_done;
}

void CAT_LCD_set_backlight(int percent)
{
	
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// EINK SCREEN

void CAT_ink_post(uint8_t* buffer)
{
	
}

bool CAT_ink_is_posted()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SPEAKER

void CAT_play_tone(float pitch_hz, float time_s)
{
	
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// INPUT

uint16_t CAT_get_buttons()
{
	return get_buttons();
}

void CAT_get_touch(CAT_touch* touch)
{
	touch->x = touch_mapped_x;
	touch->y = touch_mapped_y;
	touch->pressure = touch_pressure;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// TIME

uint64_t CAT_get_time_ms()
{
	return k_uptime_get();
}

float CAT_get_delta_time()
{
	return delta_t;
}

void CAT_get_datetime(CAT_datetime* datetime)
{
	
}

void CAT_set_datetime(CAT_datetime* datetime)
{
	
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY

void* CAT_malloc(int bytes)
{
	
}

void CAT_free(void* ptr)
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// STORAGE

void CAT_write_save(uint8_t* in)
{
	
}

bool CAT_check_save()
{
	return false;
}

void CAT_read_save(uint8_t* out)
{
	
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

int CAT_get_battery_pct()
{
	return 100;
}
