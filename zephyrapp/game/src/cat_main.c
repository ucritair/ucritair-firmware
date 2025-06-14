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
#include "caring/cat_actions.h"
#include "cat_menu.h"
#include "cat_bag.h"
#include "cat_arcade.h"
#include "cat_vending.h"
#include "cat_deco.h"
#include "cat_item_dialog.h"
#include "cat_aqi.h"
#include "cat_monitors.h"

#include "cat_version.h"
#include "theme_assets.h"
#include "config.h"
#include "sprite_assets.h"

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

const int eink_update_time_threshold = CAT_MIN_SECS;
float time_since_eink_update = 0.0f;
bool first_eink_update_complete = false;

CAT_screen_orientation current_orientation;
CAT_screen_orientation last_orientation;
float time_since_reorient = 0.0f;

void CAT_force_save()
{
	CAT_printf("Saving...\n");
	CAT_save* save = CAT_start_save();
	CAT_initialize_save(save);

	strcpy(save->pet.name, pet.name);
	save->pet.level = pet.level;
	save->pet.xp = pet.xp;
	save->pet.lifespan = 30;
	save->pet.lifetime = pet.lifetime;
	save->pet.vigour = pet.vigour;
	save->pet.focus = pet.focus;
	save->pet.spirit = pet.spirit;

	for(int i = 0; i < item_table.length; i++)
	{
		int bag_idx = CAT_item_list_find(&bag, i);
		if(bag_idx != -1)
			save->inventory.counts[i] = bag.counts[bag_idx];
	}
	for(int i = 0; i < room.pickup_count; i++)
	{
		if(room.pickups[i].sprite == &coin_world_sprite)
		{
			coins += 1;
		}
	}
	save->inventory.coins = coins;

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

	save->timing.stat_timer = CAT_timer_get(pet.stat_timer_id);
	save->timing.life_timer = CAT_timer_get(pet.life_timer_id);
	save->timing.earn_timer = CAT_timer_get(room.earn_timer_id);
	save->timing.petting_timer = CAT_timer_get(pet.petting_timer_id);
	save->timing.petting_count = pet.times_pet;
	save->timing.milking_count = pet.times_milked;

	save->config.flags = CAT_export_config_flags();
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
	CAT_room_init();
	CAT_pet_init();
	CAT_item_list_init(&bag);
	
	CAT_item_list_add(&bag, prop_eth_farm_item, 1);
	CAT_item_list_add(&bag, toy_laser_pointer_item, 1);
	coins = 10;
}

void CAT_load_turnkey()
{
	CAT_room_init();
	CAT_pet_init();
	CAT_item_list_init(&bag);

	CAT_room_add_prop(prop_plant_plain_item, (CAT_ivec2) {0, 0});
	CAT_room_add_prop(prop_eth_farm_item, (CAT_ivec2) {2, 0});
	CAT_room_add_prop(prop_table_mahogany_item, (CAT_ivec2) {3, 3});
	CAT_room_stack_prop(room.prop_count-1, prop_bowl_walnut_item);
	CAT_room_add_prop(prop_chair_mahogany_item, (CAT_ivec2) {1, 3});
	CAT_room_add_prop(prop_stool_wood_item, (CAT_ivec2) {7, 4});
	CAT_room_add_prop(prop_bush_plain_item, (CAT_ivec2) {13, 2});
	CAT_room_add_prop(prop_bush_plain_item, (CAT_ivec2) {13, 3});
	CAT_room_add_prop(prop_table_sm_plastic_item, (CAT_ivec2) {13, 7});
	CAT_room_stack_prop(room.prop_count-1, prop_coffeemaker_item);
	CAT_room_add_prop(prop_plant_daisy_item, (CAT_ivec2) {0, 9});

	CAT_item_list_add(&bag, book_1_item, 1);
	CAT_item_list_add(&bag, food_bread_item, 2);
	CAT_item_list_add(&bag, food_milk_item, 2);
	CAT_item_list_add(&bag, food_coffee_item, 1);
	CAT_item_list_add(&bag, prop_succulent_item, 1);
	CAT_item_list_add(&bag, toy_baseball_item, 1);
	CAT_item_list_add(&bag, toy_laser_pointer_item, 1);
	coins = 100;
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
			CAT_force_save();
			CAT_printf("Game state reset!\n");
			return;
		}
		else if(save_status == CAT_SAVE_ERROR_SECTOR_CORRUPT)
		{
			CAT_printf("Save requires migration...\n");
			CAT_migrate_legacy_save(save);
			CAT_set_config_flags(CAT_CONFIG_FLAG_MIGRATED);
			CAT_printf("Save migrated!\n");
		}
		else if(save_status == CAT_SAVE_ERROR_SECTOR_MISSING)
		{
			CAT_printf("Save is missing sector...\n");
			CAT_extend_save(save);
			CAT_printf("Save extended!\n");
		}
	}

	CAT_printf("Loading...\n");

	if(strlen(save->pet.name) <= CAT_TEXT_INPUT_MAX)
		strcpy(pet.name, save->pet.name);
	if(save->pet.level < CAT_NUM_LEVELS)
		pet.level = save->pet.level;
	if(save->pet.xp < level_cutoffs[pet.level])
		pet.xp = save->pet.xp;
	if(pet.lifetime <= 365)
		pet.lifetime = save->pet.lifetime;
	if(save->pet.vigour <= 12)
		pet.vigour = save->pet.vigour;
	if(save->pet.focus <= 12)
		pet.focus = save->pet.focus;
	if(save->pet.spirit <= 12)
		pet.spirit = save->pet.spirit;

	for(int i = 0; i < item_table.length; i++)
	{
		CAT_item_list_add(&bag, i, save->inventory.counts[i]);
	}
	coins = save->inventory.coins;

	for(int i = 0; i < 150; i++)
	{
		int prop_id = save->deco.props[i] - 1;
		CAT_item* prop = CAT_item_get(prop_id);
		if(prop == NULL)
			continue;
		int child_id = save->deco.children[i] - 1;
		CAT_item* child = CAT_item_get(child_id);

		CAT_ivec2 position =
		{
			save->deco.positions[i*2+0],
			save->deco.positions[i*2+1],
		};
		int prop_idx = CAT_room_add_prop(prop_id, position);

		if(prop_idx == -1)
		{
			CAT_item_list_add(&bag, prop_id, 1);
			if(child != NULL)
				CAT_item_list_add(&bag, child_id, 1);
		}
		else
		{
			int override = save->deco.overrides[i];
			if(override < prop->sprite->frame_count)
				room.prop_overrides[prop_idx] = override;
			
			if(child != NULL)
				CAT_room_stack_prop(prop_idx, child_id);
		}
	}

	snake_high_score = save->highscores.snake;

	CAT_timer_set(pet.stat_timer_id, save->timing.stat_timer);
	CAT_timer_set(pet.life_timer_id, save->timing.life_timer);
	CAT_timer_set(room.earn_timer_id, save->timing.earn_timer);
	CAT_timer_set(pet.petting_timer_id, save->timing.petting_timer);
	pet.times_pet = save->timing.petting_count;
	pet.times_milked = save->timing.milking_count;

	CAT_set_config_flags(save->config.flags);
	if(save->config.theme < THEME_COUNT)
		room.theme = themes_list[save->config.theme];

	CAT_printf("Load complete!\n");
}

void CAT_apply_sleep(int seconds)
{
	int stat_ticks = seconds / CAT_STAT_TICK_SECS;
	int stat_remainder = seconds % CAT_STAT_TICK_SECS;
	CAT_pet_stat(stat_ticks);
	CAT_timer_add(pet.stat_timer_id, stat_remainder);

	int life_ticks = seconds / CAT_LIFE_TICK_SECS;
	int life_remainder = seconds % CAT_LIFE_TICK_SECS;
	CAT_pet_life(life_ticks);
	CAT_timer_add(pet.life_timer_id, life_remainder);

	int earn_ticks = seconds / CAT_EARN_TICK_SECS;
	int earn_remainder = seconds % CAT_EARN_TICK_SECS;
	CAT_room_earn(earn_ticks);
	CAT_timer_add(room.earn_timer_id, earn_remainder);

	CAT_timer_add(pet.petting_timer_id, seconds);

	time_since_eink_update += seconds;
}

void CAT_init()
{
	CAT_platform_init();
	CAT_input_init();
	CAT_sound_power(true);

	CAT_timetable_init();
	CAT_animator_init();
	CAT_rand_seed();

	CAT_room_init();
	CAT_pet_init();

	CAT_force_load();
	CAT_apply_sleep(CAT_get_slept_s());

	if(CAT_check_config_flags(CAT_CONFIG_FLAG_AQ_FIRST))
		CAT_machine_transition(CAT_MS_monitor);
	else
		CAT_machine_transition(CAT_MS_room);

	if(CAT_is_AQ_initialized())
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

	CAT_animator_tick();

	CAT_room_tick();
	CAT_pet_tick();

	CAT_machine_tick();

	CAT_gui_io();

	time_since_eink_update += CAT_get_delta_time_s();
	if
	(
		(CAT_is_charging() &&
		time_since_eink_update >= eink_update_time_threshold &&
		CAT_input_time_since_last() >= eink_update_time_threshold) ||
		(!first_eink_update_complete && CAT_is_AQ_initialized())
	)
	{
		CAT_set_eink_update_flag(true);
	}

	time_since_reorient += CAT_get_delta_time_s();
	last_orientation = current_orientation;
	current_orientation = CAT_IMU_is_upside_down() ? CAT_SCREEN_ORIENTATION_DOWN : CAT_SCREEN_ORIENTATION_UP;
	if(current_orientation != last_orientation && time_since_reorient >= 1.0f)
	{
		CAT_set_screen_orientation(current_orientation);
		time_since_reorient = 0;

		CAT_set_eink_update_flag(true);
	}
}

void CAT_tick_render()
{
	if (CAT_get_render_cycle() == 0)
			CAT_draw_queue_clear();

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
		CAT_set_draw_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
		CAT_draw_sprite(&null_sprite, 0, 120, 160);
	}

	CAT_draw_queue_submit();
	CAT_gui_render();

	if(CAT_eink_needs_update())
	{
		CAT_draw_sprite(&eink_refresh_splash_sprite, 0, 0, 0);
		first_eink_update_complete = true;
	}
}

#ifdef CAT_DESKTOP

void aq_spoof()
{
	readings.lps22hh.uptime_last_updated = 0;
	readings.lps22hh.temp = 20;
	readings.lps22hh.pressure = 1013;

	readings.sunrise.uptime_last_updated = 0;
	readings.sunrise.ppm_filtered_compensated = 400;
	readings.sunrise.ppm_filtered_uncompensated = 400;
	readings.sunrise.temp = 20;

	readings.sen5x.uptime_last_updated = 0;
	readings.sen5x.pm2_5 = 9;
	readings.sen5x.pm10_0 = 15;
	readings.sen5x.humidity_rhpct = 40;

	readings.sen5x.temp_degC = 20;
	readings.sen5x.voc_index = 1;
	readings.sen5x.nox_index = 100;
}

int main(int argc, char** argv)
{
	aq_spoof();

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
			time_since_eink_update = 0;
		}

		// 1 / FPS * 1_000_000 yields microseconds between frames at fixed FPS
		usleep((1.0f / (float) 14) * 1000000);
	}

	CAT_force_save();

	CAT_platform_cleanup();
	return 0;
}
#endif