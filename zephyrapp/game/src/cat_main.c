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
#include "cat_bag.h"
#include "cat_arcade.h"
#include "cat_vending.h"
#include "cat_deco.h"
#include "cat_item_dialog.h"
#include "cat_aqi.h"

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

int logged_sleep = 0;
bool needs_load = true;
bool override_load = false;
bool clear_load = false;

uint8_t saved_version_major = CAT_VERSION_MAJOR;
uint8_t saved_version_minor = CAT_VERSION_MINOR;
uint8_t saved_version_patch = CAT_VERSION_PATCH;
uint8_t saved_version_push = CAT_VERSION_PUSH;

float time_since_last_eink_update = 0.0f;
const int eink_update_time_threshold = CAT_MIN_SECS;

CAT_screen_orientation current_orientation;
CAT_screen_orientation last_orientation;
float time_since_last_flip = 0.0f;

#ifdef CAT_DESKTOP
int CAT_load_sleep()
{
	time_t sleep_time;
	int fd = open("sleep.dat", O_RDONLY);
	if(fd != -1)
	{
		read(fd, &sleep_time, sizeof(sleep_time));
		close(fd);
		time_t now;
		time(&now);
		return difftime(now, sleep_time);
	}
	return 0;
}

void CAT_save_sleep()
{
	time_t now;
	time(&now);
	int fd = open("sleep.dat", O_WRONLY | O_CREAT | O_TRUNC,  S_IRUSR | S_IWUSR);
	write(fd, &now, sizeof(now));
	close(fd);
}
#endif

void CAT_force_save()
{
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

	save->magic_number = CAT_SAVE_MAGIC;
	CAT_finish_save(save);
}

void CAT_load_failsafe()
{
	saved_version_major = CAT_VERSION_MAJOR;
	saved_version_minor = CAT_VERSION_MINOR;
	saved_version_patch = CAT_VERSION_PATCH;
	saved_version_push = CAT_VERSION_PUSH;

	pet.vigour = 9;
	pet.focus = 9;
	pet.spirit = 9;

	CAT_item_list_init(&bag);
	CAT_item_list_add(&bag, prop_eth_farm_item, 1);
	coins = 10;

	CAT_space_init();
	CAT_room_init();

	CAT_LCD_set_brightness(CAT_LCD_MAX_BRIGHTNESS);
	CAT_LED_set_brightness(100);
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
	coins = 100;

	CAT_space_init();
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
	CAT_save* save = CAT_start_load();

	if(clear_load || !CAT_check_save(save) || save == NULL)
	{
		CAT_load_failsafe();
		CAT_finish_load();
		needs_load = false;
		clear_load = false;
		CAT_force_save();
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
	
	if(save->lcd_brightness >= CAT_LCD_MIN_BRIGHTNESS && save->lcd_brightness <= CAT_LCD_MAX_BRIGHTNESS)
		CAT_LCD_set_brightness(save->lcd_brightness);
	CAT_LED_set_brightness(save->led_brightness);
	
	if(save->temperature_unit <= CAT_TEMPERATURE_UNIT_DEGREES_FAHRENHEIT)
		CAT_AQ_set_temperature_unit(save->temperature_unit);
	
	if(override_load)
		CAT_load_override();

	CAT_finish_load();
	needs_load = false;
}

void CAT_apply_sleep()
{
	int stat_ticks = logged_sleep / CAT_STAT_TICK_SECS;
	int stat_remainder = logged_sleep % CAT_STAT_TICK_SECS;
	CAT_pet_stat(stat_ticks);
	CAT_timer_add(pet.stat_timer_id, stat_remainder);

	int life_ticks = logged_sleep / CAT_LIFE_TICK_SECS;
	int life_remainder = logged_sleep % CAT_LIFE_TICK_SECS;
	CAT_pet_life(life_ticks);
	CAT_timer_add(pet.life_timer_id, life_remainder);

	int earn_ticks = logged_sleep / CAT_EARN_TICK_SECS;
	int earn_remainder = logged_sleep % CAT_EARN_TICK_SECS;
	CAT_room_earn(earn_ticks);
	CAT_timer_add(room.earn_timer_id, earn_remainder);

	CAT_timer_add(pet.petting_timer_id, logged_sleep);

	time_since_last_eink_update += logged_sleep;
}

void CAT_init(int seconds_slept)
{
	logged_sleep = seconds_slept;

	CAT_platform_init();
	CAT_input_init();

	CAT_timetable_init();

	CAT_animator_init();

	CAT_space_init();
	CAT_room_init();
	CAT_pet_init();

	CAT_force_load();
	CAT_apply_sleep();
	
	CAT_pet_reposition();

	CAT_machine_transition(CAT_MS_room);

	CAT_eink_update();
}

void CAT_tick_logic()
{
	if(needs_load)
		CAT_force_load();

	CAT_platform_tick();
	CAT_input_tick();
	CAT_get_AQ_readings();
	CAT_IMU_tick();

	CAT_animator_tick();

	CAT_room_tick();
	CAT_pet_tick();

	CAT_machine_tick();

	CAT_gui_io();

	time_since_last_eink_update += CAT_get_delta_time();
	if
	(
		CAT_is_charging() &&
		time_since_last_eink_update >= eink_update_time_threshold &&
		CAT_input_time_since_last() >= eink_update_time_threshold
	)
	{
		CAT_eink_update();
		time_since_last_eink_update = 0;
	}

	last_orientation = current_orientation;
	current_orientation = CAT_IMU_is_upside_down() ? CAT_SCREEN_ORIENTATION_DOWN : CAT_SCREEN_ORIENTATION_UP;
	if(current_orientation != last_orientation && time_since_last_flip >= 1.0f)
	{
		CAT_set_screen_orientation(current_orientation);
		if(time_since_last_eink_update > 10)
		{
			CAT_eink_update();
			time_since_last_eink_update = 0;
		}
		time_since_last_flip = 0;
	}
	else
		time_since_last_flip += CAT_get_delta_time();
}

void CAT_tick_render()
{
	if (CAT_get_render_cycle() == 0)
		CAT_draw_queue_clear();

	if(CAT_get_render_callback() != NULL)
	{
		(CAT_get_render_callback())();
	}
	else
	{
		/*
		Effortlessly at height hangs his still eye.
		His wings hold all creation in a weightless quiet,
		steady as a hallucination in the streaming air.
		*/
		CAT_draw_sprite(&null_sprite, 0, 120-12, 160-12);
	}

	CAT_gui_render();
	
	CAT_draw_queue_submit();
}

#ifdef CAT_DESKTOP
int main(int argc, char** argv)
{
	CAT_init(CAT_load_sleep());

	while (CAT_get_battery_pct() > 0)
	{
		CAT_tick_logic();
		
		for(int render_cycle = 0; render_cycle < CAT_LCD_FRAMEBUFFER_SEGMENTS; render_cycle++)
		{
			CAT_set_render_cycle(render_cycle);
			CAT_tick_render();
			CAT_LCD_post();
		}
		CAT_LCD_flip();

		// 1 / FPS * 1_000_000 yields microseconds between frames at fixed FPS
		usleep((1.0f / (float) 12) * 1000000);
	}

	CAT_force_save();
	CAT_save_sleep();

	CAT_platform_cleanup();
	return 0;
}
#endif