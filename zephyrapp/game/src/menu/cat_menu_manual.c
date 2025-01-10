#include "cat_menu.h"

#include "cat_machine.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_bag.h"
#include "cat_version.h"
#include "cat_menu.h"
#include "cat_pet.h"
#include "cat_main.h"

static enum
{
	CONTROLS,
	STATS,
	ACTIONS,
	DECO,
	AIR,
	SHOPPING,
	CRYPTO,
	ARCADE,
	ABOUT,
	CREDITS,
	LAST
} page = CONTROLS;

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

void CAT_MS_manual(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			for(int i = 0; i < NUM_CREDITS; i++)
			{
				int j = CAT_rand_int(0, NUM_CREDITS-1);
				int temp = credit_indices[i];
				credit_indices[i] = credit_indices[j];
				credit_indices[j] = temp;
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);

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
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_draw_controls()
{
	CAT_gui_title
	(
		true,
		NULL, &icon_exit_sprite,
		"CONTROLS"
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_text("PRESS ");
	CAT_gui_image(&icon_w_sprite, 0);
	CAT_gui_text(" OR ");
	CAT_gui_image(&icon_e_sprite, 0);
	CAT_gui_text(" TO TRAVERSE\nTHE MANUAL SCREENS");
	CAT_gui_div("");
	CAT_gui_line_break();

	CAT_gui_image(&icon_start_sprite, 0);
	CAT_gui_text("Open/close menu");
	CAT_gui_line_break();
	CAT_gui_image(&icon_select_sprite, 0);
	CAT_gui_text("Cycle decor mode");
	CAT_gui_line_break();
	CAT_gui_image(&icon_a_sprite, 0);
	CAT_gui_text("Confirm");
	CAT_gui_line_break();
	CAT_gui_image(&icon_b_sprite, 0);
	CAT_gui_text("Cancel");
	CAT_gui_line_break();
	CAT_gui_image(&icon_n_sprite, 0);
	CAT_gui_text("Navigate up");
	CAT_gui_line_break();
	CAT_gui_image(&icon_e_sprite, 0);
	CAT_gui_text("Navigate right");
	CAT_gui_line_break();
	CAT_gui_image(&icon_s_sprite, 0);
	CAT_gui_text("Navigate down");
	CAT_gui_line_break();
	CAT_gui_image(&icon_w_sprite, 0);
	CAT_gui_text("Navigate left");
}

void CAT_draw_stats()
{
	CAT_gui_title
	(
		true,
		NULL, &icon_exit_sprite,
		"STATS"
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_image(&icon_vig_sprite, 0);
	CAT_gui_image(&icon_foc_sprite, 0);
	CAT_gui_image(&icon_spi_sprite, 0);
	CAT_gui_line_break();

	CAT_gui_text("\1 VIGOUR");
	CAT_gui_line_break();
	CAT_gui_text
	(
		"is your pet's physical stat.\n"
		"A pet with high vigour is\n"
		"healthy. Vigour is restored\n"
		"by eating good food."
	);
	CAT_gui_line_break();

	CAT_gui_text("\1 FOCUS");
	CAT_gui_line_break();
	CAT_gui_text
	(
		"is your pet's mental stat.\n"
		"A pet with high focus is\n"
		"attentive. Focus is restored\n"
		"by reading good books."
	);
	CAT_gui_line_break();

	CAT_gui_text("\1 SPIRIT");
	CAT_gui_line_break();
	CAT_gui_text
	(
		"is your pet's emotional stat.\n"
		"A pet with high spirit is\n"
		"happy. Spirit is restored by\n"
		"playing with good toys."
	);
}

void CAT_draw_actions()
{
	CAT_gui_title
	(
		true,
		NULL, &icon_exit_sprite,
		"CARETAKING"
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_image(&icon_feed_sprite, 0);
	CAT_gui_image(&padkaprow_sprite, 0);
	CAT_gui_image(&icon_study_sprite, 0);
	CAT_gui_image(&book_static_sprite, 0);
	CAT_gui_image(&icon_play_sprite, 0);
	CAT_gui_image(&toy_duck_sprite, 0);
	CAT_gui_line_break();

	CAT_gui_text("\1 FEEDING");
	CAT_gui_line_break();
	CAT_gui_text
	(
		"restores your pet's vigour.\n"
		"Select food from your bag\n"
		"and place it in the room\n"
		"for your pet to eat."
	);
	CAT_gui_line_break();
	
	CAT_gui_text("\1 STUDYING");
	CAT_gui_line_break();
	CAT_gui_text
	(
		"restores your pet's focus.\n"
		"Select a book from your bag\n"
		"and place it in the room for\n"
		"your pet to read."
	);
	CAT_gui_line_break();

	CAT_gui_text("\1 PLAYING");
	CAT_gui_line_break();
	CAT_gui_text
	(
		"restores your pet's spirit.\n"
		"Select a toy from your bag\n"
		"and place it in the room for\n"
		"your pet to play with."
	);
}

void CAT_draw_deco()
{
	CAT_gui_title
	(
		true,
		NULL, &icon_exit_sprite,
		"DECORATION"
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_image(&icon_deco_sprite, 0);
	CAT_gui_image(&padkaprop_sprite, 0);
	CAT_gui_image(&lantern_sprite, 0);
	CAT_gui_image(&bush_daisy_sprite, 0);
	CAT_gui_image(&crystal_purple_lg_sprite, 0);
	CAT_gui_image(&succulent_sprite, 0);
	CAT_gui_line_break();
	CAT_gui_text
	(
		"Furniture can be selected\n"
		"from your bag and placed in\n"
		"the room.\n"
		"Most furniture exists for\n"
		"decoration alone, but some\n"
		"pieces have special effects\n"
		"on your pet or the room"
	);

	CAT_gui_line_break();
	CAT_gui_line_break();

	CAT_gui_image(&cursor_add_sprite, 0);
	CAT_gui_image(&cursor_flip_sprite, 0);
	CAT_gui_image(&cursor_remove_sprite, 0);
	CAT_gui_line_break();
	CAT_gui_text
	(
		"There are three modes of\n"
		"decoration: ADD, FLIP, and\n"
		"REMOVE. Some furnishings\n"
		"have special forms that can\n"
		"be flipped through."
	);
}

void CAT_draw_air()
{
	CAT_gui_title
	(
		true,
		NULL, &icon_exit_sprite,
		"AIR QUALITY"
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_image(&icon_co2_sprite, 0);
	CAT_gui_image(&icon_mask_sprite, 0);
	CAT_gui_image(&icon_pm_sprite, 2);
	CAT_gui_image(&icon_pure_sprite, 0);
	CAT_gui_image(&icon_nox_sprite, 0);
	CAT_gui_image(&icon_uv_sprite, 0);
	CAT_gui_image(&icon_voc_sprite, 2);
	CAT_gui_line_break();

	CAT_gui_text
	(
		"Real-life air quality\n"
		"affects the rate at which\n"
		"your pet's stats degrade.\n"
		"Air quality factors are\n"
		"given a score of\n"
	);
	CAT_gui_text("GOOD");
	CAT_gui_image(&icon_aq_ccode_sprite, 0);
	CAT_gui_text(", NORMAL");
	CAT_gui_image(&icon_aq_ccode_sprite, 1);
	CAT_gui_text(", or BAD");
	CAT_gui_image(&icon_aq_ccode_sprite, 2);
	CAT_gui_line_break();
	CAT_gui_text("where the scored factors are\n");
	CAT_gui_image(&icon_co2_sprite, 1);
	CAT_gui_text("CO2, ");
	CAT_gui_image(&icon_nox_sprite, 1);
	CAT_gui_text("NOx, ");
	CAT_gui_image(&icon_voc_sprite, 1);
	CAT_gui_text("VOCs,\n");
	CAT_gui_text("and ");
	CAT_gui_image(&icon_pm_sprite, 1);
	CAT_gui_text("PM2.5.");
	CAT_gui_line_break();
	CAT_gui_line_break();
	
	CAT_gui_text
	(
		"Some items have effects\n"
		"which protect the pet's\n"
		"stats from low air quality\n"
		"if placed or equipped.\n"
	);
}

void CAT_draw_shopping()
{
	CAT_gui_title
	(
		true,
		NULL, &icon_exit_sprite,
		"SHOPPING"
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	gui.cursor.x += 6;
	for(int i = 0; i < 11; i++)
	{
		CAT_gui_image(&coin_world_sprite, i % 4);
	}
	CAT_gui_line_break();
	CAT_gui_text
	(
		"Feeding, studying, playing,\n"
		"decoration, and air quality\n"
		"protection all rely on the\n"
		"use of items.\n"
		"Items of all kinds can be\n"
		"purchased at the vending\n"
		"machine using coins. Click\n"
		"on the vending machine or\n"
		"navigate to it from the\n"
		"menu to purchase items.\n"
		"Some items disappear upon\n"
		"use, but others persist.\n"
		"To earn coins, place ETH\n"
		"mining devices in the room\n"
		"or play games at the arcade."
	);
	CAT_gui_line_break();
	gui.cursor.x += 2;
	for(int i = 2; i < 13; i++)
	{
		CAT_gui_image(&coin_world_sprite, i % 4);
	}
}

void CAT_draw_crypto()
{
	CAT_gui_title
	(
		true,
		NULL, &icon_exit_sprite,
		"MINING"
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_text
	(
		"While coins can be earned\n"
		"from a variety of sources\n"
		"the surest and steadiest way\n"
		"to generate income is mining."
	);
	CAT_gui_line_break();

	CAT_gui_image(&gpu_sprite, 0);
	CAT_gui_image(&coin_world_sprite, 2);
	CAT_gui_image(&gpu_sprite, 0);
	CAT_gui_image(&coin_world_sprite, 1);
	CAT_gui_image(&gpu_sprite, 0);
	CAT_gui_image(&coin_world_sprite, 0);
	CAT_gui_line_break();

	CAT_gui_text
	(
		"Mining devices can be placed\n"
		"in the room, and will emit\n"
		"earned coins at a fixed rate.\n"
		"There is a cap on the number\n"
		"of coins present at one time,\n"
		"so make sure to pick them up\n"
		"by touching them."
	);
	CAT_gui_line_break();

	gui.cursor.y += 16;
	CAT_gui_image(&coin_world_sprite, 0);
	CAT_gui_image(&gpu_sprite, 0);
	CAT_gui_image(&coin_world_sprite, 1);
	CAT_gui_image(&gpu_sprite, 0);
	CAT_gui_image(&coin_world_sprite, 2);
	CAT_gui_image(&gpu_sprite, 0);
}

void CAT_draw_arcade()
{
	CAT_gui_title
	(
		true,
		NULL, &icon_exit_sprite,
		"ARCADE"
	);

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_image(&snake_tail_sprite, 0);
	gui.cursor.x -= gui.pad;
	for(int i = 0; i < 11; i++)
	{
		CAT_gui_image(&snake_body_sprite, 0);
		gui.cursor.x -= gui.pad;
	}
	CAT_gui_image(&snake_head_sprite, 0);
	CAT_gui_image(&padkaprow_sprite, 0);
	CAT_gui_line_break();

	CAT_gui_text
	(
		"One way to actively earn\n"
		"money and have some fun\n"
		"along the way is to play\n"
		"games on the arcade cabinet.\n"
		"The popular game Snack is\n"
		"currently available there."
	);
	CAT_gui_line_break();

	CAT_gui_image(&coffee_sprite, 0);
	CAT_gui_image(&snake_head_sprite, 2);
	gui.cursor.x -= gui.pad;
	for(int i = 0; i < 11; i++)
	{
		CAT_gui_image(&snake_body_sprite, 2);
		gui.cursor.x -= gui.pad;
	}
	CAT_gui_image(&snake_tail_sprite, 2);
	CAT_gui_line_break();

	CAT_gui_text
	(
		"Snack Cat primarily grows by\n"
		"gobbling up snacks, but every\n"
		"few bites he'll encounter a\n"
		"shiny golden coin. Snap up\n"
		"the coin to spend it at the\n"
		"shop, and remember not to\n"
		"smack into anything."
	);
	CAT_gui_line_break();

	CAT_gui_image(&snake_tail_sprite, 0);
	gui.cursor.x -= gui.pad;
	for(int i = 0; i < 11; i++)
	{
		CAT_gui_image(&snake_body_sprite, 0);
		gui.cursor.x -= gui.pad;
	}
	CAT_gui_image(&snake_head_sprite, 0);
	CAT_gui_image(&coin_static_sprite, 0);
}

void CAT_draw_about()
{
	CAT_gui_title
	(
		true,
		NULL, &icon_exit_sprite,
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
		NULL, &icon_exit_sprite,
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
		case CONTROLS:
		{
			CAT_draw_controls();
			break;
		}
		case STATS:
		{
			CAT_draw_stats();
			break;
		}
		case ACTIONS:
		{
			CAT_draw_actions();
			break;
		}
		case DECO:
		{
			CAT_draw_deco();
			break;
		}
		case AIR:
		{
			CAT_draw_air();
			break;
		}
		case SHOPPING:
		{
			CAT_draw_shopping();
			break;
		}
		case CRYPTO:
		{
			CAT_draw_crypto();
			break;
		}
		case ARCADE:
		{
			CAT_draw_arcade();
			break;
		}
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
			CAT_gui_title
			(
				true,
				NULL, &icon_exit_sprite,
				"LAST"
			);
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			CAT_gui_set_flag(CAT_GUI_WRAP_TEXT);
			CAT_gui_text("You shouldn't be here");
			break;
		}
	}
}