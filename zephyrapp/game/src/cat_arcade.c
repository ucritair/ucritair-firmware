#include "cat_arcade.h"

#include "cat_machine.h"
#include "cat_math.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_sprite.h"
#include "cat_bag.h"
#include <stdio.h>

#ifdef CAT_EMBEDDED
#include "menu_system.h"
#include "menu_aqi.h"
#include "menu_graph.h"
#endif

//////////////////////////////////////////////////////////////////////////
// MACHINE

static struct
{
	const char* title;
	CAT_machine_state state;
} entries[] =
{
	{"AIR", NULL},
#ifdef CAT_EMBEDDED
	{"Air Quality Log", CAT_MS_aqi},
	{"Air Quality Graph", CAT_MS_graph},
#endif
	{"ARCADE", NULL},
	{"Snack", CAT_MS_snake},
	{"Sweep", CAT_MS_mines}
};
#define NUM_ENTRIES (sizeof(entries)/sizeof(entries[0]))
static int selector = -1;

int seek_entry(int idx, int dir)
{
	if(dir == 0)
		return idx;
	for(int i = idx + dir; i >= 0 && i < NUM_ENTRIES; i += dir)
	{
		if(entries[i].state != NULL)
			return i;
	}
	return idx;
}

void CAT_MS_arcade(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			if(selector == -1)
				selector = seek_entry(0, 1);
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
			
			int seldir = 0;
			if(CAT_input_pulse(CAT_BUTTON_UP))
				seldir = -1;
			if(CAT_input_pulse(CAT_BUTTON_DOWN))
				seldir = 1;
			selector = seek_entry(selector, seldir);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				if(entries[selector].state != NULL)
					CAT_machine_transition(entries[selector].state);
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_arcade()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_gui_text("ARCADE ");
	CAT_gui_image(icon_a_sprite, 1);
	CAT_gui_image(icon_enter_sprite, 0);
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
	
	CAT_gui_textf("uCritAir Score %0.1f%%\n", CAT_AQI_aggregate());
	for(int i = 0; i < NUM_ENTRIES; i++)
	{
		if(entries[i].state == NULL)
		{
			CAT_gui_div(entries[i].title);
		}
		else
		{
			CAT_gui_textf("\t& %s ", entries[i].title);
			if(i == selector)
				CAT_gui_image(icon_pointer_sprite, 0);
			CAT_gui_line_break();
		}
	}
}
