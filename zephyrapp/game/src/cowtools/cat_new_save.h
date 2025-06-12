#pragma once

#include <stdint.h>
#include "cat_math.h"
#include <string.h>
#include "cat_version.h"
#include "cat_item.h"

#define CAT_SAVE_MAGIC 0xaabbccde

typedef struct __attribute__((__packed__))
{
	uint32_t magic_number;

	uint8_t version_major;
	uint8_t version_minor;
	uint8_t version_patch;
	uint8_t version_push;

	uint8_t vigour;
	uint8_t focus;
	uint8_t spirit;
	uint32_t lifetime;

	uint8_t prop_ids[150];
	CAT_ivec2 prop_places[150];
	uint8_t prop_overrides[150];
	int16_t prop_children[150];
	uint8_t prop_count;

	uint8_t bag_ids[128];
	uint8_t bag_counts[128];
	uint8_t bag_length;
	uint32_t coins;

	uint16_t snake_high_score;

	float stat_timer;
	float life_timer;
	float earn_timer;
	uint8_t times_pet;
	float petting_timer;
	uint8_t times_milked;

	char name[32];

	uint32_t theme;

	uint16_t level;
	uint32_t xp;

	uint8_t lcd_brightness;
	uint8_t led_brightness;

	uint8_t temperature_unit;

	int save_flags;
} CAT_legacy_save;

typedef enum
{
	CAT_SAVE_SECTOR_PET = 0,
	CAT_SAVE_SECTOR_INVENTORY = 1,
	CAT_SAVE_SECTOR_DECO = 2,
	CAT_SAVE_SECTOR_HIGHSCORES = 3,
	CAT_SAVE_SECTOR_TIMING = 4,
	CAT_SAVE_SECTOR_CONFIG = 5,
	CAT_SAVE_SECTOR_FOOTER
} CAT_save_sector;

typedef struct __attribute__((__packed__))
{
	uint8_t label;
	uint16_t size;
} CAT_save_sector_header;

typedef enum
{
	CAT_CONFIG_FLAG_NONE = 0,
	CAT_CONFIG_FLAG_DEVELOPER = 1,
	CAT_CONFIG_FLAG_USE_FAHRENHEIT = 2,
	CAT_CONFIG_FLAG_AQ_FIRST = 4,
} CAT_config_flag;

typedef struct __attribute__((__packed__))
{
	// HEADER : MAGIC NUMBER
	uint32_t magic_number;

	// HEADER : VERSION
	uint8_t version_major;
	uint8_t version_minor;
	uint8_t version_patch;
	uint8_t version_push;

	// SECTOR : PET
	struct __attribute__((__packed__))
	{
		CAT_save_sector_header header;
		char name[32];
		uint8_t level;
		uint32_t xp;
		uint8_t lifespan;
		uint8_t lifetime;
		uint8_t vigour;
		uint8_t focus;
		uint8_t spirit;
	} pet;

	// SECTOR : INVENTORY
	struct __attribute__((__packed__))
	{
		CAT_save_sector_header header;
		uint8_t counts[256];
		uint32_t coins;
	} inventory;

	// SECTOR : DECO
	struct __attribute__((__packed__))
	{
		CAT_save_sector_header header;
		uint8_t props[150];
		uint8_t positions[150*2];
		uint8_t overrides[150];
		uint8_t children[150];
	} deco;
	
	// SECTOR : HIGHSCORES
	struct __attribute__((__packed__))
	{
		CAT_save_sector_header header;
		uint16_t snake;
		uint16_t mine;
		uint16_t foursquares;
	} highscores;

	// SECTOR : TIMING
	struct __attribute__((__packed__))
	{
		CAT_save_sector_header header;
		uint32_t care_timer;
		uint32_t life_timer;
		uint32_t earn_timer;
		uint32_t petting_timer;
		uint8_t petting_count;
		uint8_t milking_count;
	} timing;

	// SECTOR : CONFIG
	struct __attribute__((__packed__))
	{
		CAT_save_sector_header header;
		uint64_t config_flags;
		uint8_t room_theme;
	} config;

	// FOOTER
	CAT_save_sector_header footer;
} CAT_save;

#define ZERO_STATIC_BUFFER(x) (memset(x, 0, sizeof(x)))

void CAT_initialize_save(CAT_save* save)
{
	save->magic_number = CAT_SAVE_MAGIC;

	save->version_major = CAT_VERSION_MAJOR;
	save->version_minor = CAT_VERSION_MINOR;
	save->version_patch = CAT_VERSION_PATCH;
	save->version_push = CAT_VERSION_PUSH;

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

	save->inventory.header.label = CAT_SAVE_SECTOR_INVENTORY;
	save->inventory.header.size = sizeof(save->inventory);
	ZERO_STATIC_BUFFER(save->inventory.counts);
	save->inventory.coins = 0;

	save->deco.header.label = CAT_SAVE_SECTOR_DECO;
	save->deco.header.size = sizeof(save->deco);
	ZERO_STATIC_BUFFER(save->deco.props);
	ZERO_STATIC_BUFFER(save->deco.positions);
	ZERO_STATIC_BUFFER(save->deco.overrides);
	ZERO_STATIC_BUFFER(save->deco.children);

	save->highscores.header.label = CAT_SAVE_SECTOR_HIGHSCORES;
	save->highscores.header.size = sizeof(save->highscores);
	save->highscores.snake = 0;
	save->highscores.mine = 0;
	save->highscores.foursquares = 0;

	save->timing.header.label = CAT_SAVE_SECTOR_TIMING;
	save->timing.header.size = sizeof(save->timing);
	save->timing.care_timer = 0;
	save->timing.life_timer = 0;
	save->timing.earn_timer = 0;
	save->timing.petting_timer = 0;
	save->timing.petting_count = 0;
	save->timing.milking_count = 0;

	save->config.header.label = CAT_SAVE_SECTOR_CONFIG;
	save->config.header.size = sizeof(save->config);
	save->config.config_flags = CAT_CONFIG_FLAG_NONE;
	save->config.room_theme = 0;

	save->footer.label = CAT_SAVE_SECTOR_FOOTER;
	save->footer.size = 0;
}

void CAT_migrate_legacy_save(void* save_location)
{
	CAT_legacy_save legacy;
	memcpy(&legacy, save_location, sizeof(CAT_legacy_save));
	CAT_save* new = (CAT_save*) save_location;
	CAT_initialize_save(new);

	strcpy(new->pet.name, "Waldo");
	new->pet.level = legacy.level;
	new->pet.xp = legacy.xp;
	new->pet.lifetime = legacy.lifetime;
	new->pet.vigour = legacy.vigour;
	new->pet.focus = legacy.focus;
	new->pet.spirit = legacy.spirit;

	ZERO_STATIC_BUFFER(new->inventory.counts);
	for(int i = 0; i < item_table.length; i++)
	{
		new->inventory.counts[i] = legacy.bag_counts[i];
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

	new->timing.care_timer = legacy.stat_timer;
	new->timing.life_timer = legacy.life_timer;
	new->timing.earn_timer = legacy.earn_timer;
	new->timing.petting_timer = legacy.petting_timer;
	new->timing.petting_count = legacy.times_pet;
	new->timing.milking_count = legacy.times_milked;

	new->config.room_theme = legacy.theme;
}