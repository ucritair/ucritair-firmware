#include "cat_main.h"

#include "cat_core.h"
#include <string.h>
#include "cat_pet.h"
#include "cat_room.h"
#include "item_assets.h"
#include "cat_item.h"
#include "cat_arcade.h"
#include "theme_assets.h"
#include "cat_crisis.h"

void persist_save()
{
	CAT_printf("Persist save!\n");
	CAT_AQ_export_crisis_state(CAT_AQ_crisis_state_persist());
	CAT_pet_export_timing_state(CAT_pet_timing_state_persist());
}

void CAT_force_save()
{
	persist_save();
	
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

	save->highscores.snake = snake_high_score;
	save->highscores.mine = 0;
	save->highscores.foursquares = 0;

	if(CAT_AQ_get_temperature_unit() == CAT_TEMPERATURE_UNIT_DEGREES_FAHRENHEIT)
		CAT_raise_config_flags(CAT_CONFIG_FLAG_USE_FAHRENHEIT);
	else
		CAT_lower_config_flags(CAT_CONFIG_FLAG_USE_FAHRENHEIT);
	save->config.flags = CAT_get_config_flags();

	for(int i = 0; i < THEME_COUNT; i++)
	{
		if(themes_list[i] == CAT_room_get_theme())
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
	CAT_room_place_prop(2, 0, prop_eth_farm_item);
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
	CAT_room_place_prop(2, 0, prop_eth_farm_item);
	CAT_room_place_prop(3, 3, prop_table_mahogany_item);
	CAT_room_stack_prop(CAT_room_get_props()->length-1, prop_xen_crystal_item);
	CAT_room_place_prop(1, 3, prop_chair_mahogany_item);
	CAT_room_place_prop(7, 4, prop_stool_wood_item);
	CAT_room_place_prop(13, 2, prop_bush_plain_item);
	CAT_room_place_prop(13, 3, prop_bush_plain_item);
	CAT_room_place_prop(13, 7, prop_table_sm_plastic_item);
	CAT_room_stack_prop(CAT_room_get_props()->length-1, prop_coffeemaker_item);
	CAT_room_place_prop(0, 9, prop_plant_daisy_item);
}

void persist_load()
{
	CAT_printf("Persist load!\n");
	CAT_AQ_import_crisis_state(CAT_AQ_crisis_state_persist());
	CAT_pet_import_timing_state(CAT_pet_timing_state_persist());
}

void CAT_force_load()
{
	persist_load();
	
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
			CAT_raise_config_flags(CAT_CONFIG_FLAG_MIGRATED);
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

	if(strlen(save->pet.name) <= CAT_TEXT_INPUT_MAX_LENGTH)
		strncpy(pet.name, save->pet.name, sizeof(pet.name));

	if(save->pet.lifespan <= 30)
		pet.lifespan = save->pet.lifespan;
	pet.lifetime = min(save->pet.lifetime, 31);
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
		CAT_item* prop = CAT_item_get(prop_id);
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
		CAT_item* child = CAT_item_get(child_id);
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

	snake_high_score = save->highscores.snake;

	CAT_set_config_flags(save->config.flags);
	if(save->config.flags & CAT_CONFIG_FLAG_USE_FAHRENHEIT)
		CAT_AQ_set_temperature_unit(CAT_TEMPERATURE_UNIT_DEGREES_FAHRENHEIT);
	else
		CAT_AQ_set_temperature_unit(CAT_TEMPERATURE_UNIT_DEGREES_CELSIUS);
	if(save->config.theme < THEME_COUNT)
		CAT_room_set_theme(themes_list[save->config.theme]);

	CAT_printf("Load complete!\n");
}