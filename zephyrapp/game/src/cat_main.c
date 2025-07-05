#include "cat_main.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <stddef.h>

#include "cat_core.h"
#include "cat_render.h"
#include "cat_math.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_machine.h"
#include "cat_room.h"
#include "cat_pet.h"
#include "cat_actions.h"
#include "cat_menu.h"
#include "cat_arcade.h"
#include "cat_deco.h"
#include "cat_aqi.h"
#include "cat_monitors.h"
#include "cat_crisis.h"
#include "cat_version.h"
#include "theme_assets.h"
#include "config.h"
#include "sprite_assets.h"
#include "item_assets.h"

#ifdef CAT_EMBEDDED
#include "menu_time.h"
#include "menu_aqi.h"
#include "menu_system.h"
#include "menu_graph.h"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

uint64_t last_eink_time;
bool first_eink_update_complete = false;

CAT_screen_orientation current_orientation;
CAT_screen_orientation last_orientation;
uint64_t last_reorient_time;

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

	for(int i = 0; i < room.pickup_count; i++)
	{
		room.pickups[i].proc();
	}
	for(int i = 0; i < item_table.length; i++)
	{
		save->inventory.counts[i] = item_table.counts[i];
	}

	for(int i = 0; i < room.prop_count; i++)
	{
		save->deco.props[i] = room.prop_ids[i] + 1;
		save->deco.positions[i*2+0] = room.prop_rects[i].min.x;
		save->deco.positions[i*2+1] = room.prop_rects[i].min.y;
		save->deco.overrides[i] = room.prop_overrides[i];
		save->deco.children[i] = room.prop_children[i] + 1;
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
		if(themes_list[i] == room.theme)
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
	CAT_room_add_prop(prop_eth_farm_item, (CAT_ivec2) {2, 0});
	CAT_room_add_prop(prop_table_mahogany_item, (CAT_ivec2) {6, 3});
	CAT_room_add_prop(prop_chair_mahogany_item, (CAT_ivec2) {4, 3});
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

	CAT_room_init();
	CAT_room_add_prop(prop_plant_plain_item, (CAT_ivec2) {0, 0});
	CAT_room_add_prop(prop_eth_farm_item, (CAT_ivec2) {2, 0});
	CAT_room_add_prop(prop_table_mahogany_item, (CAT_ivec2) {3, 3});
	CAT_room_stack_prop(room.prop_count-1, prop_xen_crystal_item);
	CAT_room_add_prop(prop_chair_mahogany_item, (CAT_ivec2) {1, 3});
	CAT_room_add_prop(prop_stool_wood_item, (CAT_ivec2) {7, 4});
	CAT_room_add_prop(prop_bush_plain_item, (CAT_ivec2) {13, 2});
	CAT_room_add_prop(prop_bush_plain_item, (CAT_ivec2) {13, 3});
	CAT_room_add_prop(prop_table_sm_plastic_item, (CAT_ivec2) {13, 7});
	CAT_room_stack_prop(room.prop_count-1, prop_coffeemaker_item);
	CAT_room_add_prop(prop_plant_daisy_item, (CAT_ivec2) {0, 9});

	CAT_inventory_add(prop_portal_orange_item, 1);
	CAT_inventory_add(prop_portal_blue_item, 1);
	CAT_inventory_add(prop_hoopy_item, 1);
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
			CAT_ivec2 position =
			{
				save->deco.positions[i*2+0],
				save->deco.positions[i*2+1],
			};
			prop_idx = CAT_room_add_prop(prop_id, position);
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
		room.theme = themes_list[save->config.theme];

	CAT_printf("Load complete!\n");
}

void CAT_init()
{
	CAT_platform_init();
	CAT_input_init();

	CAT_rand_seed();
	CAT_animator_init();

	CAT_pet_init();
	CAT_room_init();

	CAT_force_load();

	if(CAT_check_config_flags(CAT_CONFIG_FLAG_AQ_FIRST))
		CAT_machine_transition(CAT_MS_monitor);
	else
		CAT_machine_transition(CAT_MS_room);

	if(CAT_AQ_sensors_initialized())
		CAT_set_eink_update_flag(true);
}

void CAT_tick_logic()
{	
	if(CAT_check_load_flags(CAT_LOAD_FLAG_DIRTY))
	{
		CAT_force_load();
		CAT_unset_load_flags(CAT_LOAD_FLAG_DIRTY);
	}
		
	CAT_platform_tick();
	CAT_input_tick();
	CAT_IMU_tick();

	CAT_AQ_tick();
	CAT_AQ_crisis_tick();

	CAT_animator_tick();

	CAT_room_tick();
	CAT_pet_tick();
	
	CAT_machine_tick();

	CAT_gui_io();

	uint64_t now = CAT_get_RTC_now();

	if
	(
		(CAT_is_charging() &&
		(now - last_eink_time) >= EINK_UPDATE_PERIOD &&
		CAT_input_time_since_last() >= EINK_UPDATE_PERIOD) ||
		(!first_eink_update_complete && CAT_AQ_sensors_initialized())
	)
	{
		CAT_set_eink_update_flag(true);
	}

	last_orientation = current_orientation;
	current_orientation = CAT_IMU_is_upside_down() ? CAT_SCREEN_ORIENTATION_DOWN : CAT_SCREEN_ORIENTATION_UP;
	if(current_orientation != last_orientation && (now - last_reorient_time) >= 1)
	{
		CAT_set_screen_orientation(current_orientation);
		last_reorient_time = now;

		CAT_set_eink_update_flag(true);
	}
}

void CAT_draw_eink_refresh_notice()
{
	CAT_draw_background(&eink_update_splash_sprite, 0, 0);
}

void CAT_tick_render()
{
	CAT_render_callback render_callback = CAT_get_render_callback();
	if(render_callback != NULL)
	{
		render_callback();
	}
	else
	{
		/*
		Effortlessly at height hangs his still eye.
		His wings hold all creation in a weightless quiet,
		steady as a hallucination in the streaming air.
		*/
		CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
		CAT_draw_sprite(&null_sprite, 0, 120, 160);
	}

	CAT_gui_render();

	if(CAT_eink_needs_update())
	{
		CAT_draw_eink_refresh_notice();
		first_eink_update_complete = true;
	}
}

#ifdef CAT_DESKTOP
int main(int argc, char** argv)
{
	CAT_init();
	
	while (CAT_get_battery_pct() > 0)
	{
		CAT_tick_logic();
		
		for(int render_cycle = 0; render_cycle < CAT_LCD_FRAMEBUFFER_SEGMENTS; render_cycle++)
		{
			CAT_set_render_cycle(render_cycle);
			CAT_tick_render();
			CAT_LCD_post();
			CAT_LCD_flip();
		}

		if(CAT_eink_needs_update())
		{
			CAT_set_eink_update_flag(false);
			CAT_eink_update();
			last_eink_time = CAT_get_RTC_now();
		}

		// 1 / FPS * 1_000_000 yields microseconds between framebuffer updates at fixed FPS
		usleep((1.0f / 14.0f) * 1000000);
	}

	CAT_force_save();
	CAT_platform_cleanup();
	return 0;
}
#endif