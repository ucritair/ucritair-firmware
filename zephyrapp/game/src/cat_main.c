#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cat_core.h"
#include "cat_sprite.h"
#include "cat_math.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_machine.h"

#include "cat_room.h"
#include "cat_pet.h"
#include "cat_actions.h"
#include "cat_menu.h"
#include "cat_stats.h"
#include "cat_bag.h"
#include "cat_arcade.h"
#include "cat_vending.h"
#include "cat_manual.h"
#include "cat_deco.h"

#include "cat_version.h"

#ifdef CAT_EMBEDDED
#include "menu_time.h"
#include "menu_aqi.h"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

void CAT_force_save()
{
	CAT_save* save = CAT_start_save();

	save->version.major = CAT_VERSION_MAJOR;
	save->version.minor = CAT_VERSION_MINOR;
	save->version.patch = CAT_VERSION_PATCH;
	save->version.push = CAT_VERSION_PUSH;

	save->vigour = pet.vigour;
	save->focus = pet.focus;
	save->spirit = pet.spirit;

	for(int i = 0; i < room.prop_count; i++)
	{
		save->prop_ids[i] = room.prop_ids[i];
		save->prop_places[i] = room.prop_places[i];
		save->prop_overrides[i] = room.prop_overrides[i];
	}
	save->prop_count = room.prop_count;

	for(int i = 0; i < bag.length; i++)
	{
		save->bag_ids[i] = bag.item_ids[i];
		save->bag_counts[i] = bag.counts[i];
	}
	save->bag_length = bag.length;
	save->coins = bag.coins;

	CAT_finish_save(save);
}

void CAT_force_load()
{
	CAT_save* save = CAT_start_load();

	pet.vigour = save->vigour;
	pet.focus = save->focus;
	pet.spirit = save->spirit;

	for(int i = 0; i < save->prop_count; i++)
	{
		room.prop_ids[i] = save->prop_ids[i];
		room.prop_places[i] = save->prop_places[i];
		room.prop_overrides[i] = save->prop_overrides[i];
	}
	room.prop_count = save->prop_count;

	for(int i = 0; i < save->bag_length; i++)
	{
		bag.item_ids[i] = save->bag_ids[i];
		bag.counts[i] = save->bag_counts[i];
	}
	bag.length = save->bag_length;
	bag.coins = save->coins;

	CAT_finish_load();
}

void CAT_init(bool is_first_boot, int sceonds_slept)
{
	CAT_rand_init();
	CAT_platform_init();
	CAT_input_init();

	CAT_atlas_init();
	CAT_sprite_mass_define();

	CAT_spriter_init();
	CAT_draw_queue_init();
	
	CAT_item_table_init();
	CAT_item_mass_define();

	CAT_timetable_init();

	CAT_pet_init();
	CAT_room_init();
	CAT_bag_init();
	CAT_deco_state_init();

#ifdef CAT_DESKTOP
	CAT_force_load();
#endif
	
	machine = NULL;
	CAT_machine_transition(&machine, CAT_MS_room);
}

void CAT_tick_logic()
{
	CAT_platform_tick();
	CAT_AQI_tick();
	CAT_input_tick();

	CAT_machine_tick(&machine);
}

void CAT_tick_render(int cycle)
{
	if (cycle == 0)
	{
		draw_queue.length = 0;
	}

	if(machine == CAT_MS_room)
	{
		CAT_render_room(cycle);
		CAT_render_pet(cycle);
		CAT_draw_queue_submit(cycle);
	}
	else if
	(
		machine == CAT_MS_feed ||
		machine == CAT_MS_study ||
		machine == CAT_MS_play
	)
	{
		CAT_render_room(cycle);
		CAT_render_pet(cycle);
		CAT_render_action(cycle);
		CAT_draw_queue_submit(cycle);
	}
	else if(machine == CAT_MS_deco)
	{
		CAT_render_room(cycle);
		CAT_render_pet(cycle);
		CAT_render_deco(cycle);
		CAT_draw_queue_submit(cycle);
	}
	else if(machine == CAT_MS_menu)
		CAT_render_menu();
	else if(machine == CAT_MS_stats)
		CAT_render_stats();
	else if(machine == CAT_MS_bag)
		CAT_render_bag();
	else if(machine == CAT_MS_arcade)
		CAT_render_arcade();
	else if(machine == CAT_MS_vending)
		CAT_render_vending();
	else if(machine == CAT_MS_manual)
		CAT_render_manual();
#ifdef CAT_EMBEDDED
	else if(machine == CAT_MS_time)
		CAT_render_time();
	else if(machine == CAT_MS_aqi)
		CAT_render_aqi();
#endif
}

#ifdef CAT_DESKTOP
#include <sys/stat.h>

int main()
{
	struct stat buf;
	bool first_boot = stat("save.dat", &buf) == 0;

	CAT_init(first_boot, 12);

	while (CAT_get_battery_pct() > 0)
	{
		CAT_tick_logic();
		CAT_tick_render(0);
		CAT_LCD_post(spriter.framebuffer);
	}

#ifdef CAT_DESKTOP
	CAT_force_save();
#endif

	CAT_spriter_cleanup();
#ifndef CAT_BAKED_ASSETS
	CAT_atlas_cleanup();
#endif
	CAT_platform_cleanup();
	return 0;
}
#endif