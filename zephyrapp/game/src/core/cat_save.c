#include "cat_save.h"

#include "cat_pet.h"
#include "cat_room.h"
#include "cat_arcade.h"
#include "cat_item.h"
#include "cat_persist.h"
#include "cat_version.h"

#include "theme_assets.h"
#include "item_assets.h"

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

static uint64_t save_flags = CAT_SAVE_CONFIG_FLAG_NONE;

uint64_t CAT_get_save_flags()
{
	return save_flags;
}

void CAT_set_save_flags(uint64_t flags)
{
	save_flags = flags;
}

void CAT_raise_save_flags(uint64_t flags)
{
	save_flags |= flags; 
}

void CAT_lower_save_flags(uint64_t flags)
{
	save_flags &= ~flags;
}

void CAT_toggle_save_flags(uint64_t flags)
{
	save_flags ^= flags;
}

bool CAT_check_save_flags(uint64_t flags)
{
	return save_flags & flags;
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

void CAT_force_save()
{	
	CAT_printf("Saving...\n");
	CAT_save* save = CAT_start_save();
	CAT_initialize_save(save);

	strcpy(save->pet.name, pet.name);

	save->pet.lifespan = pet.lifespan;
	save->pet.lifetime = pet.lifetime;
	save->pet.incarnations = pet.incarnations;

	save->pet.level = pet.level;
	save->pet.xp = pet.xp;

	save->pet.vigour = pet.vigour;
	save->pet.focus = pet.focus;
	save->pet.spirit = pet.spirit;

	CAT_pickup_list* pickups = CAT_room_get_pickups();
	for(int i = 0; i < pickups->length; i++)
	{
		CAT_room_consume_pickup(i);
	}
	for(int i = 0; i < item_table.length; i++)
	{
		save->inventory.counts[i] = item_table.counts[i];
	}

	CAT_prop_list* props = CAT_room_get_props();
	for(int i = 0; i < props->length; i++)
	{
		struct prop_list_item* item = &props->data[i];
		save->deco.props[i] = item->prop + 1;
		save->deco.positions[i*2+0] = item->x0;
		save->deco.positions[i*2+1] = item->y0;
		save->deco.overrides[i] = item->override;
		save->deco.children[i] = item->child + 1;
	}

	save->highscores.snake = snake_highscore;
	save->highscores.mines = mines_highscore;
	save->highscores.foursquares = foursquares_highscore;

	save->config.flags = CAT_get_save_flags();

	for(int i = 0; i < CAT_ROOM_THEME_COUNT; i++)
	{
		if(CAT_room_theme_list[i] == CAT_room_get_theme())
		{
			save->config.theme = i;
		}
	}

	CAT_finish_save(save);
	CAT_printf("Save complete!\n");
}

void CAT_load_default()
{
	CAT_pet_init();
	CAT_room_init();

	CAT_inventory_clear();
	CAT_inventory_add(food_bread_item, 2);
	CAT_inventory_add(food_milk_item, 2);
	CAT_inventory_add(food_coffee_item, 1);
	CAT_inventory_add(prop_succulent_item, 1);
	CAT_inventory_add(toy_laser_pointer_item, 1);
	CAT_inventory_add(coin_item, 100);

	CAT_room_init();
	CAT_room_place_prop(2, 0, prop_crafter_item);
	CAT_room_place_prop(6, 3, prop_table_mahogany_item);
	CAT_room_place_prop(4, 3, prop_chair_mahogany_item);
}

void CAT_load_turnkey()
{
	CAT_pet_init();
	CAT_room_init();

	CAT_inventory_clear();
	CAT_inventory_add(book_1_item, 1);
	CAT_inventory_add(food_bread_item, 2);
	CAT_inventory_add(food_milk_item, 2);
	CAT_inventory_add(food_coffee_item, 1);
	CAT_inventory_add(prop_succulent_item, 1);
	CAT_inventory_add(toy_baseball_item, 1);
	CAT_inventory_add(toy_laser_pointer_item, 1);
	CAT_inventory_add(coin_item, 100);
	CAT_inventory_add(prop_portal_orange_item, 1);
	CAT_inventory_add(prop_portal_blue_item, 1);
	CAT_inventory_add(prop_hoopy_item, 1);

	CAT_room_init();
	CAT_room_place_prop(0, 0, prop_plant_plain_item);
	CAT_room_place_prop(2, 0, prop_crafter_item);
	CAT_room_place_prop(3, 3, prop_table_mahogany_item);
	CAT_room_stack_prop(CAT_room_get_props()->length-1, prop_xen_crystal_item);
	CAT_room_place_prop(1, 3, prop_chair_mahogany_item);
	CAT_room_place_prop(7, 4, prop_stool_wood_item);
	CAT_room_place_prop(13, 2, prop_bush_plain_item);
	CAT_room_place_prop(13, 3, prop_bush_plain_item);
	CAT_room_place_prop(13, 7, prop_table_sm_plastic_item);
	CAT_room_stack_prop(CAT_room_get_props()->length-1, prop_coffeemaker_item);
	CAT_room_place_prop(0, 9, prop_plant_daisy_item);

#if CAT_PRIORITIZE_AQ
	persist_flags |= CAT_PERSIST_CONFIG_FLAG_AQ_FIRST | CAT_PERSIST_CONFIG_FLAG_PAUSE_CARE;
#endif

	CAT_datetime zero_day =
	{
		.year = 2025,
		.month = 11,
		.day = 8
	};
	CAT_set_datetime(zero_day);
	CAT_erase_logs();
}

void CAT_force_load()
{	
	CAT_printf("Load requested...\n");
	
	CAT_save* save = CAT_start_load();

	if(CAT_check_load_flags(CAT_LOAD_FLAG_DEFAULT))
	{
		CAT_printf("Reset flag encountered...\n");
		CAT_initialize_save(save);
		CAT_load_default();
		CAT_force_save();
		CAT_printf("Game state reset and saved!\n");
		CAT_unset_load_flags(CAT_LOAD_FLAG_DEFAULT);
		return;
	}
	else if(CAT_check_load_flags(CAT_LOAD_FLAG_TURNKEY))
	{	
		CAT_printf("Turnkey flag encountered...\n");
		CAT_initialize_save(save);
		CAT_load_turnkey();
		CAT_force_save();
		CAT_printf("Game state set to turnkey configuration!\n");
		CAT_unset_load_flags(CAT_LOAD_FLAG_TURNKEY);
		return;
	}
	else
	{
		int save_status = CAT_verify_save_structure(save);

		if(save_status == CAT_SAVE_ERROR_MAGIC)
		{
			CAT_printf("Invalid save header...\n");
			CAT_initialize_save(save);
			CAT_load_default();
			CAT_inventory_add(save_reset_mark_item, 1);
			CAT_force_save();
			CAT_printf("Game state reset!\n");
			return;
		}
		else if(save_status == CAT_SAVE_ERROR_SECTOR_CORRUPT)
		{
			CAT_printf("Save requires migration...\n");
			CAT_migrate_legacy_save(save);
			CAT_inventory_add(save_migrate_mark_item, 1);
			CAT_printf("Save migrated!\n");
		}
		else if(save_status == CAT_SAVE_ERROR_SECTOR_MISSING)
		{
			CAT_printf("Save is missing sector...\n");
			CAT_extend_save(save);
			CAT_inventory_add(save_extend_mark_item, 1);
			CAT_printf("Save extended!\n");
		}
	}

	CAT_printf("Loading...\n");

	if(strlen(save->pet.name) <= sizeof(pet.name))
		strncpy(pet.name, save->pet.name, sizeof(pet.name));

	if(save->pet.lifespan <= 30)
		pet.lifespan = save->pet.lifespan;
	pet.lifetime = CAT_min(save->pet.lifetime, 31);
	if(save->pet.incarnations <= UINT16_MAX)
		pet.incarnations = save->pet.incarnations;

	if(save->pet.level < CAT_NUM_LEVELS)
		pet.level = save->pet.level;
	if(save->pet.xp < level_cutoffs[pet.level])
		pet.xp = save->pet.xp;

	if(save->pet.vigour <= 12)
		pet.vigour = save->pet.vigour;
	if(save->pet.focus <= 12)
		pet.focus = save->pet.focus;
	if(save->pet.spirit <= 12)
		pet.spirit = save->pet.spirit;

	for(int i = 0; i < item_table.length; i++)
	{
		CAT_inventory_add(i, save->inventory.counts[i]);
	}

	for(int i = 0; i < 150; i++)
	{
		int prop_id = save->deco.props[i] - 1;
		CAT_item* prop = CAT_get_item(prop_id);
		if(prop == NULL)
			continue;

		int prop_idx = -1;
		if(prop->type != CAT_ITEM_TYPE_PROP)
		{
			CAT_inventory_add(prop_id, 1);
		}
		else
		{
			prop_idx = CAT_room_place_prop
			(
				save->deco.positions[i*2+0],
				save->deco.positions[i*2+1],
				prop_id
			);
			if(prop_idx == -1)
				CAT_inventory_add(prop_id, 1);
		}
		
		int child_id = save->deco.children[i] - 1;
		CAT_item* child = CAT_get_item(child_id);
		if(child == NULL)
			continue;
		if
		(
			child->type != CAT_ITEM_TYPE_PROP ||
			child->prop_type != CAT_PROP_TYPE_TOP ||
			prop_idx == -1 ||
			prop->prop_type != CAT_PROP_TYPE_BOTTOM
		)
		{
			CAT_inventory_add(child_id, 1);
		}
		else
		{
			CAT_room_stack_prop(prop_idx, child_id);
		}
	}

	snake_highscore = save->highscores.snake;
	mines_highscore = save->highscores.mines;
	foursquares_highscore = save->highscores.foursquares;

	CAT_set_save_flags(save->config.flags);
	if(save->config.theme < CAT_ROOM_THEME_COUNT)
		CAT_room_set_theme(CAT_room_theme_list[save->config.theme]);

	CAT_printf("Load complete!\n");
}