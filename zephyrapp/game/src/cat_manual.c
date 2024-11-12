#include "cat_manual.h"

#include "cat_machine.h"
#include "cat_sprite.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_bag.h"

enum
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
	LAST
} page = CONTROLS;

void CAT_MS_manual(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(CAT_MS_menu);
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
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_gui_text("< CONTROLS > ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_image(icon_start_sprite, 0);
	CAT_gui_text("Open/close menu");
	CAT_gui_line_break();
	CAT_gui_image(icon_select_sprite, 0);
	CAT_gui_text("Cycle decor mode");
	CAT_gui_line_break();
	CAT_gui_image(icon_a_sprite, 0);
	CAT_gui_text("Confirm");
	CAT_gui_line_break();
	CAT_gui_image(icon_b_sprite, 0);
	CAT_gui_text("Cancel");
	CAT_gui_line_break();
	CAT_gui_image(icon_n_sprite, 0);
	CAT_gui_text("Navigate up");
	CAT_gui_line_break();
	CAT_gui_image(icon_e_sprite, 0);
	CAT_gui_text("Navigate right");
	CAT_gui_line_break();
	CAT_gui_image(icon_s_sprite, 0);
	CAT_gui_text("Navigate down");
	CAT_gui_line_break();
	CAT_gui_image(icon_w_sprite, 0);
	CAT_gui_text("Navigate left");
}

void CAT_draw_stats()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_gui_text("< STATS > ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_image(icon_vig_sprite, 0);
	CAT_gui_image(icon_foc_sprite, 0);
	CAT_gui_image(icon_spi_sprite, 0);
	CAT_gui_line_break();

	CAT_gui_text("# VIGOUR");
	CAT_gui_line_break();
	CAT_gui_text
	(
		"is your pet's physical stat.\n"
		"A pet with high vigour is\n"
		"healthy. Vigour is restored\n"
		"by eating good food."
	);
	CAT_gui_line_break();

	CAT_gui_text("# FOCUS");
	CAT_gui_line_break();
	CAT_gui_text
	(
		"is your pet's mental stat.\n"
		"A pet with high focus is\n"
		"attentive. Focus is restored\n"
		"by reading good books."
	);
	CAT_gui_line_break();

	CAT_gui_text("# SPIRIT");
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
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_gui_text("< CARETAKING > ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_image(icon_feed_sprite, 0);
	CAT_gui_image(padkaprow_sprite, 0);
	CAT_gui_image(icon_study_sprite, 0);
	CAT_gui_image(book_static_sprite, 0);
	CAT_gui_image(icon_play_sprite, 0);
	CAT_gui_image(toy_duck_sprite, 0);
	CAT_gui_line_break();

	CAT_gui_text("# FEEDING");
	CAT_gui_line_break();
	CAT_gui_text
	(
		"restores your pet's vigour.\n"
		"Select food from your bag\n"
		"and place it in the room\n"
		"for your pet to eat."
	);
	CAT_gui_line_break();
	
	CAT_gui_text("# STUDYING");
	CAT_gui_line_break();
	CAT_gui_text
	(
		"restores your pet's focus.\n"
		"Select a book from your bag\n"
		"and place it in the room for\n"
		"your pet to read."
	);
	CAT_gui_line_break();

	CAT_gui_text("# PLAYING");
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
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_gui_text("< DECORATION > ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_image(icon_deco_sprite, 0);
	CAT_gui_image(padkaprop_sprite, 0);
	CAT_gui_image(lantern_sprite, 0);
	CAT_gui_image(bush_daisy_sprite, 0);
	CAT_gui_image(crystal_purple_lg_sprite, 0);
	CAT_gui_image(succulent_sprite, 0);
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

	CAT_gui_image(cursor_add_sprite, 0);
	CAT_gui_image(cursor_flip_sprite, 0);
	CAT_gui_image(cursor_remove_sprite, 0);
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
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_gui_text("< AIR QUALITY > ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	CAT_gui_text
	(
		"Real-life air quality\n"
		"affects the rate at which\n"
		"your pet's stats degrade.\n"
		"Air quality factors are given\n"
		"a score of:\n"
	);
	CAT_gui_text("GOOD");
	CAT_gui_image(icon_aq_ccode_sprite, 0);
	CAT_gui_text(", NORMAL");
	CAT_gui_image(icon_aq_ccode_sprite, 1);
	CAT_gui_text(", and BAD");
	CAT_gui_image(icon_aq_ccode_sprite, 2);
	CAT_gui_line_break();
	CAT_gui_text("for each of\n");
	CAT_gui_image(icon_co2_sprite, 1);
	CAT_gui_text("CO2, ");
	CAT_gui_image(icon_nox_sprite, 1);
	CAT_gui_text("NOx, ");
	CAT_gui_image(icon_voc_sprite, 1);
	CAT_gui_text("VOCs, ");
	CAT_gui_image(icon_pm_sprite, 1);
	CAT_gui_text("PM2.5\n");

	CAT_gui_image(icon_mask_sprite, 0);
	CAT_gui_image(icon_pure_sprite, 0);
	CAT_gui_image(icon_uv_sprite, 0);
	CAT_gui_text("INTERVENTIONS");
	CAT_gui_line_break();

}

void CAT_draw_shopping()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_gui_text("< SHOPPING > ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
}

void CAT_draw_crypto()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_gui_text("< MINING > ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
}

void CAT_draw_arcade()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_gui_text("< ARCADE > ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
}

void CAT_draw_about()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
	CAT_gui_text("< ABOUT > ");
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
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
			CAT_draw_arcade();
			break;
		}
		default:
		{
			CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});
			CAT_gui_text("WHAT THE HELL...");
			CAT_gui_image(icon_b_sprite, 1);
			CAT_gui_image(icon_exit_sprite, 0);
			CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
			CAT_gui_text("How the did you even\nget here man? Try < or >\nor something I guess");
			break;
		}
	}
}