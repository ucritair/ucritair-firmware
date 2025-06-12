#include "cat_core.h"

#include "cat_aqi.h"
#include <math.h>
#include "cat_structures.h"
#include "cat_version.h"

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

#define ZERO_STATIC_BUFFER(x) (memset(x, 0, sizeof(x)))

void CAT_initialize_save_sector(CAT_save* save, CAT_save_sector sector)
{
	switch (sector)
	{
		case CAT_SAVE_SECTOR_PET:
			CAT_printf("[INFO] Initializing save sector PET\n");
			save->pet.header.label = CAT_SAVE_SECTOR_PET;
			save->pet.header.size = sizeof(save->pet);
			strcpy(save->pet.name, "Waldo");
			save->pet.level = 0;
			save->pet.xp = 0;
			save->pet.lifespan = 30;
			save->pet.lifetime = 0;
			save->pet.vigour = 12;
			save->pet.focus = 12;
			save->pet.spirit = 12;
			break;

		case CAT_SAVE_SECTOR_INVENTORY:
			CAT_printf("[INFO] Initializing save sector INVENTORY\n");
			save->inventory.header.label = CAT_SAVE_SECTOR_INVENTORY;
			save->inventory.header.size = sizeof(save->inventory);
			ZERO_STATIC_BUFFER(save->inventory.counts);
			save->inventory.coins = 0;
			break;

		case CAT_SAVE_SECTOR_DECO:
			CAT_printf("[INFO] Initializing save sector DECO\n");
			save->deco.header.label = CAT_SAVE_SECTOR_DECO;
			save->deco.header.size = sizeof(save->deco);
			ZERO_STATIC_BUFFER(save->deco.props);
			ZERO_STATIC_BUFFER(save->deco.positions);
			ZERO_STATIC_BUFFER(save->deco.overrides);
			ZERO_STATIC_BUFFER(save->deco.children);
			break;

		case CAT_SAVE_SECTOR_HIGHSCORES:
			CAT_printf("[INFO] Initializing save sector HIGHSCORES\n");
			save->highscores.header.label = CAT_SAVE_SECTOR_HIGHSCORES;
			save->highscores.header.size = sizeof(save->highscores);
			save->highscores.snake = 0;
			save->highscores.mine = 0;
			save->highscores.foursquares = 0;
			break;

		case CAT_SAVE_SECTOR_TIMING:
			CAT_printf("[INFO] Initializing save sector TIMING\n");
			save->timing.header.label = CAT_SAVE_SECTOR_TIMING;
			save->timing.header.size = sizeof(save->timing);
			save->timing.stat_timer = 0;
			save->timing.life_timer = 0;
			save->timing.earn_timer = 0;
			save->timing.petting_timer = 0;
			save->timing.petting_count = 0;
			save->timing.milking_count = 0;
			break;

		case CAT_SAVE_SECTOR_CONFIG:
			CAT_printf("[INFO] Initializing save sector CONFIG\n");
			save->config.header.label = CAT_SAVE_SECTOR_CONFIG;
			save->config.header.size = sizeof(save->config);
			save->config.flags = CAT_CONFIG_FLAG_NONE;
			save->config.theme = 0;
			break;

		case CAT_SAVE_SECTOR_FOOTER:
			CAT_printf("[INFO] Initializing save sector FOOTER\n");
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
	CAT_initialize_save_sector(save, CAT_SAVE_SECTOR_TIMING);
	CAT_initialize_save_sector(save, CAT_SAVE_SECTOR_CONFIG);
	CAT_initialize_save_sector(save, CAT_SAVE_SECTOR_FOOTER);
}

CAT_save_error CAT_verify_save_structure(CAT_save* save)
{
	if(save->magic_number != CAT_SAVE_MAGIC)
		return CAT_SAVE_ERROR_MAGIC;

	CAT_save_sector_header* current = &save->pet;

	while(current->label != CAT_SAVE_SECTOR_FOOTER)
	{
		uint8_t* proxy = (uint8_t*) current;
		CAT_save_sector_header* next = proxy + current->size;

		if(next->label != current->label + 1)
		{
			if
			(
				current->label >= CAT_SAVE_SECTOR_CONFIG &&
				current->label < CAT_SAVE_SECTOR_FOOTER &&
				next->label < CAT_SAVE_SECTOR_FOOTER && next->size == 0
			)
			{
				return CAT_SAVE_ERROR_SECTOR_MISSING;
			}
			return CAT_SAVE_ERROR_SECTOR_CORRUPT;
		}
		
		current = next;
	}

	return CAT_SAVE_ERROR_NONE;
}

void CAT_migrate_legacy_save(void* save_location)
{
	CAT_save_legacy legacy;
	memcpy(&legacy, save_location, sizeof(CAT_save_legacy));
	CAT_save* new = (CAT_save*) save_location;
	CAT_initialize_save(new);

	CAT_printf("[INFO] Migrating legacy save to new format\n");
	CAT_printf("[INFO] Save was %d bytes, now %d bytes\n", sizeof(CAT_save_legacy), sizeof(CAT_save));

	strcpy(new->pet.name, legacy.name);
	new->pet.level = legacy.level;
	new->pet.xp = legacy.xp;
	new->pet.lifetime = legacy.lifetime;
	new->pet.vigour = legacy.vigour;
	new->pet.focus = legacy.focus;
	new->pet.spirit = legacy.spirit;

	for(int i = 0; i < 128; i++)
	{
		new->inventory.counts[legacy.bag_ids[i]] = legacy.bag_counts[i];
	}
	new->inventory.coins = legacy.coins;

	for(int i = 0; i < legacy.prop_count; i++)
	{
		new->deco.props[i] = legacy.prop_ids[i]+1;
		new->deco.positions[i*2+0] = legacy.prop_places[i].x;
		new->deco.positions[i*2+1] = legacy.prop_places[i].y;
		new->deco.overrides[i] = legacy.prop_overrides[i];
		new->deco.children[i] = legacy.prop_children[i]+1;
	}

	new->highscores.snake = legacy.snake_high_score;

	new->timing.stat_timer = legacy.stat_timer;
	new->timing.life_timer = legacy.life_timer;
	new->timing.earn_timer = legacy.earn_timer;
	new->timing.petting_timer = legacy.petting_timer;
	new->timing.petting_count = legacy.times_pet;
	new->timing.milking_count = legacy.times_milked;

	new->config.theme = legacy.theme;
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

int CAT_export_config_flags()
{
	return CAT_start_save()->config.flags;
}

void CAT_import_config_flags(int flags)
{
	CAT_start_save()->config.flags = flags;
}

void CAT_set_config_flags(int flags)
{
	CAT_start_save()->config.flags =
	CAT_set_flag(CAT_start_save()->config.flags, flags);
}

void CAT_unset_config_flags(int flags)
{
	CAT_start_save()->config.flags =
	CAT_unset_flag(CAT_start_save()->config.flags, flags);
}

bool CAT_check_config_flags(int flags)
{
	return
	CAT_get_flag(CAT_start_save()->config.flags, flags);
}

static int load_flags = CAT_LOAD_FLAG_NONE;

void CAT_set_load_flags(int flags)
{
	load_flags = CAT_set_flag(load_flags, flags);
}

void CAT_unset_load_flags(int flags)
{
	load_flags = CAT_unset_flag(load_flags, flags);
}

bool CAT_check_load_flags(int flags)
{
	return CAT_get_flag(load_flags, flags);
}


