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
#include "cat_chat.h"
#include "cat_save.h"
#include "cat_wifi.h"
#include "cat_crypto.h"
#include "cat_radio.h"

#ifdef CAT_EMBEDDED
#include "menu_system.h"
#endif

void CAT_MS_menu(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_menu);
			CAT_gui_menu_force_reset();
		}
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

				if(CAT_gui_begin_menu("ARCADE"))
				{
					if(CAT_gui_menu_item("SNACK"))
						CAT_pushdown_push(CAT_MS_snake);
					if(CAT_gui_menu_item("SWEEP"))
						CAT_pushdown_push(CAT_MS_mines);
					if(CAT_gui_menu_item("FOURSQUARES"))
						CAT_pushdown_push(CAT_MS_foursquares);
					if(CAT_gui_menu_item("STROOP"))
						CAT_pushdown_push(CAT_MS_stroop);
					CAT_gui_end_menu();
				}

				if(CAT_pushdown_last() == &CAT_MS_room)
				{
					if(CAT_gui_menu_item("EXPLORE"))
					{
						CAT_show_world_intro();
						CAT_pushdown_rebase(CAT_MS_world);
					}
				}
				else if(CAT_pushdown_last() == &CAT_MS_world)
				{
					if(CAT_gui_menu_item("GO HOME"))
					{
						CAT_room_intro();
						CAT_pushdown_rebase(CAT_MS_room);
					}
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

						if(CAT_gui_menu_item("COLOUR PICKER"))
							CAT_pushdown_push(CAT_MS_colour_picker);

						if(CAT_gui_menu_toggle("ALWAYS AWAKE", persist_flags & CAT_PERSIST_CONFIG_FLAG_ETERNAL_WAKE, CAT_GUI_TOGGLE_STYLE_CHECKBOX))
						{
							if(persist_flags & CAT_PERSIST_CONFIG_FLAG_ETERNAL_WAKE)
								persist_flags &= ~CAT_PERSIST_CONFIG_FLAG_ETERNAL_WAKE;
							else
								persist_flags |= CAT_PERSIST_CONFIG_FLAG_ETERNAL_WAKE;
						}
						
#if CAT_WIFI_ENABLED					
						if(CAT_gui_menu_item("RP2350 BOOTLOADER"))
						{
							if (CAT_wifi_bootloader(2000)) {
								CAT_printf("Rebooted RP2350 to bootloader\n");
							} else {
								CAT_printf("Failed to reboot RP2350 to bootloader\n");
							}
						}

						if(CAT_is_wifi_connected())
						{
							if(CAT_gui_menu_item("AUTHENTICATE DEVICE"))
							{
								msg_payload_zkp_authenticate_response_t response;
								CAT_wifi_ZK_authenticate(&response, CAT_MINUTE_SECONDS * 10 * 1000);
							}
						}

						if(CAT_wifi_is_ZK_authenticated() && CAT_AQ_sensors_initialized())
						{
							if(CAT_gui_menu_item("UPLOAD LATEST DATA"))
							{
								uint32_t data[CAT_WIFI_DATUM_COUNT] = 
								{
									CAT_ZK_CO2(),
									CAT_ZK_PM2_5(),
									CAT_ZK_temp(),
									CAT_ZK_stroop(),
									CAT_ZK_survey()
								};
								CAT_wifi_send_data(data, CAT_WIFI_DATUM_COUNT, 120000);
							}
						}
#endif						

#if CAT_RADIO_ENABLED
						if(CAT_AQ_sensors_initialized())
						{
							if(CAT_gui_menu_item("BROADCAST AQ"))
							{
								CAT_radio_telemetry_TX();
							}
						}
#endif

						CAT_gui_end_menu();
					}				
				}

#if CAT_RADIO_ENABLED | defined(CAT_DESKTOP)
				if(CAT_gui_menu_item("CHAT"))
					CAT_pushdown_push(CAT_MS_chat);
#endif

				if(CAT_gui_begin_menu("SETTINGS"))
				{
					if(CAT_gui_begin_menu("GAMEPLAY"))
					{
						if(CAT_gui_menu_toggle("GAME FIRST", !(persist_flags & CAT_PERSIST_CONFIG_FLAG_AQ_FIRST), CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON))
							persist_flags &= ~CAT_PERSIST_CONFIG_FLAG_AQ_FIRST;
						if(CAT_gui_menu_toggle("DASHBOARD FIRST", persist_flags & CAT_PERSIST_CONFIG_FLAG_AQ_FIRST, CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON))
							persist_flags |= CAT_PERSIST_CONFIG_FLAG_AQ_FIRST;

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
							CAT_gui_open_keyboard(pet.name, sizeof(pet.name));

						if(CAT_gui_begin_menu("ROOM THEME"))
						{
							for(int i = 0; i < CAT_ROOM_THEME_COUNT; i++)
							{		
								bool this_theme = CAT_room_get_theme() == CAT_room_theme_list[i];
								if(CAT_gui_menu_toggle(CAT_room_theme_list[i]->name, this_theme, CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON))
									CAT_room_set_theme(CAT_room_theme_list[i]);
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
							CAT_set_eink_update_flag(true);

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

#if CAT_WIFI_ENABLED | defined(CAT_DESKTOP)
					if(CAT_gui_menu_item("NETWORK"))
						CAT_pushdown_push(CAT_MS_wifi);
#endif

#ifdef CAT_EMBEDDED
					if(CAT_gui_menu_item("SYSTEM"))
						CAT_pushdown_push(CAT_MS_system_menu);
#endif

					if(CAT_gui_begin_menu("DANGER ZONE"))
					{						
						if(CAT_gui_menu_item("RESET SAVE"))
							CAT_gui_open_popup("Are you sure? This will delete all game data!\n", CAT_POPUP_STYLE_YES_NO);
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
						CAT_gui_open_popup("The device must be powered on via the reset button! This will clear important settings. Proceed?\n", CAT_POPUP_STYLE_YES_NO);
					if(CAT_gui_consume_popup())
						CAT_shutdown();
					
					CAT_gui_end_menu();
				}
				CAT_gui_end_menu();
			}
			break;
		}

		case CAT_FSM_SIGNAL_EXIT:
		{
		}
		break;
	}
}

void CAT_render_menu()
{
	// Replaced by GUI render pass
}
