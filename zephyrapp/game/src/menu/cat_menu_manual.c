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

enum
{
	ABOUT,
	CREDITS,
	LAST
};
static int page = ABOUT;

static const char* credits[] =
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
	"Bilal",
	"Griffon",
	"V",
	"Justin"
};
#define NUM_CREDITS (sizeof(credits) / sizeof(credits[0]))
static int credit_indices[NUM_CREDITS];

void CAT_MS_manual(CAT_FSM_signal signal)
{
	switch(signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_manual);
			for(int i = 0; i < NUM_CREDITS; i++)
				credit_indices[i] = i;
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
				CAT_pushdown_pop();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_pushdown_rebase(CAT_MS_room);

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

#define PAD 8
static int cursor_x = PAD;
static int cursor_y = PAD;

static void draw_page(const char* title)
{
	cursor_x = PAD;
	cursor_y = PAD;

	CAT_frameberry(CAT_WHITE);
	cursor_y = CAT_draw_textf(cursor_x, cursor_y, "%s\n", title);
	cursor_y += PAD;
	CAT_rowberry(cursor_y, cursor_y+1, CAT_BLACK);
	cursor_y += PAD;
}

void CAT_draw_about()
{
	draw_page("< ABOUT >");

	CAT_draw_sprite_raw(&icon_ee_sprite, 0, cursor_x, cursor_y);
	cursor_y += icon_ee_sprite.height + PAD;
	
	cursor_y = CAT_draw_textf
	(
		cursor_x, cursor_y,
		"CAT v%d.%d.%d.%d\n"
		"by Entropic Engineering.\n"
		"\n"
		"Powered by grants from\n"
		"Balvi and Kanro.\n"
		"\n"
		"Visit uCritter.com/air\n"
		"for more information.",
		CAT_VERSION_MAJOR,
		CAT_VERSION_MINOR,
		CAT_VERSION_PATCH,
		CAT_VERSION_PUSH
	);
}

void CAT_draw_credits()
{
	draw_page("< CREDITS >");

	int cursor_y = CAT_TILE_SIZE * 2 + 4;
	for(int i = 0; i < NUM_CREDITS; i++)
	{
		CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
		cursor_y = CAT_draw_text(120, cursor_y, credits[credit_indices[i]]);
		cursor_y += CAT_TEXT_LINE_HEIGHT;
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
			draw_page("INVALID");
			break;
		}
	}
}