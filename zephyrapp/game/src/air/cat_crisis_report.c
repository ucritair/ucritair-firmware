#include "cat_crisis.h"

#include "cat_input.h"
#include "cat_gui.h"
#include "cat_curves.h"
#include "cat_gizmos.h"
#include "cat_room.h"

static enum
{
	INTRO,
	OUTCOMES,
	PAGE_MAX
};
static int page = INTRO;

static float exit_progress = 0;

void CAT_MS_crisis_report(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_crisis_report);
			page = INTRO;
			exit_progress = 0;
		break;

		case CAT_MACHINE_SIGNAL_TICK:
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
				if(exit_progress >= 1.0f && input.time[CAT_BUTTON_A] >= 1.25f)
				{
					CAT_AQ_dismiss_crisis_notice();
					CAT_machine_transition(CAT_MS_room);
				}
			}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
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
		CRISIS_YELLOW
	);

	if(CAT_AQ_is_crisis_ongoing())
	{
		if(countdown > 0)
		{
			CAT_set_text_colour(CRISIS_RED);
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
			CAT_set_text_colour(CRISIS_RED);
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
		CAT_set_text_colour(CRISIS_GREEN);
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

	int uptime = CAT_AQ_get_crisis_uptime();
	int countdown = CAT_AQ_get_crisis_primetime() - uptime;
	draw_countdown(countdown);

	int cursor_y = 12;

	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_colour(CRISIS_YELLOW);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "CRISIS REPORT:\n");

	CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_set_text_colour(CRISIS_RED);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "THREAT: %s\n", CAT_AQ_get_crisis_title());

	CAT_set_text_colour(CRISIS_YELLOW);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "SEVERITY: %s\n", CAT_AQ_get_crisis_severity_string());

	CAT_datetime datetime;
	CAT_make_datetime(CAT_AQ_get_crisis_start(), &datetime);
	CAT_set_text_colour(CRISIS_RED);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "ONSET: %.2d/%.2d/%.4d %.2d:%.2d:%.2d\n", datetime.month, datetime.day, datetime.year, datetime.hour, datetime.minute, datetime.second);

	CAT_set_text_colour(CRISIS_GREEN);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, ">>>>>>>>>>>>>>>>>>>>>>>>>>");
	cursor_y += 36;
	int box_y = cursor_y-8;

	if(CAT_AQ_is_crisis_ongoing())
	{
		if(countdown <= 0)
		{
			configure_description_text();
			CAT_set_text_colour(CAT_WHITE);
			cursor_y = CAT_draw_textf
			(
				MARGIN*2+MARGIN, cursor_y,
				"No response has yet been made. "
				"With disastrous conditions ongoing, the critter's health has begun to deteriorate.\n"
			);
		}
		else
		{
			configure_description_text();
			CAT_set_text_colour(CAT_WHITE);
			cursor_y = CAT_draw_textf
			(
				MARGIN*2+MARGIN, cursor_y,
				"No response has yet been made. "
				"If conditions are not improved, the critter will suffer damaging outcomes to its health.\n"
			);
		}
	}
	else
	{
		configure_description_text();
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf
		(
			MARGIN*2+MARGIN, cursor_y,
			"Mitigation occurred %dm, %ds into the crisis. "
			"The mitigation has been graded %s. "
			"With the crisis suppressed, the critter's condition is stable.\n"
			,
			uptime / 60, uptime % 60,
			CAT_AQ_get_crisis_response_grade_string()
		);
	}

	CAT_draw_cross_box
	(
		MARGIN*2, box_y,
		CAT_LCD_SCREEN_W-MARGIN*2-12,
		cursor_y+MARGIN,
		CAT_AQ_is_crisis_ongoing() ? CRISIS_RED : CRISIS_GREEN
	);

	CAT_set_text_colour(CRISIS_GREEN);
	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
	CAT_draw_text(120, CAT_LCD_SCREEN_H-MARGIN-12, "<<<                     >>>");
	CAT_set_text_colour(CRISIS_YELLOW);
	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
	CAT_draw_text(120, CAT_LCD_SCREEN_H-MARGIN-12, "VIEW OUTCOMES");
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
	CAT_set_text_colour(CRISIS_YELLOW);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_textf(MARGIN, cursor_y, "OUTCOMES:\n");

	if(CAT_AQ_is_crisis_ongoing())
	{
		CAT_set_text_colour(CRISIS_GREEN);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>");
		cursor_y += 24;

		int box_y = cursor_y;
		cursor_y += 8;

		configure_description_text();
		CAT_set_text_colour(CRISIS_YELLOW);
		cursor_y = CAT_draw_textf
		(
			MARGIN*2+MARGIN, cursor_y,
			"With the crisis ongoing, no outcomes have yet been determined.\n"
			"\n"
			"Return to the crisis report once crisis conditions have been alleviated.\n"
		);
		CAT_draw_cross_box
		(
			MARGIN*2, box_y,
			CAT_LCD_SCREEN_W-MARGIN*2,
			cursor_y+MARGIN,
			CRISIS_RED
		);
	}
	else
	{
		int uptime = CAT_AQ_get_crisis_uptime();
		CAT_set_text_colour(CRISIS_RED);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, "RESPONSE TIME: %.2dm %.2ds\n", uptime/60, uptime%60);

		CAT_set_text_colour(CRISIS_YELLOW);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, "RESPONSE GRADE: %s\n", CAT_AQ_get_crisis_response_grade_string());

		CAT_set_text_colour(CRISIS_RED);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, "INTERVENTION TYPE: %s\n", CAT_AQ_get_crisis_response_type_string());

		CAT_set_text_colour(CRISIS_GREEN);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>");
		cursor_y += 24;
	}

	CAT_set_text_colour(CRISIS_GREEN);
	CAT_draw_textf
	(
		EXIT_BAR_X-EXIT_BAR_W/2,
		EXIT_BAR_Y-EXIT_BAR_H/2-16,
		"[A] CLOSE REPORT >>>>>>>>>>>"
	);
	CAT_draw_progress_bar(EXIT_BAR_X, EXIT_BAR_Y, EXIT_BAR_W, EXIT_BAR_H, CRISIS_YELLOW, CRISIS_GREEN, exit_progress);
}

void CAT_render_crisis_report()
{
	switch (page)
	{
		case INTRO: draw_intro_page(); return;
		case OUTCOMES: draw_outcomes_page(); return;
	}
}