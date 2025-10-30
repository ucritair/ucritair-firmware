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

	imu_update();
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
	set_all_same_color((struct led_rgb){.r=power * r, .g=power * g, .b=power * b});
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// SPEAKER

// TEMPORARILY DISABLED DUE TO CRASH

void CAT_sound_power(bool value)
{
	// soundPower(value);
}

void CAT_play_sound(CAT_sound* sound)
{
	// soundPlay(sound->samples, sound->size, SoundReplaceCurrent);
}

void CAT_beep()
{
	test_speaker();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// INPUT

uint16_t CAT_get_buttons()
{
	return current_buttons;
}

void CAT_get_touch(CAT_touch* touch)
{
	touch->x = CAT_clamp(0, touch_mapped_x, CAT_LCD_SCREEN_W-1);
	touch->y = CAT_clamp(0, touch_mapped_y, CAT_LCD_SCREEN_H-1);
	touch->pressure = CAT_clamp(0, touch_pressure, 3999);
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

uint64_t CAT_get_RTC_offset()
{
	return rtc_offset / 8;
}

uint64_t CAT_get_RTC_now()
{
	return get_current_rtc_time();
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
	flash_save_tomas_save((uint8_t*) epaper_framebuffer, CAT_max(sizeof(CAT_save), sizeof(CAT_save_legacy)));
}

// Call to start loading, then load from the returned CAT_save*
CAT_save* CAT_start_load()
{
	// NEED ENOUGH SPACE TO LOAD AND MIGRATE A LEGACY SAVE
	flash_load_tomas_save((uint8_t*) epaper_framebuffer, CAT_max(sizeof(CAT_save), sizeof(CAT_save_legacy)));
	return (CAT_save*) epaper_framebuffer;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// LOGS

void CAT_read_log_cell_at_idx(int idx, CAT_log_cell* out)
{
	flash_get_cell_by_nr(idx, out);
}

int CAT_read_log_cell_before_time(int base_idx, uint64_t time, CAT_log_cell* out)
{
	return flash_get_first_cell_before_time(base_idx, time, out);
}

int CAT_read_log_cell_after_time(int base_idx, uint64_t time, CAT_log_cell* out)
{
	return flash_get_first_cell_after_time(base_idx, time, out);
}

int CAT_read_first_calendar_cell(CAT_log_cell* cell)
{
	return flash_get_first_calendar_cell(cell);
}

int CAT_get_log_cell_count()
{
	return next_log_cell_nr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

int CAT_get_battery_pct()
{
	return get_battery_pct();
}

bool CAT_is_charging()
{
	return get_is_charging();
}

bool CAT_is_on()
{
	return true;
}

void CAT_sleep()
{
	power_off(sensor_wakeup_period*1000, false);
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
// IMU

void CAT_IMU_get_raw(CAT_IMU_values* out)
{
	memcpy(out, &imu_raw, sizeof(imu_raw));
}

void CAT_IMU_get_normalized(CAT_IMU_values* out)
{
	memcpy(out, &imu_normalized, sizeof(imu_normalized));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// DEBUG

#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
LOG_MODULE_REGISTER(cat_embedded, LOG_LEVEL_DBG);

static char debug_print_buffer[128];

void CAT_printf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int printed = vsnprintf(debug_print_buffer, sizeof(debug_print_buffer), fmt, args);
	va_end(args);
	if(debug_print_buffer[printed-1] == '\n')
		debug_print_buffer[printed-1] = '\0';
	LOG_DBG("%s", debug_print_buffer);
}