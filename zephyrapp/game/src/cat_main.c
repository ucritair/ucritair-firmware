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

#include "cat_version.h"
#include "theme_assets.h"
#include "config.h"

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
bool first_load = true;

uint8_t saved_version_major = CAT_VERSION_MAJOR;
uint8_t saved_version_minor = CAT_VERSION_MINOR;
uint8_t saved_version_patch = CAT_VERSION_PATCH;
uint8_t saved_version_push = CAT_VERSION_PUSH;

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

void CAT_save_failsafe()
{
	saved_version_major = CAT_VERSION_MAJOR;
	saved_version_minor = CAT_VERSION_MINOR;
	saved_version_patch = CAT_VERSION_PATCH;
	saved_version_push = CAT_VERSION_PUSH;

	CAT_item_list_add(&bag, prop_eth_farm_item, 1);
	coins = 10;
}

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
		save->prop_places[i] = room.prop_places[i];
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

	save->magic_number = CAT_SAVE_MAGIC;
	CAT_finish_save(save);
}

void CAT_force_load()
{
	CAT_save* save = CAT_start_load();

	if(!CAT_check_save(save) || save == NULL)
	{
		CAT_save_failsafe();
		CAT_finish_load();
		return;
	}
	first_load = false;

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

	if(pet.level < CAT_NUM_LEVELS)
		pet.level = save->level;
	if(save->xp <= level_cutoffs[pet.level])
		pet.xp = save->xp;

	CAT_finish_load();
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
}

void CAT_init(int seconds_slept)
{
	logged_sleep = seconds_slept;

	CAT_platform_init();
	CAT_input_init();

	CAT_timetable_init();

	CAT_anim_table_init();
	CAT_sprite_mass_define();
	CAT_spriter_init();

	CAT_space_init();
	CAT_room_init();
	CAT_pet_init();

	CAT_force_load();
	CAT_apply_sleep();
	
	CAT_pet_reanimate();
	CAT_pet_reposition();

	CAT_machine_transition(CAT_MS_room);
}

bool in_world()
{
	return
	machine == CAT_MS_room ||
	machine == CAT_MS_feed ||
	machine == CAT_MS_study ||
	machine == CAT_MS_play ||
	machine == CAT_MS_deco;
}

void CAT_tick_logic()
{
	CAT_platform_tick();
	CAT_input_tick();

	CAT_room_tick(in_world());
	CAT_pet_tick(in_world());

	CAT_machine_tick();

	if(CAT_gui_keyboard_is_open())
		CAT_gui_keyboard_io();
}

void CAT_tick_render(int cycle)
{
	if (cycle == 0)
	{
		draw_queue.length = 0;
	}

	if(in_world())
	{
		CAT_render_room(cycle);
		CAT_render_pet(cycle);
		if(machine == CAT_MS_deco)
			CAT_render_deco(cycle);
		else
			CAT_render_action(cycle);
		CAT_draw_queue_submit(cycle);
	}
	else if(machine == CAT_MS_menu)
		CAT_render_menu();
	else if(machine == CAT_MS_insights)
		CAT_render_insights();
	else if(machine == CAT_MS_bag)
		CAT_render_bag();
	else if(machine == CAT_MS_arcade)
		CAT_render_arcade();
	else if(machine == CAT_MS_snake)
		CAT_render_snake();
	else if(machine == CAT_MS_mines)
		CAT_render_mines();
	else if(machine == CAT_MS_crawl)
		CAT_render_crawl();
	else if(machine == CAT_MS_vending)
		CAT_render_vending();
	else if(machine == CAT_MS_manual)
		CAT_render_manual();
	else if(machine == CAT_MS_item_dialog)
		CAT_render_item_dialog();
	else if(machine == CAT_MS_inspector)
		CAT_render_inspector();
#ifdef CAT_EMBEDDED
	else if(machine == CAT_MS_time)
		CAT_render_time();
	else if(machine == CAT_MS_aqi)
		CAT_render_aqi();
	else if(machine == CAT_MS_system_menu)
		CAT_render_system_menu();
	else if(machine == CAT_MS_graph)
		CAT_render_graph();
#endif
	else if(machine == CAT_MS_debug)
		CAT_render_debug();	
	else if(machine == CAT_MS_hedron)
		CAT_render_hedron();
	else if(machine == CAT_MS_magic)
		CAT_render_magic();
	else
	{
		CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 20});
		CAT_gui_text("This machine state\nhas no render routine!");
	}

	if(CAT_gui_keyboard_is_open())
		CAT_gui_keyboard();
}

#ifdef CAT_DESKTOP
int main(int argc, char** argv)
{
	int sleep = 0;
	if (argc == 2)
		sleep = atoi(argv[1]);
	if (sleep == 0)
		sleep = CAT_load_sleep();

	CAT_init(sleep);

	while (CAT_get_battery_pct() > 0)
	{
		CAT_tick_logic();
		CAT_tick_render(0);
		CAT_LCD_post(spriter.framebuffer);
	}

	CAT_force_save();
	CAT_save_sleep();

	CAT_spriter_cleanup();
	CAT_platform_cleanup();
	return 0;
}
#endif