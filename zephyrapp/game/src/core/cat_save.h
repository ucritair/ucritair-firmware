#include <stdint.h>
#include "cat_math.h"

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
} CAT_save_legacy;

typedef enum
{
	CAT_SAVE_CONFIG_FLAG_NONE = 0,
	CAT_SAVE_CONFIG_FLAG_DEVELOPER = (1 << 0),
	CAT_SAVE_CONFIG_FLAG_KALI_YUGA = (1 << 1)
} CAT_save_config_flag;

typedef enum
{
	CAT_SAVE_SECTOR_PET,
	CAT_SAVE_SECTOR_INVENTORY,
	CAT_SAVE_SECTOR_DECO,
	CAT_SAVE_SECTOR_HIGHSCORES,
	CAT_SAVE_SECTOR_CONFIG,
	CAT_SAVE_SECTOR_FOOTER,
} CAT_save_sector;

typedef struct __attribute__((__packed__))
{
	uint8_t label;
	uint16_t size;
} CAT_save_sector_header;

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
		char name[24];
		uint8_t lifespan;
		uint8_t lifetime;
		uint16_t incarnations;
		uint8_t level;
		uint32_t xp;
		uint8_t vigour;
		uint8_t focus;
		uint8_t spirit;
	} pet;

	// SECTOR : INVENTORY
	struct __attribute__((__packed__))
	{
		CAT_save_sector_header header;
		uint16_t counts[256];
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
		uint16_t mines;
		uint16_t foursquares;
		uint16_t stroop;
	} highscores;

	// SECTOR : CONFIG
	struct __attribute__((__packed__))
	{
		CAT_save_sector_header header;
		uint64_t flags;
		uint8_t theme;
	} config;

	// FOOTER
	CAT_save_sector_header footer;
} CAT_save;

typedef enum
{
	CAT_SAVE_ERROR_NONE,
	CAT_SAVE_ERROR_MAGIC,
	CAT_SAVE_ERROR_SECTOR_CORRUPT,
	CAT_SAVE_ERROR_SECTOR_MISSING
} CAT_save_error;

void CAT_initialize_save(CAT_save* save);
void CAT_migrate_legacy_save(void* save);
void CAT_extend_save(CAT_save* save);
CAT_save_error CAT_verify_save_structure(CAT_save* save);

CAT_save* CAT_start_save();
void CAT_finish_save(CAT_save* save);
CAT_save* CAT_start_load();

uint64_t CAT_get_config_flags();
void CAT_set_config_flags(uint64_t flags);

void CAT_raise_config_flags(uint64_t flags);
void CAT_lower_config_flags(uint64_t flags);
bool CAT_check_config_flags(uint64_t flags);

typedef enum
{
	CAT_LOAD_FLAG_NONE = 0,
	CAT_LOAD_FLAG_DIRTY = 1,
	CAT_LOAD_FLAG_DEFAULT = 2,
	CAT_LOAD_FLAG_TURNKEY = 4
} CAT_load_flag;

void CAT_set_load_flags(uint64_t flags);
void CAT_unset_load_flags(uint64_t flags);
bool CAT_check_load_flags(uint64_t flags);