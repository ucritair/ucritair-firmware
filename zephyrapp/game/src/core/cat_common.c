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


////////////////////////////////////////////////////////////////////////////////////////////////////
// EINK SCREEN

#define EINK_UPDATE_PERIOD CAT_MINUTE_SECONDS

static bool eink_boot_update = true;
static bool eink_dirty = false;
static uint64_t eink_update_timestamp = 0;

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
	CAT_eink_update();
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

bool CAT_AQ_logs_initialized()
{
	return CAT_get_log_cell_count() >= 1;
}

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

int float2int(float f, int scale_factor)
{
	return roundf(f * scale_factor);
}

float int2float(int i, float scale_factor)
{
	return i / scale_factor;
}

int move_average(int x_bar, float x, float scale_factor)
{
	float x_bar_f = int2float(x_bar, scale_factor);
	float n = aq_moving_score_samples+1;
	float old_weight = (n-1) / n;
	float new_weight = 1.0f / n;
	x_bar_f = x_bar_f * old_weight + x * new_weight;
	return float2int(x_bar_f, scale_factor);
}

bool CAT_AQ_moving_scores_initialized()
{
	return aq_moving_score_time > 0;
}

void CAT_AQ_update_moving_scores()
{
	CAT_AQ_score_block* block = &aq_moving_scores;
	block->CO2 = move_average(block->CO2, readings.sunrise.ppm_filtered_uncompensated, 1);
	block->NOX = move_average(block->NOX, readings.sen5x.nox_index, 1);
	block->VOC = move_average(block->VOC, readings.sen5x.voc_index, 1);
	block->PM2_5 = move_average(block->PM2_5, readings.sen5x.pm2_5, 100);
	block->temp = move_average(block->temp, CAT_canonical_temp(), 1000);
	block->rh = move_average(block->rh, readings.sen5x.humidity_rhpct, 100);
	block->aggregate = move_average(block->aggregate, CAT_AQ_aggregate_score(), 1);
	
	aq_moving_score_samples += 1;
	aq_moving_score_time = CAT_get_RTC_now();
}

bool CAT_AQ_weekly_scores_initialized()
{
	return aq_weekly_score_time > 0;
}

void CAT_AQ_push_weekly_scores(CAT_AQ_score_block* in)
{
	if(aq_weekly_score_time == 0)
	{
		for(int i = 0; i < 7; i++)
			memcpy(&aq_weekly_scores[i], &aq_moving_scores, sizeof(CAT_AQ_score_block));
		aq_weekly_score_head = 0;
	}

	CAT_AQ_score_block* block = &aq_weekly_scores[aq_weekly_score_head];
	memcpy(block, in, sizeof(CAT_AQ_score_block));

	aq_weekly_score_head = (aq_weekly_score_head+1) % 7;
	aq_weekly_score_time = CAT_get_RTC_now();
	aq_moving_score_samples = 0;
}

CAT_AQ_score_block* CAT_AQ_get_weekly_scores(int idx)
{
	idx = (aq_weekly_score_head+idx) % 7;
	return &aq_weekly_scores[idx];
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
// SAVE

#define ZERO_STATIC_BUFFER(x) (memset(x, 0, sizeof(x)))

void CAT_initialize_save_sector(CAT_save* save, CAT_save_sector sector)
{
	switch (sector)
	{
		case CAT_SAVE_SECTOR_PET:
			save->pet.header.label = CAT_SAVE_SECTOR_PET;
			save->pet.header.size = sizeof(save->pet);
			strncpy(save->pet.name, CAT_DEFAULT_PET_NAME, 24);
			save->pet.lifespan = 30;
			save->pet.lifetime = 0;
			save->pet.incarnations = 1;
			save->pet.level = 0;
			save->pet.xp = 0;
			save->pet.vigour = 12;
			save->pet.focus = 12;
			save->pet.spirit = 12;
			break;

		case CAT_SAVE_SECTOR_INVENTORY:
			save->inventory.header.label = CAT_SAVE_SECTOR_INVENTORY;
			save->inventory.header.size = sizeof(save->inventory);
			ZERO_STATIC_BUFFER(save->inventory.counts);
			break;

		case CAT_SAVE_SECTOR_DECO:
			save->deco.header.label = CAT_SAVE_SECTOR_DECO;
			save->deco.header.size = sizeof(save->deco);
			ZERO_STATIC_BUFFER(save->deco.props);
			ZERO_STATIC_BUFFER(save->deco.positions);
			ZERO_STATIC_BUFFER(save->deco.overrides);
			ZERO_STATIC_BUFFER(save->deco.children);
			break;

		case CAT_SAVE_SECTOR_HIGHSCORES:
			save->highscores.header.label = CAT_SAVE_SECTOR_HIGHSCORES;
			save->highscores.header.size = sizeof(save->highscores);
			save->highscores.snake = 0;
			save->highscores.mines = 0;
			save->highscores.foursquares = 0;
			break;

		case CAT_SAVE_SECTOR_CONFIG:
			save->config.header.label = CAT_SAVE_SECTOR_CONFIG;
			save->config.header.size = sizeof(save->config);
			save->config.flags = CAT_SAVE_CONFIG_FLAG_NONE;
			save->config.theme = 0;
			break;

		case CAT_SAVE_SECTOR_FOOTER:
			save->footer.label = CAT_SAVE_SECTOR_FOOTER;
			save->footer.size = 0;
			break;
	}
}

void CAT_initialize_save(CAT_save* save)
{
	save->magic_number = CAT_SAVE_MAGIC;

	save->version_major = CAT_VERSION_MAJOR;
	save->version_minor = CAT_VERSION_MINOR;
	save->version_patch = CAT_VERSION_PATCH;
	save->version_push = CAT_VERSION_PUSH;

	CAT_initialize_save_sector(save, CAT_SAVE_SECTOR_PET);
	CAT_initialize_save_sector(save, CAT_SAVE_SECTOR_INVENTORY);
	CAT_initialize_save_sector(save, CAT_SAVE_SECTOR_DECO);
	CAT_initialize_save_sector(save, CAT_SAVE_SECTOR_HIGHSCORES);
	CAT_initialize_save_sector(save, CAT_SAVE_SECTOR_CONFIG);
	CAT_initialize_save_sector(save, CAT_SAVE_SECTOR_FOOTER);
}

bool verify_sector_label(CAT_save* save, CAT_save_sector sector)
{
	switch(sector)
	{
		case CAT_SAVE_SECTOR_PET: return save->pet.header.label == CAT_SAVE_SECTOR_PET;
		case CAT_SAVE_SECTOR_INVENTORY: return save->inventory.header.label == CAT_SAVE_SECTOR_INVENTORY;
		case CAT_SAVE_SECTOR_DECO: return save->deco.header.label == CAT_SAVE_SECTOR_DECO;
		case CAT_SAVE_SECTOR_HIGHSCORES: return save->highscores.header.label == CAT_SAVE_SECTOR_HIGHSCORES;
		case CAT_SAVE_SECTOR_CONFIG: return save->config.header.label == CAT_SAVE_SECTOR_CONFIG;
		case CAT_SAVE_SECTOR_FOOTER: return save->footer.label ==  CAT_SAVE_SECTOR_FOOTER;
	}
}

bool verify_sector_size(CAT_save* save, CAT_save_sector sector)
{
	switch(sector)
	{
		case CAT_SAVE_SECTOR_PET: return save->pet.header.size == sizeof(save->pet);
		case CAT_SAVE_SECTOR_INVENTORY: return save->inventory.header.size == sizeof(save->inventory);
		case CAT_SAVE_SECTOR_DECO: return save->deco.header.size == sizeof(save->deco);
		case CAT_SAVE_SECTOR_HIGHSCORES: return save->highscores.header.size == sizeof(save->highscores);
		case CAT_SAVE_SECTOR_CONFIG: return save->config.header.size == sizeof(save->config);
		case CAT_SAVE_SECTOR_FOOTER: return save->footer.size == 0;
	}
}

CAT_save_error CAT_verify_save_structure(CAT_save* save)
{
	if(save->magic_number != CAT_SAVE_MAGIC)
		return CAT_SAVE_ERROR_MAGIC;

	CAT_save_sector_header* current = &save->pet;
	int sectors_traversed = 0;

	while(current->label < CAT_SAVE_SECTOR_FOOTER)
	{
		sectors_traversed += 1;

		if(!verify_sector_label(save, current->label))
		{
			return CAT_SAVE_ERROR_SECTOR_CORRUPT;
		}
		if(!verify_sector_size(save, current->label))
		{
			return CAT_SAVE_ERROR_SECTOR_CORRUPT;
		}

		uint8_t* proxy = (uint8_t*) current;
		CAT_save_sector_header* next = proxy + current->size;

		if(next->label != current->label + 1)
		{
			if
			(
				current->label == CAT_SAVE_SECTOR_CONFIG &&
				next->label < CAT_SAVE_SECTOR_FOOTER && next->size == 0
			)
			{
				return CAT_SAVE_ERROR_SECTOR_MISSING;
			}

			return CAT_SAVE_ERROR_SECTOR_CORRUPT;
		}
		
		current = next;
	}

	if(sectors_traversed < CAT_SAVE_SECTOR_FOOTER)
	{
		return CAT_SAVE_ERROR_SECTOR_CORRUPT;
	}

	return CAT_SAVE_ERROR_NONE;
}

CAT_save_legacy migration_buffer;

void CAT_migrate_legacy_save(void* save)
{
	memcpy(&migration_buffer, save, sizeof(CAT_save_legacy));

	CAT_save* new = save;
	CAT_initialize_save(new);

	CAT_printf("[INFO] Migrating legacy save to new format...\n");

	strncpy(new->pet.name, migration_buffer.name, 24);
	new->pet.lifespan = 30;
	new->pet.lifetime = CAT_min(15, migration_buffer.lifetime);
	new->pet.incarnations = 1;
	new->pet.level = migration_buffer.level;
	new->pet.xp = migration_buffer.xp;
	new->pet.vigour = migration_buffer.vigour;
	new->pet.focus = migration_buffer.focus;
	new->pet.spirit = migration_buffer.spirit;

	for(int i = 0; i < migration_buffer.bag_length; i++)
	{
		new->inventory.counts[migration_buffer.bag_ids[i]] = migration_buffer.bag_counts[i];
	}
	new->inventory.counts[coin_item] = migration_buffer.coins;

	for(int i = 0; i < migration_buffer.prop_count; i++)
	{
		new->deco.props[i] = migration_buffer.prop_ids[i]+1;
		new->deco.positions[i*2+0] = migration_buffer.prop_places[i].x;
		new->deco.positions[i*2+1] = migration_buffer.prop_places[i].y;
		new->deco.overrides[i] = migration_buffer.prop_overrides[i];
		new->deco.children[i] = migration_buffer.prop_children[i]+1;
	}

	new->highscores.snake = migration_buffer.snake_high_score;

	new->config.flags = CAT_SAVE_CONFIG_FLAG_NONE;
	if(migration_buffer.save_flags & 1)
		new->config.flags |= CAT_SAVE_CONFIG_FLAG_DEVELOPER;
	if(migration_buffer.temperature_unit == CAT_TEMPERATURE_UNIT_DEGREES_FAHRENHEIT)
		persist_flags |= CAT_PERSIST_CONFIG_FLAG_USE_FAHRENHEIT;
	new->config.theme = migration_buffer.theme;
}

void CAT_extend_save(CAT_save* save)
{
	CAT_save_sector_header* current = &save->pet;
	CAT_save_sector missing_sector = CAT_SAVE_SECTOR_FOOTER;

	while(current->label != CAT_SAVE_SECTOR_FOOTER)
	{
		uint8_t* proxy = (uint8_t*) current;
		CAT_save_sector_header* next = proxy + current->size;

		if(next->label < CAT_SAVE_SECTOR_FOOTER && next->size == 0)
		{
			missing_sector = next->label;
			break;
		}

		current = next;
	}

	while(missing_sector <= CAT_SAVE_SECTOR_FOOTER)
	{
		CAT_initialize_save_sector(save, missing_sector);
		missing_sector += 1;
	}
}

static uint64_t config_flags = CAT_SAVE_CONFIG_FLAG_NONE;

uint64_t CAT_get_config_flags()
{
	return config_flags;
}

void CAT_set_config_flags(uint64_t flags)
{
	config_flags = flags;
}

void CAT_raise_config_flags(uint64_t flags)
{
	config_flags |= flags; 
}

void CAT_lower_config_flags(uint64_t flags)
{
	config_flags &= ~flags;
}

bool CAT_check_config_flags(uint64_t flags)
{
	return config_flags & flags;
}

static uint64_t load_flags = CAT_LOAD_FLAG_NONE;

void CAT_set_load_flags(uint64_t flags)
{
	load_flags |= flags;
}

void CAT_unset_load_flags(uint64_t flags)
{
	load_flags &= ~flags;
}

bool CAT_check_load_flags(uint64_t flags)
{
	return load_flags & flags;
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
