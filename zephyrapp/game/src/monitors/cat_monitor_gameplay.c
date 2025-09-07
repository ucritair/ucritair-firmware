#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include "cat_aqi.h"
#include "mesh2d_assets.h"
#include "sprite_assets.h"
#include "cat_crisis.h"
#include "cat_curves.h"
#include "cat_gizmos.h"
#include "cat_pet.h"

#define MARGIN 12

#define EXIT_BAR_X (CAT_LCD_SCREEN_W/2)
#define EXIT_BAR_H 20
#define EXIT_BAR_Y (CAT_LCD_SCREEN_H - EXIT_BAR_H/2 - MARGIN)
#define EXIT_BAR_W (CAT_LCD_SCREEN_W-MARGIN*2)

static float exit_progress = 0;

void draw_ongoing_crisis()
{
	int cursor_y = 44;

	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_scale(2);
	CAT_set_text_colour(CAT_CRISIS_YELLOW);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "%s CRISIS\n", CAT_AQ_crisis_type_string(-1));
	CAT_set_text_colour(CAT_CRISIS_RED);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "%s SEVERITY\n", CAT_AQ_crisis_severity_string(-1));

	cursor_y += 8;
	CAT_set_text_colour(CAT_CRISIS_GREEN);
	CAT_draw_textf(MARGIN, cursor_y, ">>>>>>>>>>>>>>>>>>>>>>>>>>>");
	cursor_y += 22;

	int countdown = -CAT_AQ_get_crisis_disaster_uptime();
	if(countdown > 0)
	{
		CAT_set_text_colour(CAT_CRISIS_YELLOW);
		CAT_set_text_scale(2);
		cursor_y = CAT_draw_textf
		(
			MARGIN,
			cursor_y,
			"DISASTER IN\n"
		);
		if(countdown > 60 || CAT_pulse(0.25f))
		{
			CAT_set_text_colour(CAT_CRISIS_RED);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf
			(
				MARGIN,
				cursor_y,
				"%.2dm %.2ds\n",
				countdown / 60, countdown % 60
			);
		}

		if(!CAT_pet_is_dead())
		{
			CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
			CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
			CAT_set_text_colour(CAT_CRISIS_YELLOW);
			CAT_draw_text(MARGIN, CAT_LCD_SCREEN_H-92, "CRITTER AT RISK! ALLEVIATE CRISIS CONDITIONS IMMEDIATELY!\n");
		}
		else
		{
			CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
			CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
			CAT_set_text_colour(CAT_CRISIS_YELLOW);
			CAT_draw_text(MARGIN, CAT_LCD_SCREEN_H-92, "A GRAVE EGG FEELS NO PAIN\n");
		}
	}
	else
	{
		if(CAT_pulse(1))
		{
			CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
			CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
			CAT_set_text_colour(CAT_CRISIS_RED);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf
			(
				MARGIN,
				cursor_y,
				"DISASTER ONGOING FOR %.2dm %.2ds\n",
				-countdown / 60, -countdown % 60
			);
		}
	}

	CAT_set_text_colour(CAT_CRISIS_GREEN);
	CAT_draw_textf
	(
		EXIT_BAR_X-EXIT_BAR_W/2,
		EXIT_BAR_Y-EXIT_BAR_H/2-16,
		"[A] TO CRISIS REPORT >>>>>>"
	);
	CAT_draw_progress_bar(EXIT_BAR_X, EXIT_BAR_Y, EXIT_BAR_W, EXIT_BAR_H, CAT_CRISIS_YELLOW, CAT_CRISIS_GREEN, exit_progress);
}

void draw_waiting_crisis()
{
	int cursor_y = 44;

	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_scale(2);
	CAT_set_text_colour(CAT_CRISIS_YELLOW);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "RESOLVED %s CRISIS\n", CAT_AQ_crisis_type_string(-1));
	CAT_set_text_colour(CAT_CRISIS_RED);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "%s SEVERITY\n", CAT_AQ_crisis_severity_string(CAT_AQ_get_crisis_peak_severity()));

	cursor_y += 8;
	CAT_set_text_colour(CAT_CRISIS_GREEN);
	CAT_draw_textf(MARGIN, cursor_y, ">>>>>>>>>>>>>>>>>>>>>>>>>>>");
	cursor_y += 22;

	CAT_set_text_colour(CAT_CRISIS_YELLOW);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_textf
	(
		MARGIN,
		cursor_y,
		"RESPONSE:\n"
	);

	CAT_AQ_crisis_response_grade grade = CAT_AQ_get_crisis_response_grade();
	uint16_t grade_colour =
	grade >= CAT_AQ_CRISIS_RESPONSE_GRADE_ADEQUATE ? CAT_CRISIS_GREEN :
	grade >= CAT_AQ_CRISIS_RESPONSE_GRADE_INADEQUATE ? CAT_CRISIS_YELLOW :
	CAT_CRISIS_RED;
	CAT_set_text_colour(grade_colour);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_textf
	(
		MARGIN,
		cursor_y,
		"%s\n",
		CAT_AQ_crisis_response_grade_string(-1)
	);

	if(!CAT_pet_is_dead())
	{
		int damage = CAT_AQ_get_crisis_lifespan_damage();
		CAT_set_text_colour(damage <= 0 ? CAT_CRISIS_GREEN : CAT_CRISIS_RED);
		CAT_draw_text
		(
			EXIT_BAR_X-EXIT_BAR_W/2,
			EXIT_BAR_Y-EXIT_BAR_H/2-32,
			damage > 0 ?
			"CRITTER HAS SUFFERED DAMAGE\n" : "ALL CRITTER DAMAGE AVOIDED\n"
		);
	}

	CAT_set_text_colour(CAT_CRISIS_GREEN);
	CAT_draw_textf
	(
		EXIT_BAR_X-EXIT_BAR_W/2,
		EXIT_BAR_Y-EXIT_BAR_H/2-16,
		"[A] TO CRISIS REPORT >>>>>>"
	);
	CAT_draw_progress_bar(EXIT_BAR_X, EXIT_BAR_Y, EXIT_BAR_W, EXIT_BAR_H, CAT_CRISIS_YELLOW, CAT_CRISIS_GREEN, exit_progress);
}

void draw_all_clear()
{
	int cursor_y = 44;

	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_scale(2);
	CAT_set_text_colour(CAT_CRISIS_GREEN);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "ALL CLEAR\n", CAT_AQ_crisis_type_string(-1));
	CAT_set_text_colour(CAT_CRISIS_YELLOW);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "NO CRISIS DETECTED\n");

	cursor_y += 8;
	CAT_set_text_colour(CAT_CRISIS_GREEN);
	CAT_draw_textf(MARGIN, cursor_y, ">>>>>>>>>>>>>>>>>>>>>>>>>>>");
	cursor_y += 22;

	CAT_draw_gizmo_primitive(CAT_GIZMO_PRIMITIVE_HEX, 120, cursor_y + 64, 64, 0, CAT_CRISIS_GREEN);

	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_colour(CAT_CRISIS_YELLOW);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_textf
	(
		MARGIN,
		cursor_y,
		"ALWAYS ON STANDBY\n"
	);

	int box_x0 = MARGIN+4; int box_x1 =  CAT_LCD_SCREEN_W-MARGIN-4;
	int box_y0 = cursor_y + 8; int box_y1 = CAT_LCD_SCREEN_H-64;
	CAT_draw_cross_box(box_x0, box_y0, box_x1, box_y1, CAT_CRISIS_YELLOW);

	CAT_set_text_mask(box_x0+4, box_y0+4, box_x1, box_y1-4);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_text
	(
		box_x0+8, box_y0+12,
		"CONDITIONS ARE NORMAL. IN CASE OF CRITICAL CONDITIONS CRISIS INTERFACE WILL APPEAR.\n"
	);

	CAT_set_text_colour(CAT_CRISIS_GREEN);
	CAT_draw_textf
	(
		EXIT_BAR_X-EXIT_BAR_W/2,
		EXIT_BAR_Y-EXIT_BAR_H/2-16,
		"[A] TO INNERWORLD >>>>>>"
	);
	CAT_draw_progress_bar(EXIT_BAR_X, EXIT_BAR_Y, EXIT_BAR_W, EXIT_BAR_H, CAT_CRISIS_YELLOW, CAT_CRISIS_GREEN, exit_progress);
}

void CAT_monitor_render_gameplay()
{
	if(CAT_AQ_is_crisis_ongoing())
	{
		draw_ongoing_crisis();
	}
	else if(CAT_AQ_is_crisis_waiting())
	{
		draw_waiting_crisis();
	}
	else
	{
		draw_all_clear();
	}
}

void CAT_monitor_MS_gameplay(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			exit_progress = 0;
		break;

		case CAT_FSM_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				CAT_monitor_retreat();
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				CAT_monitor_advance();

			if(CAT_input_held(CAT_BUTTON_A, 0))
				exit_progress += CAT_get_delta_time_s();
			else
				exit_progress -= CAT_get_delta_time_s() * 1.5f;
			exit_progress = clamp(exit_progress, 0, 1);
			if(exit_progress >= 1.0f && input.time[CAT_BUTTON_A] >= 1.25f)
			{
				CAT_monitor_exit();
			}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}