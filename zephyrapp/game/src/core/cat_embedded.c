#include "cat_core.h"

#include <zephyr/kernel.h>

#include "buttons.h"
#include "touch.h"
#include "epaper_driver.h"
#include "epaper_rendering.h"
#include "flash.h"
#include "rtc.h"
#include "sound.h"
#include "rgb_leds.h"
#include "batt.h"
#include "power_control.h"
#include "airquality.h"
#include "lcd_driver.h"
#include "lcd_rendering.h"
#include "imu.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CORE

float delta_t = 0.1;
uint64_t last_uptime = 0;

void CAT_platform_init()
{
	return;
}

void CAT_platform_tick()
{
	uint64_t now = k_uptime_get();
	delta_t = ((float)(now - last_uptime))/1000.;
	last_uptime = now;
}

void CAT_platform_cleanup()
{
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// LCD SCREEN

uint16_t* CAT_LCD_get_framebuffer()
{
	return lcd_framebuffer;
}

void CAT_LCD_post()
{
	return;
}

void CAT_LCD_flip()
{
	return;
}

extern volatile bool write_done;
bool CAT_LCD_is_posted()
{
	return write_done;
}

extern volatile bool first_frame_complete;
bool CAT_first_frame_complete()
{
	return first_frame_complete;
}

int render_cycle = 0;

void CAT_set_render_cycle(int cycle)
{
	render_cycle = cycle;
}

int CAT_get_render_cycle()
{
	return render_cycle;
}

bool CAT_is_first_render_cycle()
{
	return render_cycle == 0;
}

bool CAT_is_last_render_cycle()
{
	return render_cycle == LCD_FRAMEBUFFER_SEGMENTS-1;
}

uint8_t CAT_LCD_get_brightness()
{
	return screen_brightness;
}

void CAT_LCD_set_brightness(uint8_t percent)
{
	screen_brightness = clamp(percent, CAT_LCD_MIN_BRIGHTNESS, CAT_LCD_MAX_BRIGHTNESS);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// EINK SCREEN

void CAT_eink_post(uint8_t* buffer)
{
	return;
}

bool CAT_eink_is_posted()
{
	return true;
}

void CAT_eink_update()
{
	epaper_render_test();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// LEDs

void CAT_set_LEDs(uint8_t r, uint8_t g, uint8_t b)
{
	float power = CAT_LED_get_brightness() / 100.0f;
	set_all_same_color((struct led_rgb){.r=SCALEBYTE(r, power), .g=SCALEBYTE(r, power), .b=SCALEBYTE(r, power)});
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// SPEAKER

void CAT_sound_power(bool value)
{
	soundPower(value);
}

void CAT_play_sound(CAT_sound* sound)
{
	CAT_printf("[CAT_play_sound] Playing %d bytes at %p", sound->size, sound->samples);
	// soundPlay(sound->samples, sound->size, SoundReplaceCurrent);
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

uint64_t CAT_get_slept_s()
{
	return slept_s;
}

uint64_t CAT_get_uptime_ms()
{
	return k_uptime_get();
}

float CAT_get_delta_time_s()
{
	return delta_t;
}

void CAT_get_datetime(CAT_datetime* datetime)
{
	time_t now = get_current_rtc_time();
	struct tm local;
	gmtime_r(&now, &local);
	
	datetime->year = local.tm_year;
	datetime->month = local.tm_mon;
	datetime->day = local.tm_mday;
	datetime->hour = local.tm_hour;
	datetime->minute = local.tm_min;
	datetime->second = local.tm_sec;	
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY

void* CAT_malloc(int bytes)
{
	return NULL;
}

void CAT_free(void* ptr)
{
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// SAVE

// DONT even talk to me I know this is ugly as shit
// I just didn't want tomas to have to worry about the sizes of the saves
_Static_assert(sizeof(CAT_save) < sizeof(epaper_framebuffer));
_Static_assert(sizeof(CAT_save) < ROOM_FOR_TOMAS);

// Call to start saving, then populate the returned CAT_save*
CAT_save* CAT_start_save()
{
	return (CAT_save*) epaper_framebuffer;
}

// then call with the CAT_save* to finish saving
void CAT_finish_save(CAT_save*)
{
	flash_save_tomas_save((uint8_t*) epaper_framebuffer, sizeof(CAT_save));
}

// Call to start loading, then load from the returned CAT_save*
CAT_save* CAT_start_load()
{
	flash_load_tomas_save((uint8_t*) epaper_framebuffer, sizeof(CAT_save));
	return (CAT_save*) epaper_framebuffer;
}
// then call once done loading
void CAT_finish_load()
{
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

int CAT_get_battery_pct()
{
	return ((adc_get_voltage()-3.6)/(4.2-3.6))*100.;
}

bool CAT_is_charging()
{
	return get_is_charging();
}

void CAT_sleep()
{
	power_off(sensor_wakeup_rate*1000, false);
}

void CAT_shutdown()
{
	epaper_render_protected_off();
	power_off(0, true);
}

void CAT_factory_reset()
{
	cat_game_running = 0;
	flash_nuke_tomas_save();
	power_off(0, false);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// AIR QUALITY

CAT_AQ_readings readings;

void CAT_get_AQ_readings()
{
	readings.lps22hh.uptime_last_updated = current_readings.lps22hh.uptime_last_updated;
	readings.lps22hh.temp = current_readings.lps22hh.temp;
	readings.lps22hh.pressure = current_readings.lps22hh.pressure;

	readings.sunrise.uptime_last_updated = current_readings.sunrise.uptime_last_updated;
	readings.sunrise.ppm_filtered_compensated = current_readings.sunrise.ppm_filtered_compensated;
	readings.sunrise.temp = current_readings.sunrise.temp;

	readings.sen5x.uptime_last_updated = current_readings.sen5x.uptime_last_updated;
	readings.sen5x.pm2_5 = current_readings.sen5x.pm2_5;
	readings.sen5x.pm10_0 = current_readings.sen5x.pm10_0;
	readings.sen5x.humidity_rhpct = current_readings.sen5x.humidity_rhpct;

	readings.sen5x.temp_degC = current_readings.sen5x.temp_degC;
	readings.sen5x.voc_index = current_readings.sen5x.voc_index;
	readings.sen5x.nox_index = current_readings.sen5x.nox_index;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// IMU

void CAT_IMU_tick()
{
	imu_update();
}

bool CAT_IMU_is_upside_down()
{
	return imu_recognized_upside_down;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// DEBUG

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(cat_embedded, LOG_LEVEL_DBG);

char debug_print_buffer[512];

void CAT_printf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int added = vsnprintf(debug_print_buffer, sizeof(debug_print_buffer), fmt, args);
	va_end(args);
	LOG_DBG("%s", debug_print_buffer);
}