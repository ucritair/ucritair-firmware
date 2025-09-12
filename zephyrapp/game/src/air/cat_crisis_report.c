#include "cat_crisis.h"

#include "cat_input.h"
#include "cat_gui.h"
#include "cat_curves.h"
#include "cat_gizmos.h"
#include "cat_room.h"
#include "cat_pet.h"

static enum
{
	INTRO,
	DETAILS,
	OUTCOMES,
	PAGE_MAX
};
static int page = INTRO;

static float exit_progress = 0;

void CAT_MS_crisis_report(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_crisis_report);
			page = INTRO;
			exit_progress = 0;
		break;

		case CAT_FSM_SIGNAL_TICK:
			if(CAT_input_dismissal())
				page = OUTCOMES;
				
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				page += 1;
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				page -= 1;
			page = (page+PAGE_MAX)%PAGE_MAX;

			if(page == PAGE_MAX-1)
			{
				if(CAT_input_held(CAT_BUTTON_A, 0))
					exit_progress += CAT_get_delta_time_s();
				else
					exit_progress -= CAT_get_delta_time_s() * 1.5f;
				exit_progress = clamp(exit_progress, 0, 1);
				if(exit_progress >= 1.0f && CAT_input_time(CAT_BUTTON_A) >= 1.25f)
				{
					CAT_AQ_dismiss_crisis_report();
					CAT_pushdown_transition(CAT_MS_room);
				}
			}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

#define MARGIN 8

#define COUNTDOWN_W 64
#define COUNTDOWN_X (CAT_LCD_SCREEN_W-MARGIN-COUNTDOWN_W)
#define COUNTDOWN_Y MARGIN
#define COUNTDOWN_H 40

static void draw_countdown(int countdown)
{
	CAT_draw_corner_box
	(
		COUNTDOWN_X, COUNTDOWN_Y,
		COUNTDOWN_X+COUNTDOWN_W,
		COUNTDOWN_Y+COUNTDOWN_H,
		CAT_CRISIS_YELLOW
	);

	if(CAT_AQ_is_crisis_ongoing())
	{
		if(countdown > 0)
		{
			CAT_set_text_colour(CAT_CRISIS_RED);
			CAT_set_text_mask(COUNTDOWN_X, -1, COUNTDOWN_X+COUNTDOWN_W, -1);
			CAT_set_text_flags(CAT_TEXT_FLAG_WRAP | CAT_TEXT_FLAG_CENTER);
			if(countdown > 60 || CAT_pulse(0.25f))
			{
				
				CAT_draw_textf
				(
					COUNTDOWN_X+COUNTDOWN_W/2,
					COUNTDOWN_Y+MARGIN,
					"DANGER %.2d:%.2d\n",
					countdown / 60, countdown % 60
				);
			}
		}
		else
		{
			CAT_set_text_colour(CAT_CRISIS_RED);
			CAT_set_text_mask(COUNTDOWN_X, -1, COUNTDOWN_X+COUNTDOWN_W, -1);
			CAT_set_text_flags(CAT_TEXT_FLAG_WRAP | CAT_TEXT_FLAG_CENTER);
			CAT_draw_textf
			(
				COUNTDOWN_X+COUNTDOWN_W/2,
				COUNTDOWN_Y+MARGIN,
				"ONGOING %.2d:%.2d\n",
				-countdown / 60, -countdown % 60
			);
		}
	}
	else
	{
		CAT_set_text_colour(CAT_CRISIS_GREEN);
		CAT_set_text_mask(COUNTDOWN_X, -1, COUNTDOWN_X+COUNTDOWN_W, -1);
		CAT_set_text_flags(CAT_TEXT_FLAG_WRAP | CAT_TEXT_FLAG_CENTER);
		CAT_draw_textf
		(
			COUNTDOWN_X+COUNTDOWN_W/2,
			COUNTDOWN_Y+MARGIN,
			"ALL CLEAR\n"
		);
	}
}

void configure_description_text()
{
	CAT_set_text_mask(MARGIN*2+MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN*2-MARGIN-12, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
}

void draw_intro_page()
{
	CAT_frameberry(CAT_BLACK);

	int uptime = CAT_AQ_get_crisis_total_uptime();
	int countdown = -CAT_AQ_get_crisis_disaster_uptime();
	draw_countdown(countdown);

	int cursor_y = 12;

	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_colour(CAT_CRISIS_YELLOW);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "CRISIS REPORT:\n");

	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_colour(CAT_CRISIS_RED);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "THREAT: %s\n", CAT_AQ_crisis_type_string(-1));

	int peak_severity = CAT_AQ_get_crisis_peak_severity();
	CAT_set_text_colour(CAT_CRISIS_YELLOW);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "PEAK SEVERITY: %s\n", CAT_AQ_crisis_severity_string(peak_severity));

	CAT_datetime onset;
	CAT_make_datetime(CAT_AQ_get_crisis_start(), &onset);
	CAT_set_text_colour(CAT_CRISIS_RED);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "ONSET: %.2d/%.2d/%.4d %.2d:%.2d:%.2d\n", onset.month, onset.day, onset.year, onset.hour, onset.minute, onset.second);

	CAT_set_text_colour(CAT_CRISIS_GREEN);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, ">>>>>>>>>>>>>>>>>>>>>>>>>>");
	cursor_y += 36;
	int box_y = cursor_y-8;

	configure_description_text();
	CAT_set_text_colour(CAT_WHITE);
	if(CAT_AQ_is_crisis_ongoing())
	{
		cursor_y = CAT_draw_textf
		(
			MARGIN*2+MARGIN, cursor_y,
			"No response has yet been made. "
			"%s\n"
			,
			CAT_pet_is_dead() ?
			"A grave egg feels no pain." :
			countdown <= 0 ?
			"With disastrous conditions ongoing, the critter's health has begun to deteriorate." :
			"If conditions are not improved, the critter will suffer damaging outcomes to its health."
		);
	}
	else
	{
		cursor_y = CAT_draw_textf
		(
			MARGIN*2+MARGIN, cursor_y,
			"Crisis was mitigated %dm, %ds after onset. "
			"Mitigation time has been graded %s. "
			"%s\n"
			,
			uptime / 60, uptime % 60,
			CAT_AQ_crisis_response_grade_string(-1),
			CAT_pet_is_dead() ?
			"A grave egg feels no pain." :
			"With the crisis suppressed, the critter's condition is stable."
		);
	}

	CAT_draw_cross_box
	(
		MARGIN*2, box_y,
		CAT_LCD_SCREEN_W-MARGIN*2-12,
		cursor_y+MARGIN,
		CAT_AQ_is_crisis_ongoing() ? CAT_CRISIS_RED : CAT_CRISIS_GREEN
	);

	CAT_set_text_colour(CAT_CRISIS_GREEN);
	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
	CAT_draw_text(120, CAT_LCD_SCREEN_H-MARGIN-12, "<<<                     >>>");
	CAT_set_text_colour(CAT_CRISIS_YELLOW);
	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
	CAT_draw_text(120, CAT_LCD_SCREEN_H-MARGIN-12, "CHANGE PAGE");
}

void draw_details_page()
{
	CAT_frameberry(CAT_BLACK);

	int cursor_y = 12;

	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_colour(CAT_CRISIS_YELLOW);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "INTERVENTION:\n");

	if(CAT_AQ_is_crisis_ongoing())
	{
		CAT_set_text_colour(CAT_CRISIS_GREEN);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>");
		cursor_y += 24;

		int box_y = cursor_y;
		cursor_y += 8;

		configure_description_text();
		CAT_set_text_colour(CAT_CRISIS_YELLOW);
		cursor_y = CAT_draw_textf
		(
			MARGIN*2+MARGIN, cursor_y,
			"With no intervention having occurred, the crisis is ongoing.\n"		
			"\n"
			"To intervene in the crisis, enable the appropriate in game equipment or take action to improve conditions in the real world!\n"
		);
		CAT_draw_cross_box
		(
			MARGIN*2, box_y,
			CAT_LCD_SCREEN_W-MARGIN*2,
			cursor_y+MARGIN,
			CAT_CRISIS_RED
		);
	}
	else
	{
		int crisis_type = CAT_AQ_get_crisis_type();
		int peak_severity = CAT_AQ_get_crisis_peak_severity();
		int response_type = CAT_AQ_get_crisis_response_type();
		int response_time = CAT_AQ_get_crisis_total_uptime();
		int lifespan_damage = CAT_AQ_get_crisis_lifespan_damage();

		CAT_set_text_colour(CAT_CRISIS_RED);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, "TYPE: %s\n", CAT_AQ_crisis_response_type_string(response_type));

		CAT_set_text_colour(CAT_CRISIS_GREEN);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>");
		cursor_y += 24;

		int box_y = cursor_y;
		cursor_y += 8;

		configure_description_text();
		CAT_set_text_colour(CAT_CRISIS_YELLOW);
		switch(response_type)
		{
			case CAT_AQ_CRISIS_RESPONSE_TYPE_AUTOMATIC:
			cursor_y = CAT_draw_textf
			(
				MARGIN*2+MARGIN, cursor_y,
				"After %.2dm and %.2ds, "
				"crisis conditions alleviated on their own "
				"and the crisis ended.\n"
				"No resources were lost to crisis mitigation.\n",
				response_time / 60, response_time % 60
			);
			break;

			case CAT_AQ_CRISIS_RESPONSE_TYPE_MANUAL:
			cursor_y = CAT_draw_textf
			(
				MARGIN*2+MARGIN, cursor_y,
				"After %.2dm and %.2ds, "
				"crisis conditions were alleviated by your personal intervention.\n"
				"No resources were lost to crisis mitigation.\n",
				response_time / 60, response_time % 60
			);
			break;

			case CAT_AQ_CRISIS_RESPONSE_TYPE_ASSISTED:
			cursor_y = CAT_draw_textf
			(
				MARGIN*2+MARGIN, cursor_y,
				"After %.2dm and %.2ds, "
				"crisis conditions were alleviated at the cost of some valuable equipment, "
				"destroyed by the strain of mitigating poor air quality.\n",
				response_time / 60, response_time % 60
			);
			break;
		}

		if(!CAT_pet_is_dead())
		{
			configure_description_text();
			if(lifespan_damage > 0)
			{
				CAT_set_text_colour(CAT_CRISIS_RED);
				cursor_y = CAT_draw_textf
				(
					MARGIN*2+MARGIN, cursor_y,
					"%s unfortunately suffered negative health effects, resulting in a reduced lifespan.\n",
					pet.name
				);
			}
			else
			{
				CAT_set_text_colour(CAT_CRISIS_GREEN);
				cursor_y = CAT_draw_textf
				(
					MARGIN*2+MARGIN, cursor_y,
					"Thankfully, %s suffered no negative health effects.\n",
					pet.name
				);
			}
		}
		else
		{
			configure_description_text();
			CAT_set_text_colour(CAT_CRISIS_RED);
			cursor_y = CAT_draw_textf
			(
				MARGIN*2+MARGIN, cursor_y,
				"A grave egg feels no pain.\n"
			);
		}

		CAT_draw_cross_box
		(
			MARGIN*2, box_y,
			CAT_LCD_SCREEN_W-MARGIN*2,
			cursor_y+MARGIN,
			CAT_CRISIS_YELLOW
		);
	}

	CAT_set_text_colour(CAT_CRISIS_GREEN);
	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
	CAT_draw_text(120, CAT_LCD_SCREEN_H-MARGIN-12, "<<<                     >>>");
	CAT_set_text_colour(CAT_CRISIS_YELLOW);
	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
	CAT_draw_text(120, CAT_LCD_SCREEN_H-MARGIN-12, "CHANGE PAGE");
}

#define EXIT_BAR_X (CAT_LCD_SCREEN_W/2)
#define EXIT_BAR_H 16
#define EXIT_BAR_Y (CAT_LCD_SCREEN_H - EXIT_BAR_H/2 - MARGIN)
#define EXIT_BAR_W (CAT_LCD_SCREEN_W-MARGIN*2)

void draw_outcomes_page()
{
	CAT_frameberry(CAT_BLACK);

	int cursor_y = 12;

	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_colour(CAT_CRISIS_YELLOW);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "OUTCOMES:\n");

	if(CAT_AQ_is_crisis_ongoing())
	{
		CAT_set_text_colour(CAT_CRISIS_GREEN);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>");
		cursor_y += 24;

		int box_y = cursor_y;
		cursor_y += 8;

		configure_description_text();
		CAT_set_text_colour(CAT_CRISIS_YELLOW);
		cursor_y = CAT_draw_textf
		(
			MARGIN*2+MARGIN, cursor_y,
			"With the crisis ongoing, final outcomes have not been determined.\n"			
			"\n"
			"Return to the crisis report once crisis conditions have been alleviated.\n"
		);
		CAT_draw_cross_box
		(
			MARGIN*2, box_y,
			CAT_LCD_SCREEN_W-MARGIN*2,
			cursor_y+MARGIN,
			CAT_CRISIS_RED
		);
		cursor_y += 24;

		if(CAT_AQ_get_crisis_peak_severity() >= CAT_AQ_CRISIS_SEVERITY_EXTREME)
		{
			CAT_set_text_colour(CAT_CRISIS_RED);
			CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
			CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
			cursor_y = CAT_draw_textf
			(
				MARGIN, cursor_y,
				"PEAK CRISIS SEVERITY HAS REACHED \"EXTREME\" LEVELS. "
				"%s\n",
				CAT_pet_is_dead() ?
				"A GRAVE EGG FEELS NO PAIN." :
				"VERY HIGH RISK OF HEALTH EFFECTS ON CRITTER!"
			);
		}
	}
	else
	{
		int uptime = CAT_AQ_get_crisis_total_uptime();
		CAT_set_text_colour(CAT_CRISIS_RED);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, "RESPONSE TIME: %.2dm %.2ds\n", uptime/60, uptime%60);

		int grade = CAT_AQ_get_crisis_response_grade();
		CAT_set_text_colour(CAT_CRISIS_YELLOW);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, "RESPONSE GRADE: %s\n", CAT_AQ_crisis_response_grade_string(-1));

		CAT_set_text_colour(CAT_CRISIS_GREEN);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>");
		cursor_y += 24;

		int box_x0 = MARGIN*2; int box_y0 = cursor_y;
		int box_x1 = CAT_LCD_SCREEN_W-MARGIN*2; int box_y1 = CAT_LCD_SCREEN_H-64;
		CAT_draw_corner_box(box_x0, box_y0, box_x1, box_y1, CAT_CRISIS_YELLOW);

		if(!CAT_pet_is_dead())
		{
			uint16_t grade_colour =
			grade >= CAT_AQ_CRISIS_RESPONSE_GRADE_ADEQUATE ? CAT_CRISIS_GREEN :
			grade >= CAT_AQ_CRISIS_RESPONSE_GRADE_INADEQUATE ? CAT_CRISIS_YELLOW :
			CAT_CRISIS_RED;
			CAT_draw_gizmo_primitive(CAT_GIZMO_PRIMITIVE_HEX, (box_x0+box_x1)/2, (box_y0+box_y1)/2, 64, 0, CAT_WHITE);

			int damage = CAT_AQ_get_crisis_lifespan_damage();
			CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
			CAT_set_text_colour(CAT_WHITE);
			cursor_y = CAT_draw_textf
			(
				(box_x0+box_x1)/2, (box_y0+box_y1)/2 - (14*3/2),
				"LIFESPAN DAMAGE [%.2d]\n"
				"PREVIOUS LIFESPAN [%.2d]\n"
				"CURRENT LIFESPAN [%.2d]\n"
				,
				damage,
				pet.lifespan+damage, pet.lifespan
			);
		}
		else
		{
			CAT_draw_gizmo_primitive(CAT_GIZMO_PRIMITIVE_HEX, (box_x0+box_x1)/2, (box_y0+box_y1)/2, 64, -0.11f, CAT_WHITE);

			CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
			CAT_set_text_colour(CAT_WHITE);
			int damage = CAT_AQ_get_crisis_lifespan_damage();
			cursor_y = CAT_draw_textf
			(
				(box_x0+box_x1)/2, (box_y0+box_y1)/2 - (14*2/2),
				"A GRAVE EGG FEELS\n"
				"NO PAIN\n"
				,
				damage,
				pet.lifespan+damage, pet.lifespan
			);
		}
	}

	CAT_set_text_colour(CAT_CRISIS_GREEN);
	CAT_draw_textf
	(
		EXIT_BAR_X-EXIT_BAR_W/2,
		EXIT_BAR_Y-EXIT_BAR_H/2-16,
		"[A] CLOSE REPORT >>>>>>>>>>>"
	);
	CAT_draw_progress_bar(EXIT_BAR_X, EXIT_BAR_Y, EXIT_BAR_W, EXIT_BAR_H, CAT_CRISIS_YELLOW, CAT_CRISIS_GREEN, exit_progress);
}

void CAT_render_crisis_report()
{
	switch (page)
	{
		case INTRO: draw_intro_page(); return;
		case DETAILS: draw_details_page(); return;
		case OUTCOMES: draw_outcomes_page(); return;
	}
}