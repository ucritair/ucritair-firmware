#include "cat_menu.h"

#include "cat_room.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_machine.h"
#include "cat_render.h"
#include "cat_version.h"
#include "cat_vending.h"
#include "cat_arcade.h"
#include "cat_bag.h"
#include "cat_pet.h"
#include <stddef.h>
#include <string.h>
#include "cat_main.h"
#include "theme_assets.h"

#ifdef CAT_EMBEDDED
#include "menu_system.h"
#include "menu_aqi.h"
#endif

void CAT_MS_menu(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
			
			if(CAT_gui_keyboard_is_open())
			{
				CAT_gui_keyboard_io();
				break;
			}

			if(CAT_gui_begin_menu("MENU"))
			{
				if(CAT_input_pressed(CAT_BUTTON_B))
					CAT_machine_back();

				if(CAT_gui_menu_item("INSIGHTS"))
					CAT_machine_transition(CAT_MS_insights);
				if(CAT_gui_begin_menu("SETTINGS"))
				{
					if(CAT_gui_menu_item("PET NAME"))
					{
						CAT_gui_open_keyboard(pet.name);
					}
					if(CAT_gui_begin_menu("ROOM THEME"))
					{
						if(CAT_gui_menu_item("BASIC THEME"))
							room.theme = &base_theme;
						if(CAT_gui_menu_item("VERDANT THEME"))
							room.theme = &grass_theme;
						if(CAT_gui_menu_item("ASHEN THEME"))
							room.theme = &ash_theme;
						CAT_gui_end_menu();
					}
					CAT_gui_end_menu();
				}
				if(CAT_gui_menu_item("INVENTORY"))
					CAT_machine_transition(CAT_MS_bag);
				if(CAT_gui_menu_item("VENDING MACHINE"))
					CAT_machine_transition(CAT_MS_vending);
				if(CAT_gui_menu_item("ARCADE"))
					CAT_machine_transition(CAT_MS_arcade);
				if(CAT_gui_menu_item("MAGIC"))
					CAT_machine_transition(CAT_MS_magic);
#ifdef CAT_EMBEDDED
				if(CAT_gui_menu_item("AIR QUALITY"))
					CAT_machine_transition(CAT_MS_aqi);
				if(CAT_gui_menu_item("SYSTEM DASHBOARD"))
					CAT_machine_transition(CAT_MS_system_menu);
#endif		
#ifdef CAT_DEBUG
				if(CAT_gui_menu_item("DEBUG DASHBOARD"))
					CAT_machine_transition(CAT_MS_debug);
				if(CAT_gui_begin_menu("CHEATS"))
				{
					if(CAT_gui_menu_item("+ 1000 COINS"))
						coins += 1000;
					if(CAT_gui_menu_item("BASE STATS"))
					{
						pet.vigour = 9;
						pet.focus = 9;
						pet.spirit = 9;
					}
					if(CAT_gui_menu_item("MAX STATS"))
					{
						pet.vigour = 12;
						pet.focus = 12;
						pet.spirit = 12;
					}
					if(CAT_gui_menu_item("CRIT STATS"))
					{
						pet.vigour = 3;
						pet.focus = 3;
						pet.spirit = 3;
					}
					if(CAT_gui_menu_item("+ EVERY ITEM"))
					{
						for(int item_id = 0; item_id < item_table.length; item_id++)
						{
							CAT_item_list_add(&bag, item_id, 1);
						}
					}
					if(CAT_gui_menu_item("TURNKEY APARTMENT"))
					{
						needs_load = true;
						override_load = true;
					}
					CAT_gui_end_menu();
				}
#endif
				if(CAT_gui_menu_item("MANUAL"))
					CAT_machine_transition(CAT_MS_manual);
				CAT_gui_end_menu();
			}

			if(CAT_gui_in_menu())
				CAT_gui_menu_io();
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_menu()
{
	if(CAT_gui_in_menu())
		CAT_gui_menu();

	if(CAT_gui_keyboard_is_open())
		CAT_gui_keyboard();
}
