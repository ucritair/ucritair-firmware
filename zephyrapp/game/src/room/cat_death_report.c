#include "cat_pet.h"

#include "cat_input.h"
#include "cat_room.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_gizmos.h"

static float exit_progress = 0;

void CAT_MS_death_report(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_death_report);
			exit_progress = 0;
		break;

		case CAT_FSM_SIGNAL_TICK:
			if(CAT_input_held(CAT_BUTTON_A, 0))
				exit_progress += CAT_get_delta_time_s();
			else
				exit_progress -= CAT_get_delta_time_s() * 1.5f;
			exit_progress = clamp(exit_progress, 0, 1);
			if(exit_progress >= 1.0f && input.time[CAT_BUTTON_A] >= 1.25f)
			{
				CAT_pet_dismiss_death_report();
				CAT_pushdown_transition(CAT_MS_room);
			}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

#define MARGIN 8
#define EXIT_BAR_X (CAT_LCD_SCREEN_W/2)
#define EXIT_BAR_H 16
#define EXIT_BAR_Y (CAT_LCD_SCREEN_H - EXIT_BAR_H/2 - MARGIN)
#define EXIT_BAR_W (CAT_LCD_SCREEN_W-MARGIN*2)

void CAT_render_death_report()
{
	CAT_frameberry(CAT_BLACK);

	int cursor_y = 12;

	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_colour(CAT_CRISIS_RED);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "DEATH REPORT:\n");

	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_colour(CAT_CRISIS_YELLOW);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "LIVED: %d DAYS\n", pet.lifespan);

	CAT_set_text_colour(CAT_CRISIS_GREEN);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	cursor_y += 28;
	int box_y = cursor_y-8;

	CAT_set_text_mask(MARGIN+4, -1, CAT_LCD_SCREEN_W-MARGIN-4, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_colour(CAT_WHITE);
	cursor_y = CAT_draw_textf
	(
		MARGIN+4, cursor_y,
		"%s has lost the animating flame of life. "
		"Now cold and still, something that is not quite %s waits for life to begin anew. "
		"The spark yet remains, and air will coax it to flame. "
		"Once more, a critter will be born.\n",
		pet.name, pet.name
	);

	CAT_draw_corner_box
	(
		MARGIN, box_y,
		CAT_LCD_SCREEN_W-MARGIN,
		cursor_y+MARGIN,
		CAT_GREY
	);

	CAT_set_text_colour(CAT_CRISIS_GREEN);
	CAT_draw_textf
	(
		EXIT_BAR_X-EXIT_BAR_W/2,
		EXIT_BAR_Y-EXIT_BAR_H/2-16,
		"[A] CLOSE REPORT >>>>>>>>>>>"
	);
	CAT_draw_progress_bar(EXIT_BAR_X, EXIT_BAR_Y, EXIT_BAR_W, EXIT_BAR_H, CAT_CRISIS_YELLOW, CAT_CRISIS_GREEN, exit_progress);
}