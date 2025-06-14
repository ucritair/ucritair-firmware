#include "cat_arcade.h"

#include "cat_machine.h"
#include "cat_math.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_render.h"
#include "cat_inventory.h"
#include <stdio.h>
#include "cat_aqi.h"
#include "sprite_assets.h"

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
} aq_entries[] =
{
#ifdef CAT_EMBEDDED
	{"Air Quality Log", CAT_MS_aqi},
	{"Air Quality Graph", CAT_MS_graph},
#endif
};
#define NUM_AQ_ENTRIES (sizeof(aq_entries)/sizeof(aq_entries[0]))
static int aq_selector = 0;

static struct
{
	const char* title;
	CAT_machine_state state;
} game_entries[] =
{
	{"Snack", CAT_MS_snake},
	{"Sweep", CAT_MS_mines},
	{"Foursquares", CAT_MS_foursquares}
};
#define NUM_GAME_ENTRIES (sizeof(game_entries)/sizeof(game_entries[0]))
static int game_selector = 0;

static enum
{
	AQ,
	GAME
} mode = AQ;

void CAT_MS_arcade(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_arcade);
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_back();

			switch(mode)
			{
				case AQ:
					if(CAT_input_pulse(CAT_BUTTON_UP))
						aq_selector -= 1;
					if(CAT_input_pulse(CAT_BUTTON_DOWN))
						aq_selector += 1;
					if(aq_selector < 0)
					{
						mode = GAME;
						game_selector = NUM_GAME_ENTRIES-1;
					}
					else if(aq_selector >= NUM_AQ_ENTRIES)
					{
						mode = GAME;
						game_selector = 0;
					}
				break;
				case GAME:
					if(CAT_input_pulse(CAT_BUTTON_UP))
						game_selector -= 1;
					if(CAT_input_pulse(CAT_BUTTON_DOWN))
						game_selector += 1;
					if(game_selector < 0)
					{
						mode = AQ;
						aq_selector = (int) NUM_AQ_ENTRIES-1;
					}
					else if(game_selector >= NUM_GAME_ENTRIES)
					{
						mode = AQ;
						aq_selector = 0;
					}
				break;
			}
			
			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				CAT_machine_transition
				(
					mode == AQ ?
					aq_entries[aq_selector].state :
					game_entries[game_selector].state
				);
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_arcade()
{
	CAT_gui_title
	(
		false,
		&icon_enter_sprite, &icon_exit_sprite,
		"ARCADE"
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
	
	CAT_gui_textf("uCritAir Score %0.1f%%\n", CAT_AQI_aggregate());
	CAT_gui_div("AIR QUALITY");
	for(int i = 0; i < NUM_AQ_ENTRIES; i++)
	{
		CAT_gui_textf("\t\1 %s ", aq_entries[i].title);
		if(mode == AQ && i == aq_selector)
			CAT_gui_image(&icon_pointer_sprite, 0);
		CAT_gui_line_break();
	}
	CAT_gui_div("GAMES");
	for(int i = 0; i < NUM_GAME_ENTRIES; i++)
	{
		CAT_gui_textf("\t\1 %s ", game_entries[i].title);
		if(mode == GAME && i == game_selector)
			CAT_gui_image(&icon_pointer_sprite, 0);
		CAT_gui_line_break();
	}
}
