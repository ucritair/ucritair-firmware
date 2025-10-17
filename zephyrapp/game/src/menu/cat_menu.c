#include "cat_menu.h"

#include "cat_room.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_machine.h"
#include "cat_render.h"
#include "cat_version.h"
#include "cat_item.h"
#include "cat_arcade.h"
#include "cat_item.h"
#include "cat_pet.h"
#include <stddef.h>
#include <string.h>
#include "cat_main.h"
#include "theme_assets.h"
#include "sound_assets.h"
#include "cat_monitors.h"
#include "item_assets.h"
#include "cat_world.h"
#include "cat_effects.h"
#include "cat_persist.h"

#ifdef CAT_EMBEDDED
#include "menu_system.h"
#endif

void CAT_MS_menu(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_menu);
			CAT_gui_begin_menu_context();
			break;
		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_pushdown_pop();

			if(CAT_gui_begin_menu("MENU"))
			{
				if(CAT_gui_menu_item("INSIGHTS"))
					CAT_pushdown_push(CAT_MS_insights);
				if(CAT_gui_menu_item("INVENTORY"))
					CAT_pushdown_push(CAT_MS_inventory);
				if(CAT_gui_menu_item("VENDING MACHINE"))
					CAT_pushdown_push(CAT_MS_shop);
				if(CAT_gui_menu_item("ARCADE"))
					CAT_pushdown_push(CAT_MS_arcade);
				if(CAT_pushdown_last() == &CAT_MS_room)
				{
					if(CAT_gui_menu_item("EXPLORE"))
						CAT_pushdown_rebase(CAT_MS_world);
				}
				else if(CAT_pushdown_last() == &CAT_MS_world)
				{
					if(CAT_gui_menu_item("GO HOME"))
						CAT_pushdown_rebase(CAT_MS_room);
				}
				if(CAT_gui_menu_item("DASHBOARD"))
					CAT_pushdown_rebase(CAT_MS_monitor);				
					
				if(CAT_gui_menu_item("MAGIC"))
					CAT_pushdown_push(CAT_MS_magic);
				if(CAT_check_config_flags(CAT_SAVE_CONFIG_FLAG_DEVELOPER))
				{
					if(CAT_gui_begin_menu("DEVELOPER"))
					{
						if(CAT_gui_menu_item("INFO"))
							CAT_pushdown_push(CAT_MS_debug);
							
						if(CAT_gui_begin_menu("CHEATS"))
						{
							if(CAT_gui_menu_item("1000 COINS"))
								CAT_inventory_add(coin_item, 1000);
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
							if(CAT_gui_menu_item("NULL STATS"))
							{
								pet.vigour = 0;
								pet.focus = 0;
								pet.spirit = 0;
							}
							if(CAT_gui_menu_item("KILL PET"))
							{
								pet.lifetime = 255;
							}
							if(CAT_gui_menu_item("EVERY ITEM"))
							{
								for(int item_id = 0; item_id < item_table.length; item_id++)
								{
									CAT_inventory_add(item_id, 1);
								}
							}
							if(CAT_gui_menu_item("TURNKEY APARTMENT"))
							{
								CAT_set_load_flags(CAT_LOAD_FLAG_DIRTY | CAT_LOAD_FLAG_TURNKEY);
							}
							CAT_gui_end_menu();
						}

						if(CAT_gui_menu_item("COLOUR PICKER"))
							CAT_pushdown_push(CAT_MS_colour_picker);
						
						if(CAT_gui_begin_menu("AQ CONTROL PANEL"))
						{
							if(CAT_gui_menu_item("EXTREME CO2"))
								readings.sunrise.ppm_filtered_compensated = 5000;
							if(CAT_gui_menu_item("MODERATE CO2"))
								readings.sunrise.ppm_filtered_compensated = 2000;
							if(CAT_gui_menu_item("MILD CO2"))
								readings.sunrise.ppm_filtered_compensated = 1000;
							if(CAT_gui_menu_item("NORMAL CO2"))
								readings.sunrise.ppm_filtered_compensated = 450;

							if(CAT_gui_menu_item("EXTREME TEMP"))
								readings.sen5x.temp_degC = 43;
							if(CAT_gui_menu_item("MODERATE TEMP"))
								readings.sen5x.temp_degC = 33;
							if(CAT_gui_menu_item("MILD TEMP"))
								readings.sen5x.temp_degC = 28;
							if(CAT_gui_menu_item("NORMAL TEMP"))
								readings.sen5x.temp_degC = 23;
							CAT_gui_end_menu();
						}

						if(CAT_gui_menu_item("LOW POWER E-INK"))
								CAT_eink_low_power();
							
						CAT_gui_end_menu();
					}				
				}
				if(CAT_gui_begin_menu("SETTINGS"))
				{
					if(CAT_gui_begin_menu("GAMEPLAY"))
					{
						if(CAT_gui_begin_menu("LAUNCH MODE"))
						{
							if(CAT_gui_menu_toggle("GAME FIRST", !(persist_flags & CAT_PERSIST_CONFIG_FLAG_AQ_FIRST), CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON))
								persist_flags &= ~CAT_PERSIST_CONFIG_FLAG_AQ_FIRST;
							if(CAT_gui_menu_toggle("DASHBOARD FIRST", persist_flags & CAT_PERSIST_CONFIG_FLAG_AQ_FIRST, CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON))
								persist_flags |= CAT_PERSIST_CONFIG_FLAG_AQ_FIRST;
							CAT_gui_end_menu();
						}
						if(CAT_gui_menu_toggle("PAUSE CRITTER CARE", persist_flags & CAT_PERSIST_CONFIG_FLAG_PAUSE_CARE, CAT_GUI_TOGGLE_STYLE_CHECKBOX))
						{
							if(persist_flags & CAT_PERSIST_CONFIG_FLAG_PAUSE_CARE)
								persist_flags &= ~CAT_PERSIST_CONFIG_FLAG_PAUSE_CARE;
							else
								persist_flags |= CAT_PERSIST_CONFIG_FLAG_PAUSE_CARE;
						}

						CAT_gui_end_menu();
					}
					if(CAT_gui_begin_menu("COSMETICS"))
					{
						if(CAT_gui_menu_item("PET NAME"))
						{
							CAT_gui_open_keyboard(pet.name);
						}
						if(CAT_gui_begin_menu("ROOM THEME"))
						{
							for(int i = 0; i < THEME_COUNT; i++)
							{		
								bool this_theme = CAT_room_get_theme() == themes_list[i];
								if(CAT_gui_menu_toggle(themes_list[i]->name, this_theme, CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON))
									CAT_room_set_theme(themes_list[i]);
							}
							CAT_gui_end_menu();
						}
						CAT_gui_end_menu();
					}
					if(CAT_gui_begin_menu("DISPLAY"))
					{
						screen_brightness = CAT_gui_menu_ticker("LCD BRIGHTNESS", screen_brightness, CAT_LCD_MIN_BRIGHTNESS, CAT_LCD_MAX_BRIGHTNESS);
						if(CAT_gui_menu_item("RESET LCD BRIGHTNESS"))
							screen_brightness = CAT_LCD_MAX_BRIGHTNESS;
						
						led_brightness = CAT_gui_menu_ticker("LED BRIGHTNESS", led_brightness, 0, 100);
						if(CAT_gui_menu_item("RESET LED BRIGHTNESS"))
							led_brightness = 100;

						if(CAT_gui_menu_item("REFRESH EINK"))
						{
							CAT_set_eink_update_flag(true);
						}

						if(CAT_gui_menu_toggle("AUTO-FLIP SCREEN", !(persist_flags & CAT_PERSIST_CONFIG_FLAG_MANUAL_ORIENT), CAT_GUI_TOGGLE_STYLE_CHECKBOX))
						{
							if(persist_flags & CAT_PERSIST_CONFIG_FLAG_MANUAL_ORIENT)
							{
								persist_flags &= ~CAT_PERSIST_CONFIG_FLAG_MANUAL_ORIENT;
							}
							else
							{
								persist_flags |= CAT_PERSIST_CONFIG_FLAG_MANUAL_ORIENT;
							}
						}	

						if(persist_flags & CAT_PERSIST_CONFIG_FLAG_MANUAL_ORIENT)
						{
							if(CAT_gui_menu_item("FLIP SCREEN"))
							{
								CAT_flip_screen();
								CAT_set_eink_update_flag(true);
							}
						}
					
						CAT_gui_end_menu();
					}
					if(CAT_gui_begin_menu("AIR QUALITY"))
					{
						if(CAT_gui_begin_menu("TEMPERATURE UNIT"))
						{
							if(CAT_gui_menu_toggle("CELSIUS", !(persist_flags & CAT_PERSIST_CONFIG_FLAG_USE_FAHRENHEIT), CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON))
								persist_flags &= ~CAT_PERSIST_CONFIG_FLAG_USE_FAHRENHEIT;
							if(CAT_gui_menu_toggle("FAHRENHEIT", persist_flags & CAT_PERSIST_CONFIG_FLAG_USE_FAHRENHEIT, CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON))
								persist_flags |= CAT_PERSIST_CONFIG_FLAG_USE_FAHRENHEIT;
							CAT_AQ_set_temperature_unit
							(
								persist_flags & CAT_PERSIST_CONFIG_FLAG_USE_FAHRENHEIT ?
								CAT_TEMPERATURE_UNIT_DEGREES_FAHRENHEIT :
								CAT_TEMPERATURE_UNIT_DEGREES_CELSIUS
							);
							CAT_gui_end_menu();
						}
						CAT_gui_end_menu();
					}
					if(CAT_gui_menu_item("SYSTEM"))
					{
#ifdef CAT_EMBEDDED
						CAT_pushdown_push(CAT_MS_system_menu);
#endif
					}
					if(CAT_gui_begin_menu("DANGER ZONE"))
					{
						if(CAT_gui_menu_item("RESET CONFIG FLAGS"))
						{
							CAT_set_config_flags(CAT_SAVE_CONFIG_FLAG_NONE);
						}
							
						if(CAT_gui_menu_item("RESET SAVE"))
							CAT_gui_open_popup("Are you sure? This will delete all game data!\n");
						if(CAT_gui_consume_popup())
							CAT_factory_reset();

						CAT_gui_end_menu();
					}
					CAT_gui_end_menu();
				}
				if(CAT_gui_menu_item("ABOUT"))
					CAT_pushdown_push(CAT_MS_manual);
				if(CAT_gui_begin_menu("POWER"))
				{
					if(CAT_gui_menu_item("SLEEP"))
						CAT_sleep();

					if(CAT_gui_menu_item("PROTECTED OFF"))
						CAT_gui_open_popup("The device must be powered on via the reset button! This will clear important settings. Proceed?\n");
					if(CAT_gui_consume_popup())
						CAT_shutdown();
					
					CAT_gui_end_menu();
				}
				CAT_gui_end_menu();
			}
			break;
		}

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_menu()
{
	// Replaced by GUI render pass
}
