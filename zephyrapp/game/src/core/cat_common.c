#include "cat_core.h"

#include <math.h>
#include "cat_structures.h"
#include "cat_version.h"
#include <stdarg.h>
#include <stdio.h>
#include "item_assets.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_persist.h"
#include "cat_save.h"
#include "tinysprite_assets.h"
#include "cat_spriter.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
// EINK SCREEN

static bool eink_boot_update = true;
static bool eink_dirty = false;
static uint64_t eink_update_timestamp = 0;
static bool eink_full_update = true;

bool CAT_eink_is_boot_update()
{
	return eink_boot_update;
}

void CAT_set_eink_update_flag(bool flag)
{
	eink_dirty = flag;
}

bool CAT_poll_eink_update_flag()
{
	return eink_dirty;
}

void CAT_eink_clear()
{
	memset(CAT_eink_get_framebuffer(), 0, CAT_EINK_FRAMEBUFFER_SIZE);
}

void CAT_eink_draw_default()
{
	CAT_printf("That one!\n");

	CAT_eink_clear();

	char buf[256] = {0};
	#define fwrite_str(x, y, s, str, ...) snprintf(buf, sizeof(buf), str, ##__VA_ARGS__); CAT_eink_draw_string(x, y, s, buf);

	CAT_datetime now;
	CAT_get_datetime(&now);

	CAT_tinysprite* selected_unicorn = &tnyspr_unicorn_default;
	CAT_tinysprite* selected_cloud = &tnyspr_cloud_default;

	if(persist_flags & CAT_PERSIST_CONFIG_FLAG_BATTERY_ALERT)
	{	
		selected_unicorn = &tnyspr_unicorn_low_battery;
	}
	else if (pet_mask)
	{
		selected_unicorn = &tnyspr_unicorn_mask;
	}
	else
	{
		if (pet_mood == 2)
		{
			selected_unicorn = &tnyspr_unicorn_happy;
		}
		else if (pet_mood == 0)
		{
			selected_unicorn = &tnyspr_unicorn_sad;
		}
	}

	float score = CAT_AQ_aggregate_score();

	if (score < 25)
	{
		selected_cloud = &tnyspr_cloud_smoke;
	}
	else if (score < 50)
	{
		selected_cloud = &tnyspr_cloud_sad;
	}
	else if (score > 75)
	{
		selected_cloud = &tnyspr_cloud_happy;
	}

	CAT_eink_draw_sprite(0, 0, selected_unicorn);
	CAT_eink_draw_sprite(0, selected_unicorn->height, selected_cloud);

	if(CAT_AQ_sensors_initialized())
	{
		fwrite_str(128, 20, 2, "%d", (int) readings.sunrise.ppm_filtered_compensated);
		fwrite_str(CAT_EINK_SCREEN_W-(8*3), 20, 1, "ppm\nCO2");
		fwrite_str(128, 40, 2, CAT_FLOAT_FMT, CAT_FMT_FLOAT(readings.sen5x.pm2_5));
		fwrite_str(CAT_EINK_SCREEN_W-(8*5), 40, 1, "ug/m3\nPM2.5");

		if (CAT_AQ_NOX_VOC_initialized()) // NEEDS THE BRACKETS ?!
		{
			fwrite_str(128, 60, 1, "%d NOX / %d VOC", (int) readings.sen5x.nox_index, (int) readings.sen5x.voc_index);
		}
		
		float deg_c = readings.sen5x.temp_degC;
		float deg_mapped = CAT_AQ_map_celsius(deg_c);
		fwrite_str(128, 70, 1, "%d %s / %d%% RH", (int) deg_mapped, CAT_AQ_get_temperature_unit_string(), (int) readings.sen5x.humidity_rhpct);
		fwrite_str(128, 80, 1, "%d%% rebreathed", (int) ((((readings.sunrise.ppm_filtered_compensated)-420.)/38000.)*100.));
		fwrite_str(128, 90, 1, "uCritAQI %d%%", (int) score);
		fwrite_str(128, 100, 1, "at %2d:%02d:%02d", now.hour, now.minute, now.second);
		fwrite_str(128, 110, 1, "%d%% battery", CAT_get_battery_pct());
	}
	else
	{
		fwrite_str(128, 64, 1, "Waiting for\nsensors...");
	}

	fwrite_str(0, CAT_EINK_SCREEN_H-10, 1, " %s LV%d", pet_name, pet_level+1);
}

void CAT_eink_draw_power_off()
{;
	CAT_eink_clear();
	
	CAT_eink_draw_sprite(0, 0, &tnyspr_protected);

	CAT_eink_draw_string(128, 52, 1, "Device is");
	CAT_eink_draw_string(132, 62, 1, "protected-off");

	CAT_eink_draw_string(160, 81, 1, "Press RESET");
	CAT_eink_draw_string(158, 91, 1, "to power on");

	CAT_eink_draw_string(146, 109, 1, "Please charge");
	CAT_eink_draw_string(142, 119, 1, "device!");
}

void CAT_eink_draw_research()
{
	CAT_printf("This one!\n");

	CAT_eink_clear();

	char buf[256] = {0};
	#define fwrite_str(x, y, s, str, ...) snprintf(buf, sizeof(buf), str, ##__VA_ARGS__); CAT_eink_draw_string(x, y, s, buf);

	CAT_eink_draw_sprite(0, 12, &tnyspr_unicorn_default);
	CAT_eink_draw_sprite(0, 12+tnyspr_unicorn_default.height, &tnyspr_cloud_default);

	fwrite_str(108, 54, 2, CAT_RESEARCH_NAME);
}

bool CAT_eink_should_update()
{
	uint64_t now = CAT_get_RTC_now();
	return 
	eink_boot_update ||
	((CAT_is_charging() &&
	(now - eink_update_timestamp) >= EINK_UPDATE_PERIOD &&
	CAT_input_downtime() >= EINK_UPDATE_PERIOD) ||
	(eink_update_timestamp == 0 && CAT_AQ_sensors_initialized()));
}

void CAT_eink_execute_update()
{
#if CAT_RESEARCH_ONLY
	CAT_eink_draw_research();
#else
	CAT_eink_draw_default();
#endif

	CAT_eink_update(false);
	CAT_set_eink_update_flag(false);

	if(eink_boot_update)
		eink_boot_update = false;
	else
		eink_update_timestamp = CAT_get_RTC_now();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// SCREEN MANAGEMENT

static uint8_t screen_orientation = CAT_SCREEN_ORIENTATION_UP;

void CAT_set_screen_orientation(int orientation)
{
	screen_orientation = orientation;
}

CAT_screen_orientation CAT_get_screen_orientation()
{
	return screen_orientation;
}

#define FLIP_BUFFER_SIZE 28
bool flip_buffer[FLIP_BUFFER_SIZE];
int flip_buffer_head = 0;
bool flip_buffer_init = false;

void CAT_poll_screen_flip()
{
	bool flip = CAT_IMU_is_upside_down();

	if(!flip_buffer_init)
	{
		for(int i = 0; i < FLIP_BUFFER_SIZE-1; i++)
		{
			flip_buffer[i] = flip;
			flip_buffer_head += 1;
		}
		flip_buffer_init = true;
	}
	
	flip_buffer[flip_buffer_head] = flip;
	flip_buffer_head = CAT_wrap(flip_buffer_head+1, FLIP_BUFFER_SIZE);
}

bool CAT_should_flip_screen()
{
	int idx = flip_buffer_head;
	int steps = 0;
	int flips = 0;
	int jerks = 0;
	while(steps < FLIP_BUFFER_SIZE-1)
	{
		int a = idx; int b = CAT_wrap(idx+1, FLIP_BUFFER_SIZE);
		if(flip_buffer[a])
			flips++;
		if(flip_buffer[b] != flip_buffer[a])
			jerks += 1;
		idx = b;
		steps += 1;
	}
	return flips > (FLIP_BUFFER_SIZE/2) && jerks < (flips/2);
}

void CAT_flip_screen()
{
	screen_orientation = !screen_orientation;
	for(int i = 0; i < FLIP_BUFFER_SIZE; i++)
		flip_buffer[i] = false;
	CAT_set_eink_update_flag(true);
}

void CAT_orientation_tick()
{
	if(persist_flags & CAT_PERSIST_CONFIG_FLAG_MANUAL_ORIENT)
		return;
	CAT_poll_screen_flip();
	if(CAT_should_flip_screen())
		CAT_flip_screen();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// AIR QUALITY

CAT_AQ_readings readings =
#ifdef CAT_DESKTOP
{
	.lps22hh.uptime_last_updated = 0,
	.lps22hh.temp = 20,
	.lps22hh.pressure = 1013,

	.sunrise.uptime_last_updated = 0,
	.sunrise.ppm_filtered_compensated = 450,
	.sunrise.ppm_filtered_uncompensated = 450,
	.sunrise.temp = 20,

	.sen5x.uptime_last_updated = 0,
	.sen5x.pm2_5 = 9,
	.sen5x.pm10_0 = 15,
	.sen5x.humidity_rhpct = 40,

	.sen5x.temp_degC = 23,
	.sen5x.voc_index = 1,
	.sen5x.nox_index = 100,
};
#else
{0};
#endif

bool CAT_AQ_sensors_initialized()
{
	return
	readings.sunrise.ppm_filtered_compensated > 0 &&
	(readings.sen5x.temp_degC != 0 ||
	readings.sen5x.humidity_rhpct > 0 ||
	readings.sen5x.pm2_5 > 0);
}

bool CAT_AQ_NOX_VOC_initialized()
{
	return
	readings.sen5x.nox_index > 0 &&
	readings.sen5x.voc_index > 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// IMU

bool CAT_IMU_is_upside_down()
{
	CAT_IMU_values imu;
	CAT_IMU_get_normalized(&imu);
	if(fabs(imu.x) > 0.2 || fabs(imu.z) > 0.2)
		return false;
	if(screen_orientation == CAT_SCREEN_ORIENTATION_UP)
		return imu.y > 0.8;
	return imu.y < -0.8;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// LOGS

bool CAT_logs_initialized()
{
	return CAT_get_log_cell_count() >= 1;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// DEBUG

static int debug_number = 0;
int CAT_get_debug_number()
{
	return debug_number;
}

void CAT_set_debug_number(int x)
{
	debug_number = x;
}
