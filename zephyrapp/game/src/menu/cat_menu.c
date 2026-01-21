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
#include "cat_text.h"
#include "sprite_assets.h"
#include "cat_alloc.h"

#ifdef CAT_EMBEDDED
#include "menu_system.h"
#endif

#ifdef CAT_RESEARCH_ONLY
#include "cat_research.h"
#endif

// Date/time
static CAT_datetime dt_real;
static CAT_datetime dt_cached;

// Log rate
static int lr_mins;

#ifdef CAT_DESKTOP
#define CO2_CAL_TIME 5
#else
#define CO2_CAL_TIME (CAT_MINUTE_SECONDS/**5*/)
#endif
static bool co2_cal_status = false;
static uint64_t co2_cal_time = 0;
static bool co2_cal_sensor_called = false;

static void start_co2_cal()
{
	co2_cal_status = true;
	co2_cal_time = CAT_get_RTC_now();
	co2_cal_sensor_called = false;
	CAT_GUI_open_window("co2_cal");
}

static void end_co2_cal()
{
	if(co2_cal_sensor_called)
		CAT_inventory_add(food_cigs_item, 1);
	co2_cal_status = false;
	CAT_GUI_close_current_window();
}

static void tick_co2_cal()
{
	if(CAT_GUI_begin_window("co2_cal", 12, CAT_LCD_SCREEN_H/4, CAT_LCD_SCREEN_W-12, 3*CAT_LCD_SCREEN_H/4))
	{
		int elapsed = CAT_get_RTC_now() - co2_cal_time;
		int remaining = CO2_CAL_TIME - elapsed;
		
		if(remaining >= 1)
		{
			CAT_GUI_text("Please go outside and wait while CO2 sensor reading settles before calibration.\n");
			CAT_GUI_image(&icon_nosmoke_sprite, 0);
			CAT_GUI_text("\n%02d:%02d remaining...", remaining/60, remaining%60);
		}
		else if(remaining >= 0)
		{
			CAT_GUI_text("Please go outside and wait while CO2 sensor reading settles before calibration.\n");
			CAT_GUI_image(&icon_nosmoke_sprite, 0);
			CAT_GUI_text("\nCalibrating...");
		}
		else if(remaining >= -1)
		{
			CAT_GUI_text("Please go outside and wait while CO2 sensor reading settles before calibration.\n");
			CAT_GUI_image(&icon_nosmoke_sprite, 0);
			CAT_GUI_text("\nCalibrating...");
			if(!co2_cal_sensor_called)
			{
				co2_cal_sensor_called = true;
				CAT_force_sensor_read(CAT_SENSOR_SUNRISE);
			}
		}
		else
		{
			CAT_GUI_text("Done. Thanks for waiting... Have some cigarettes as a reward:\n");
			CAT_GUI_image(&cigarette_sprite, 0);
			CAT_GUI_text("");
			if(CAT_GUI_option("EXIT"))
				end_co2_cal();
		}

		if(CAT_input_dismissal())
			end_co2_cal();
	}
}

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

				if(CAT_check_save_flags(CAT_SAVE_CONFIG_FLAG_DEVELOPER))
				{
					if(CAT_gui_begin_menu("DEVELOPER"))
					{
						if(CAT_gui_menu_item("INFO"))
							CAT_pushdown_push(CAT_MS_debug);

						if(CAT_gui_menu_item("COLOUR PICKER"))
							CAT_pushdown_push(CAT_MS_colour_picker);

						if(CAT_gui_menu_item("TURNKEY APARTMENT"))
							CAT_set_load_flags(CAT_LOAD_FLAG_DIRTY | CAT_LOAD_FLAG_TURNKEY);

#if CAT_RESEARCH_ONLY
						if(CAT_gui_menu_item("RESEARCH MODE"))
							CAT_pushdown_rebase(CAT_MS_research_screen);
#endif
						
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
					if(CAT_gui_begin_menu("TIME"))
					{
						CAT_get_datetime(&dt_cached);
						dt_real = dt_cached;

						dt_real.year = CAT_gui_menu_ticker("YEAR", dt_real.year, 0, 3000);
						dt_real.month = CAT_gui_menu_ticker("MONTH", dt_real.month, 1, 12);
						dt_real.day = CAT_gui_menu_ticker("DAY", dt_real.day, 1, 31);
						dt_real.hour = CAT_gui_menu_ticker("HOUR", dt_real.hour, 0, 23);
						dt_real.minute = CAT_gui_menu_ticker("MINUTE", dt_real.minute, 0, 59);
						dt_real.second = CAT_gui_menu_ticker("SECOND", dt_real.second, 0, 59);

						if(CAT_timecmp(&dt_real, &dt_cached) != 0)
							CAT_set_datetime(dt_real);

						CAT_gui_end_menu();
					}

					if(CAT_gui_begin_menu("AIR QUALITY"))
					{
						if(CAT_gui_menu_toggle("USE CELSIUS", !(persist_flags & CAT_PERSIST_CONFIG_FLAG_USE_FAHRENHEIT), CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON))
							persist_flags &= ~CAT_PERSIST_CONFIG_FLAG_USE_FAHRENHEIT;
						if(CAT_gui_menu_toggle("USE FAHRENHEIT", persist_flags & CAT_PERSIST_CONFIG_FLAG_USE_FAHRENHEIT, CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON))
							persist_flags |= CAT_PERSIST_CONFIG_FLAG_USE_FAHRENHEIT;
						CAT_AQ_set_temperature_unit
						(
							persist_flags & CAT_PERSIST_CONFIG_FLAG_USE_FAHRENHEIT ?
							CAT_TEMPERATURE_UNIT_DEGREES_FAHRENHEIT :
							CAT_TEMPERATURE_UNIT_DEGREES_CELSIUS
						);
						
						if(CAT_gui_menu_item("CALIBRATE CO2"))
							start_co2_cal();

						CAT_gui_end_menu();
					}

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

#if CAT_WIFI_ENABLED | defined(CAT_DESKTOP)
					if(CAT_gui_menu_item("NETWORK"))
						CAT_pushdown_push(CAT_MS_wifi);
#endif

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

					if(CAT_gui_begin_menu("LOGS AND SAVE"))
					{
						lr_mins = sensor_wakeup_period / CAT_MINUTE_SECONDS;
						lr_mins = CAT_gui_menu_ticker("LOG RATE (MINUTES)", lr_mins, 0, 60);
						sensor_wakeup_period = lr_mins * CAT_MINUTE_SECONDS;

						if(CAT_gui_menu_item("WRITE LOGS TO SD"))
						{
							CAT_SD_write_result result = CAT_write_logs_to_SD();
							if(result == CAT_SD_WRITE_OKAY)
								CAT_GUI_open_window("sd_write_success");
							else
								CAT_GUI_open_window("sd_write_failure");
						}
						if(CAT_GUI_begin_window("sd_write_success", 12, CAT_LCD_SCREEN_H/4, CAT_LCD_SCREEN_W-12, 2*CAT_LCD_SCREEN_H/4))
						{
							CAT_GUI_text("Write succeeded!\n");
							if(CAT_GUI_option("OKAY"))
								CAT_GUI_close_current_window();
							CAT_GUI_end_window();
						}
						if(CAT_GUI_begin_window("sd_write_failure", 12, CAT_LCD_SCREEN_H/4, CAT_LCD_SCREEN_W-12, 2*CAT_LCD_SCREEN_H/4))
						{
							CAT_GUI_text("Write failed!\nPlease ensure that an SD card is inserted.\n");
							if(CAT_GUI_option("OKAY"))
								CAT_GUI_close_current_window();
							CAT_GUI_end_window();
						}

						if(CAT_gui_menu_item("ERASE LOGS"))
							CAT_GUI_open_window("erase_logs_warn");
						if(CAT_GUI_begin_window("erase_logs_warn", 12, CAT_LCD_SCREEN_H/4, CAT_LCD_SCREEN_W-12, 3*CAT_LCD_SCREEN_H/4))
						{
							CAT_GUI_text("Deleting your logs is an irreversible action. Are you completely sure that you want to proceed?\n");
							if(CAT_GUI_option("GO BACK"))
								CAT_GUI_close_current_window();
							if(CAT_GUI_option("ERASE LOGS"))
							{
								CAT_erase_logs();
								CAT_GUI_open_window("erase_logs_done");
								CAT_GUI_close_current_window();
							}
							CAT_GUI_end_window();
						}
						if(CAT_GUI_begin_window("erase_logs_done", 12, CAT_LCD_SCREEN_H/4, CAT_LCD_SCREEN_W-12, 2*CAT_LCD_SCREEN_H/4))
						{
							CAT_GUI_text("All logs erased!\n");
							if(CAT_GUI_option("OKAY"))
								CAT_GUI_close_current_window();
							CAT_GUI_end_window();
						}

						if(CAT_gui_menu_item("ERASE SAVE"))
							CAT_GUI_open_window("erase_save_warn");
						if(CAT_GUI_begin_window("erase_save_warn", 12, CAT_LCD_SCREEN_H/4, CAT_LCD_SCREEN_W-12, 3*CAT_LCD_SCREEN_H/4))
						{
							CAT_GUI_text("Resetting your save is an irreversible action. Are you completely sure that you want to proceed?\n");
							if(CAT_GUI_option("GO BACK"))
								CAT_GUI_close_current_window();
							if(CAT_GUI_option("ERASE SAVE"))
							{
								CAT_reset_save();
								CAT_GUI_open_window("erase_save_done");
								CAT_GUI_close_current_window();
							}
							CAT_GUI_end_window();
						}
						if(CAT_GUI_begin_window("erase_save_done", 12, CAT_LCD_SCREEN_H/4, CAT_LCD_SCREEN_W-12, 2*CAT_LCD_SCREEN_H/4))
						{
							CAT_GUI_text("Save reset!");
							if(CAT_GUI_option("OKAY"))
								CAT_GUI_close_current_window();
							CAT_GUI_end_window();
						}

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
						CAT_GUI_open_window("protected_off_warn");
					if(CAT_GUI_begin_window("protected_off_warn", 12, CAT_LCD_SCREEN_H/4, CAT_LCD_SCREEN_W-12, 3*CAT_LCD_SCREEN_H/4))
					{
						CAT_GUI_text("After entering protected off mode, the device must be powered on via the reset button.");
						CAT_GUI_text("This will clear important settings. Are you sure you want to proceed?\n");
						if(CAT_GUI_option("GO BACK"))
							CAT_GUI_close_current_window();
						if(CAT_GUI_option("PROTECTED OFF"))
							CAT_shutdown();
						CAT_GUI_end_window();
					}
					
					CAT_gui_end_menu();
				}
				CAT_gui_end_menu();
			}
			
			if(co2_cal_status)
				tick_co2_cal();
		}
		break;

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
