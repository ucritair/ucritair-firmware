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

uint8_t saved_version_major = CAT_VERSION_MAJOR;
uint8_t saved_version_minor = CAT_VERSION_MINOR;
uint8_t saved_version_patch = CAT_VERSION_PATCH;
uint8_t saved_version_push = CAT_VERSION_PUSH;

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

	save->version_major = CAT_VERSION_MAJOR;
	save->version_minor = CAT_VERSION_MINOR;
	save->version_patch = CAT_VERSION_PATCH;
	save->version_push = CAT_VERSION_PUSH;
	
	save->vigour = pet.vigour;
	save->focus = pet.focus;
	save->spirit = pet.spirit;
	save->lifetime = pet.lifetime;

	for(int i = 0; i < room.prop_count; i++)
	{
		save->prop_ids[i] = room.prop_ids[i];
		save->prop_places[i] = room.prop_rects[i].min;
		save->prop_overrides[i] = room.prop_overrides[i];
		save->prop_children[i] = room.prop_children[i];
	}
	save->prop_count = room.prop_count;

	for(int i = 0; i < bag.length; i++)
	{
		save->bag_ids[i] = bag.item_ids[i];
		save->bag_counts[i] = bag.counts[i];
	}
	save->bag_length = bag.length;
	save->coins = coins;

	for(int i = 0; i < room.pickup_count; i++)
	{
		if(room.pickups[i].sprite == &coin_world_sprite)
		{
			save->coins += 1;
		}
	}

	save->snake_high_score = snake_high_score;

	save->stat_timer = CAT_timer_get(pet.stat_timer_id);
	save->life_timer = CAT_timer_get(pet.life_timer_id);
	save->earn_timer = CAT_timer_get(room.earn_timer_id);
	save->times_pet = pet.times_pet;
	save->petting_timer = CAT_timer_get(pet.petting_timer_id);
	save->times_milked = pet.times_milked;

	strcpy(save->name, pet.name);

	for(int i = 0; i < THEME_COUNT; i++)
	{
		if(themes_list[i] == room.theme)
		{
			save->theme = i;
			break;
		}
	}

	save->level = pet.level;
	save->xp = pet.xp;

	save->lcd_brightness = CAT_LCD_get_brightness();
	save->led_brightness = CAT_LED_get_brightness();

	save->temperature_unit = CAT_AQ_get_temperature_unit();

	save->save_flags = CAT_export_save_flags();

	save->magic_number = CAT_SAVE_MAGIC;
	CAT_finish_save(save);

	CAT_printf("Save complete!\n");
}

void CAT_load_reset()
{
	saved_version_major = CAT_VERSION_MAJOR;
	saved_version_minor = CAT_VERSION_MINOR;
	saved_version_patch = CAT_VERSION_PATCH;
	saved_version_push = CAT_VERSION_PUSH;

	pet.vigour = 12;
	pet.focus = 12;
	pet.spirit = 12;

	CAT_item_list_init(&bag);
	CAT_item_list_add(&bag, prop_eth_farm_item, 1);
	CAT_item_list_add(&bag, toy_laser_pointer_item, 1);
	coins = 10;

	CAT_room_init();
}

void CAT_load_override()
{
	saved_version_major = CAT_VERSION_MAJOR;
	saved_version_minor = CAT_VERSION_MINOR;
	saved_version_patch = CAT_VERSION_PATCH;
	saved_version_push = CAT_VERSION_PUSH;

	pet.vigour = 9;
	pet.focus = 9;
	pet.spirit = 9;

	CAT_item_list_init(&bag);
	CAT_item_list_add(&bag, book_1_item, 1);
	CAT_item_list_add(&bag, food_bread_item, 2);
	CAT_item_list_add(&bag, food_milk_item, 2);
	CAT_item_list_add(&bag, food_coffee_item, 1);
	CAT_item_list_add(&bag, prop_succulent_item, 1);
	CAT_item_list_add(&bag, toy_baseball_item, 1);
	CAT_item_list_add(&bag, toy_laser_pointer_item, 1);
	coins = 100;

	CAT_room_init();
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
}

void CAT_force_load()
{
	CAT_printf("Loading...\n");
	
	CAT_save* save = CAT_start_load();

	if(save == NULL || !CAT_check_save(save) || CAT_check_load_flags(CAT_LOAD_FLAG_RESET))
	{
		CAT_load_reset();
		CAT_unset_load_flags(CAT_LOAD_FLAG_RESET);

		CAT_finish_load();
		CAT_force_save();

		CAT_printf("Refreshed save state!\n");
		return;
	}
	
	saved_version_major = save->version_major;
	saved_version_minor = save->version_minor;
	saved_version_patch = save->version_patch;
	saved_version_push = save->version_push;

	if(save->vigour <= 12)
		pet.vigour = save->vigour;
	if(save->focus <= 12)
		pet.focus = save->focus;
	if(save->spirit <= 12)
		pet.spirit = save->spirit;
	pet.lifetime = save->lifetime;
		
	for(int i = 0; i < save->prop_count; i++)
	{
		uint8_t prop_id = save->prop_ids[i];
		CAT_ivec2 prop_place = save->prop_places[i];
		if(CAT_prop_fits(prop_id, prop_place))
		{
			CAT_room_add_prop(prop_id, prop_place);
			room.prop_overrides[i] = save->prop_overrides[i];
			room.prop_children[i] = save->prop_children[i];
		}
		else
		{
			CAT_item_list_add(&bag, save->prop_ids[i], 1);
		}
	}

	for(int i = 0; i < save->bag_length; i++)
	{	
		if(CAT_item_get(save->bag_ids[i]) != NULL)
			CAT_item_list_add(&bag, save->bag_ids[i], save->bag_counts[i]);
	}
	coins = save->coins;

	snake_high_score = save->snake_high_score;

	CAT_timer_set(room.earn_timer_id, save->earn_timer);
	CAT_timer_set(pet.stat_timer_id, save->stat_timer);
	CAT_timer_set(pet.life_timer_id, save->life_timer);
	pet.times_pet = save->times_pet;
	CAT_timer_set(pet.petting_timer_id, save->petting_timer);
	pet.times_milked = save->times_milked;

	if(strlen(save->name) <= CAT_TEXT_INPUT_MAX)
		strcpy(pet.name, save->name);

	if(save->theme < THEME_COUNT)
		room.theme = themes_list[save->theme];

	if(save->level < CAT_NUM_LEVELS)
		pet.level = save->level;
	if(save->xp <= level_cutoffs[pet.level])
		pet.xp = save->xp;

	CAT_import_save_flags(save->save_flags);
	
	if(save->lcd_brightness >= CAT_LCD_MIN_BRIGHTNESS && save->lcd_brightness <= CAT_LCD_MAX_BRIGHTNESS)
		CAT_LCD_set_brightness(save->lcd_brightness);
	CAT_LED_set_brightness(save->led_brightness);
	
	if(save->temperature_unit <= CAT_TEMPERATURE_UNIT_DEGREES_FAHRENHEIT)
		CAT_AQ_set_temperature_unit(save->temperature_unit);
	
	if(CAT_check_load_flags(CAT_LOAD_FLAG_OVERRIDE))
	{
		CAT_load_override();
		CAT_unset_load_flags(CAT_LOAD_FLAG_OVERRIDE);
	}
		
	CAT_finish_load();

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

	if(CAT_check_save_flags(CAT_SAVE_FLAG_AQ_FIRST))
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