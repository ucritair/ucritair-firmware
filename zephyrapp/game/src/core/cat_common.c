#include "cat_core.h"

#include "cat_aqi.h"
#include <math.h>
#include "cat_structures.h"
#include "cat_version.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "item_assets.h"


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
	return round(f * scale_factor);
}

float int2float(int i, float scale_factor)
{
	return i / scale_factor;
}

int move_average(int x_bar, int samples, float x, float scale_factor)
{
	float x_bar_f = int2float(x_bar, scale_factor);
	//x_bar_f = x_bar_f + (x - x_bar_f) / (float) samples;
	x_bar_f = x_bar_f + (x - x_bar_f) / 7;
	return float2int(x_bar_f, scale_factor);
}

void CAT_AQ_move_scores()
{
	CAT_AQ_score_block* block = CAT_AQ_get_moving_scores();
	if(block->sample_count > 0)
	{
		block->CO2 = move_average(block->CO2, block->sample_count, readings.sunrise.ppm_filtered_uncompensated, 1);
		block->NOX = move_average(block->NOX, block->sample_count, readings.sen5x.nox_index, 1);
		block->VOC = move_average(block->VOC, block->sample_count, readings.sen5x.voc_index, 1);
		block->PM2_5 = move_average(block->PM2_5, block->sample_count, readings.sen5x.pm2_5, 100);
		block->temp = move_average(block->temp, block->sample_count, CAT_canonical_temp(), 1);
		block->rh = move_average(block->rh, block->sample_count, readings.sen5x.humidity_rhpct, 100);
		block->aggregate = move_average(block->aggregate, block->sample_count, CAT_AQ_aggregate_score(), 1);
	}
	else
	{
		block->CO2 = float2int(readings.sunrise.ppm_filtered_uncompensated, 1);
		block->NOX = float2int(readings.sen5x.nox_index, 1);
		block->VOC = float2int(readings.sen5x.voc_index, 1);
		block->PM2_5 = float2int(readings.sen5x.pm2_5, 100);
		block->temp = float2int(CAT_canonical_temp(), 1000);
		block->rh = float2int(readings.sen5x.humidity_rhpct, 100);
		block->aggregate = float2int(CAT_AQ_aggregate_score(), 1);	
	}
	block->sample_count += 1;

	CAT_printf("[MOVING AVERAGES]\n");
	CAT_printf("CO2: %f NOX: %f\n", int2float(block->CO2, 1), int2float(block->NOX, 1));
	CAT_printf("VOC: %f PM2_5: %f\n", int2float(block->VOC, 1), int2float(block->PM2_5, 100));
	CAT_printf("temp: %f RH: %f\n", int2float(block->temp, 1000), int2float(block->rh, 100));
	CAT_printf("aggregate: %f count: %d\n", int2float(block->aggregate, 1), block->sample_count);		
}

void CAT_AQ_buffer_scores(CAT_AQ_score_block* block)
{
	memcpy(block, CAT_AQ_get_moving_scores(), sizeof(CAT_AQ_score_block));
	block->sample_count = 0;
}

void CAT_AQ_read_scores(int idx, CAT_AQ_score_block* out)
{
	if(idx < 0 || idx >= 7)
		return;
	idx = (CAT_AQ_get_score_buffer_head() + idx) % 7;
	memcpy(out, &(CAT_AQ_get_score_buffer()[idx]), sizeof(CAT_AQ_score_block));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// TIME

void CAT_get_datetime(CAT_datetime* datetime)
{
	time_t now = CAT_get_RTC_now();
	struct tm local;
	gmtime_r(&now, &local);
	
	datetime->year = local.tm_year;
	datetime->month = local.tm_mon+1;
	datetime->day = local.tm_mday;
	datetime->hour = local.tm_hour;
	datetime->minute = local.tm_min;
	datetime->second = local.tm_sec;	
}

int CAT_datecmp(CAT_datetime* a, CAT_datetime* b)
{
	if(a->year > b->year)
		return 1;
	if(a->year < b->year)
		return -1;
	if(a->month > b->month)
		return 1;
	if(a->month < b->month)
		return -1;
	if(a->day > b->day)
		return 1;
	if(a->day < b->day)
		return -1;
	return 0;
}

int CAT_timecmp(CAT_datetime* a, CAT_datetime* b)
{
	if(a->year > b->year)
		return 1;
	if(a->year < b->year)
		return -1;
	if(a->month > b->month)
		return 1;
	if(a->month < b->month)
		return -1;
	if(a->day > b->day)
		return 1;
	if(a->day < b->day)
		return -1;
	if(a->hour > b->hour)
		return 1;
	if(a->hour < b->hour)
		return -1;
	if(a->minute > b->minute)
		return 1;
	if(a->minute < b->minute)
		return -1;
	if(a->second > b->second)
		return 1;
	if(a->second < b->second)
		return -1;
	return 0;
}

void CAT_make_datetime(uint64_t timestamp, CAT_datetime* datetime)
{
	struct tm t;
	gmtime_r(&timestamp, &t);
	datetime->year = t.tm_year;
	datetime->month = t.tm_mon+1;
	datetime->day = t.tm_mday;
	datetime->hour = t.tm_hour;
	datetime->minute = t.tm_min;
	datetime->second = t.tm_sec;
}

uint64_t CAT_make_timestamp(CAT_datetime* datetime)
{
	struct tm t;
	t.tm_year = datetime->year;
	t.tm_mon = datetime->month-1;
	t.tm_mday = datetime->day;
	t.tm_hour = datetime->hour;
	t.tm_min = datetime->minute;
	t.tm_sec = datetime->second;
	return timegm(&t);
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
			save->highscores.mine = 0;
			save->highscores.foursquares = 0;
			break;

		case CAT_SAVE_SECTOR_CONFIG:
			save->config.header.label = CAT_SAVE_SECTOR_CONFIG;
			save->config.header.size = sizeof(save->config);
			save->config.flags = CAT_CONFIG_FLAG_NONE;
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
	new->pet.lifetime = min(15, migration_buffer.lifetime);
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

	new->config.flags = CAT_CONFIG_FLAG_NONE;
	if(migration_buffer.save_flags & 1)
		new->config.flags |= CAT_CONFIG_FLAG_DEVELOPER;
	if(migration_buffer.temperature_unit == CAT_TEMPERATURE_UNIT_DEGREES_FAHRENHEIT)
		new->config.flags |= CAT_CONFIG_FLAG_USE_FAHRENHEIT;
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

static uint64_t config_flags = CAT_CONFIG_FLAG_NONE;

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

void CAT_print_datetime(const char* title, CAT_datetime* t)
{
	CAT_printf
	(
		"%s: %.2d/%.2d/%.4d %.2d:%.2d:%.2d\n",
		title, t->month, t->day, t->year, t->hour, t->minute, t->second
	);
}
void CAT_print_timestamp(const char* title, uint64_t t)
{
	CAT_datetime dt;
	CAT_make_datetime(t, &dt);
	CAT_print_datetime(title, &dt);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// PERSISTENCE

static uint32_t stash;

void CAT_stash(uint32_t x)
{
	stash = x;
}

uint32_t CAT_pop()
{
	return stash;
}
