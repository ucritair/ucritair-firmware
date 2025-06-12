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

uint64_t CAT_get_rtc_now()
{
	return get_current_rtc_time();
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

void CAT_AQ_move_scores()
{
	float CO2 = CAT_co2_score(readings.sunrise.ppm_filtered_uncompensated);
	float NOX = CAT_nox_score(readings.sen5x.nox_index);
	float VOC = CAT_voc_score(readings.sen5x.voc_index);
	float PM2_5 = CAT_pm25_score(readings.sen5x.pm2_5);
	float temp = CAT_temperature_score(CAT_canonical_temp());
	float rh = CAT_humidity_score(readings.sen5x.humidity_rhpct);
	float aggregate = CAT_AQI_aggregate();

#define MOVE_AVERAGE(x) (aq_moving_scores.x = (aq_moving_scores.x + (x - aq_moving_scores.x) / (float) aq_moving_scores.sample_count))
	MOVE_AVERAGE(CO2);
	MOVE_AVERAGE(NOX);
	MOVE_AVERAGE(VOC);
	MOVE_AVERAGE(PM2_5);
	MOVE_AVERAGE(temp);
	MOVE_AVERAGE(rh);
	MOVE_AVERAGE(aggregate);

	aq_moving_scores.sample_count += 1;

	CAT_printf("[MOVING SCORES]\n");
	CAT_printf("CO2: %f NOX: %f\n", aq_moving_scores.CO2, aq_moving_scores.NOX);
	CAT_printf("VOC: %f PM2_5: %f\n", aq_moving_scores.VOC, aq_moving_scores.PM2_5);
	CAT_printf("temp: %f\n", aq_moving_scores.temp);
	CAT_printf("RH: %f\n", aq_moving_scores.rh);
	CAT_printf("aggregate: %f count: %d\n", aq_moving_scores.aggregate, aq_moving_scores.sample_count);		
}

void CAT_AQ_store_moving_scores(CAT_AQ_score_block* block)
{
#define STORE_SCORE(x, X) (block->x = round((aq_moving_scores.x / X) * 255.0f))
	STORE_SCORE(CO2, 5.0f);
	STORE_SCORE(NOX, 5.0f);
	STORE_SCORE(VOC, 5.0f);
	STORE_SCORE(PM2_5, 5.0f);
	STORE_SCORE(temp, 5.0f);
	STORE_SCORE(rh, 5.0f);
	STORE_SCORE(aggregate, 100.0f);

	CAT_printf("[BUFFERING MOVING SCORES]\n");
}

int CAT_AQ_get_stored_scores_count()
{
	return aq_score_count;
}

void CAT_AQ_read_stored_scores(int idx, CAT_AQ_score_block* out)
{
	if(idx < 0 || idx >= aq_score_count)
		return;
	if(aq_score_count >= 7)
		idx = (aq_score_head + idx) % 7;
	memcpy(out, &aq_score_buffer[idx], sizeof(CAT_AQ_score_block));
}


void CAT_AQ_clear_stored_scores()
{
	aq_score_count = 0;
	aq_score_head = 0;
	return;
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
#include <zephyr/logging/log_ctrl.h>
LOG_MODULE_REGISTER(cat_embedded, LOG_LEVEL_DBG);

char debug_print_buffer[512];

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