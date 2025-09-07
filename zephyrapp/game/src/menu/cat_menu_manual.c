#include "cat_menu.h"

#include "cat_machine.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_item.h"
#include "cat_version.h"
#include "cat_menu.h"
#include "cat_pet.h"
#include "cat_main.h"
#include "sprite_assets.h"

static enum
{
	ABOUT,
	CREDITS,
	LAST
} page = ABOUT;

const char* credits[] =
{
	"Aurora Aldrich",
	"Campbell",
	"DmitryGR",
	"George Rudolf",
	"Ivy Fae",
	"Kristina",
	"Lain",
	"Louis Goessling",
	"M Pang",
	"Minnerva Zou",
	"Neutron",
	"Rachel",
	"Rebecca Rehm",
	"Tasha Schneider",
	"Tomas Stegemann",
};
#define NUM_CREDITS (sizeof(credits) / sizeof(credits[0]))

int credit_indices[NUM_CREDITS] =
{
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14
};

void CAT_MS_manual(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_manual);
			for(int i = 0; i < NUM_CREDITS; i++)
			{
				int j = CAT_rand_int(0, NUM_CREDITS-1);
				int temp = credit_indices[i];
				credit_indices[i] = credit_indices[j];
				credit_indices[j] = temp;
			}
			break;
		}
		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_pushdown_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_pushdown_transition(CAT_MS_room);

			if(CAT_input_pulse(CAT_BUTTON_LEFT))
			{
				if(page == 0)
					page = LAST-1;
				else
					page -= 1;
			}
			if(CAT_input_pulse(CAT_BUTTON_RIGHT))
			{
				page += 1;
				if(page >= LAST)
					page = 0;
			}	
			break;
		}
		case CAT_FSM_SIGNAL_EXIT:
			break;
	}
}

void CAT_draw_about()
{
	CAT_gui_title
	(
		true,
		"ABOUT"
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	gui.cursor.x += 24;
	CAT_gui_image(&icon_ee_sprite, 0);
	CAT_gui_line_break();
	CAT_gui_line_break();
	
	CAT_gui_textf
	(
		"CAT v%d.%d.%d.%d\n"
		"by Entropic Engineering.\n"
		"\n"
		,CAT_VERSION_MAJOR,
		CAT_VERSION_MINOR,
		CAT_VERSION_PATCH,
		CAT_VERSION_PUSH
	);
	CAT_gui_text
	(
		"Powered by grants from\n"
		"Balvi and Kanro.\n"
		"\n"
		"Visit uCritter.com/air\n"
		"for more information."
	);
}

void CAT_draw_credits()
{
	CAT_gui_title
	(
		true,
		"CREDITS"
	);
	
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	for(int i = 0; i < NUM_CREDITS; i++)
	{
		if((i&1)==1)
			CAT_gui_text("\t");
		CAT_gui_text(credits[credit_indices[i]]);
		CAT_gui_line_break();
	}
}

void CAT_render_manual()
{
	switch(page)
	{
		case ABOUT:
		{
			CAT_draw_about();
			break;
		}
		case CREDITS:
		{
			CAT_draw_credits();
			break;
		}
		default:
		{
			CAT_gui_title(true, "LAST");
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			CAT_gui_set_flag(CAT_GUI_FLAG_WRAPPED);
			CAT_gui_text("You shouldn't be here");
			break;
		}
	}
}