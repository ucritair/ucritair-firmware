#include "cat_menu.h"
#include "cat_room.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_machine.h"
#include "cat_sprite.h"
#include "cat_version.h"

#ifdef CAT_EMBEDDED
#include "menu_time.h"
#include "menu_aqi.h"
#endif

#include <stddef.h>

int selector;

struct entry
{
	const char* title;
	CAT_machine_state state;
} entries[] =
{
	{"INSIGHTS", CAT_MS_stats},
	{"BAG", CAT_MS_bag},
	{"ARCADE CABINET", CAT_MS_arcade},
	{"VENDING MACHINE", CAT_MS_vending},
	{"GAME CONTROLS", CAT_MS_manual},

#ifdef CAT_EMBEDDED
	{"AIR QUALITY", CAT_MS_aqi},
	{"SETUP TIME/SAMPLING", CAT_MS_time},
#endif

	{"BACK", CAT_MS_room}
};
#define NUM_MENU_ITEMS (sizeof(entries)/sizeof(entries[0]))

void CAT_MS_menu(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pulse(CAT_BUTTON_UP))
				selector -= 1;
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
				selector += 1;
			selector = clamp(selector, 0, NUM_MENU_ITEMS-1);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				CAT_machine_transition(&machine, entries[selector].state);
			}

			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_room);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_menu()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("[MENU] ");
	CAT_gui_image(icon_a_sprite, 1);
	CAT_gui_image(icon_enter_sprite, 0);
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	for (int i = 0; i < NUM_MENU_ITEMS; i++)
	{
		CAT_gui_text("#");
		CAT_gui_text(entries[i].title);

		if(i == selector)
			CAT_gui_image(icon_pointer_sprite, 0);

		CAT_gui_line_break();
	}

#ifdef CAT_VERSION_H
	CAT_gui_line_break();
	CAT_gui_line_break();
	CAT_gui_textf("v%d.%d.%d.%d", CAT_VERSION_MAJOR, CAT_VERSION_MINOR, CAT_VERSION_PATCH, CAT_VERSION_PUSH);
#endif
}