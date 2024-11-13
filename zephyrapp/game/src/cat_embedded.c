#include "cat_embedded.h"
#include "cat_core.h"

#include <zephyr/kernel.h>

#include "buttons.h"
#include "touch.h"
#include "epaper_driver.h"
#include "epaper_rendering.h"
#include "flash.h"

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
	return current_buttons;
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


// DONT even talk to me I know this is ugly as shit
// I just didn't want tomas to have to worry about the sizes of the saves
_Static_assert(sizeof(CAT_save) < sizeof(epaper_framebuffer));
_Static_assert(sizeof(CAT_save) < ROOM_FOR_TOMAS);

// Call to start saving, then populate the returned CAT_save*
CAT_save* CAT_start_save()
{
	return (CAT_save*)epaper_framebuffer;
}
// then call with the CAT_save* to finish saving
void CAT_finish_save(CAT_save*)
{
	flash_save_tomas_save((uint8_t*)epaper_framebuffer, sizeof(CAT_save));
}

// Call to start loading, then load from the returned CAT_save*
CAT_save* CAT_start_load()
{
	flash_load_tomas_save((uint8_t*)epaper_framebuffer, sizeof(CAT_save));
	return (CAT_save*)epaper_framebuffer;
}
// then call once done loading
void CAT_finish_load()
{
	// no-op
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

int CAT_get_battery_pct()
{
	return 100;
}

#include "rgb_leds.h"
void CAT_set_LEDs(uint8_t r, uint8_t g, uint8_t b)
{
#define SCALE 4
	set_all_same_color((struct led_rgb){.r=r>>SCALE, .g=g>>SCALE, .b=b>>SCALE});
}