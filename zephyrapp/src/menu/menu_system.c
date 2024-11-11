#include "menu_system.h"

#include "cat_room.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_machine.h"
#include "cat_sprite.h"
#include "cat_version.h"
#include "cat_menu.h"

#include "misc.h"
#include "menu_time.h"
#include "power_control.h"
#include "rtc.h"
#include "epaper_rendering.h"
#include "sdcard.h"

#include <stddef.h>

int system_menu_selector = 0;

char* system_menu_note = "";

typedef void (*menu_t)();

void menu_t_back()
{
	CAT_machine_transition(&machine, CAT_MS_menu);
}

void menu_t_go_time()
{
	CAT_machine_transition(&machine, CAT_MS_time);
}

void menu_t_sleep()
{
	power_off(sensor_wakeup_rate*1000, false);
}

void menu_t_power_off()
{
	epaper_render_protected_off();
	power_off(0, true);
}

void menu_t_erase_logs()
{
	flash_erase_all_cells();
	system_menu_note = "Done :)";
}

void menu_t_write_logs()
{
	enum sdcard_result res = write_log_to_sdcard();

	if (res == OK)
		system_menu_note = "Written :)";
	else if (res == FAIL_INIT)
		system_menu_note = "Failed to init card";
 	else if (res == FAIL_MOUNT)
 		system_menu_note = "Failed to mount FS";
 	else if (res == FAIL_WRITE)
 		system_menu_note = "Failed to write data";
 	else if (res == FAIL_MKDIR)
 		system_menu_note = "Failed to mkdir";
 	else if (res == FAIL_CREATE)
 		system_menu_note = "Failed to creat";
 	else if (res == FAIL_CLOSE)
 		system_menu_note = "Failed closing file";
 	else
 		system_menu_note = "Unknown error saving";
}

void menu_t_update_eink()
{
	epaper_render_test();
	system_menu_note = "Done :)";
}

struct entry
{
	const char* title;
	menu_t action;
} system_entries[] =
{
	{"SET CLOCK + LOG RATE", menu_t_go_time},
	{"POWER DOWN FOR TRAVEL", menu_t_power_off},
	{"ERASE ON-DEVICE LOGS", menu_t_erase_logs},
	{"WRITE LOGS TO SDCARD", menu_t_write_logs},
	{"UPDATE EINK", menu_t_update_eink},
	{"SLEEP", menu_t_sleep},

	{"BACK", menu_t_back}
};
#define NUM_MENU_ITEMS (sizeof(system_entries)/sizeof(system_entries[0]))

void CAT_MS_system_menu(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			system_menu_note = "Some items take time";
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pulse(CAT_BUTTON_UP))
				system_menu_selector -= 1;
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
				system_menu_selector += 1;
			system_menu_selector = clamp(system_menu_selector, 0, NUM_MENU_ITEMS-1);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				system_entries[system_menu_selector].action();
			}

			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, CAT_MS_menu);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_system_menu()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("SYSTEM MENU ");
	CAT_gui_image(icon_a_sprite, 1);
	CAT_gui_image(icon_enter_sprite, 0);
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	for (int i = 0; i < NUM_MENU_ITEMS; i++)
	{
		CAT_gui_text("#");
		CAT_gui_text(system_entries[i].title);

		if(i == system_menu_selector)
			CAT_gui_image(icon_pointer_sprite, 0);

		CAT_gui_line_break();
	}

	CAT_gui_line_break();
	CAT_gui_text(system_menu_note);

	CAT_gui_line_break();
	CAT_gui_text(" ");

	CAT_gui_line_break();
	CAT_gui_textf("SYS v." SYS_FW_VERSION);
}