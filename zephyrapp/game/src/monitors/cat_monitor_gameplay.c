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

#define MARGIN 12

#define EXIT_BAR_X (CAT_LCD_SCREEN_W/2)
#define EXIT_BAR_H 20
#define EXIT_BAR_Y (CAT_LCD_SCREEN_H - EXIT_BAR_H/2 - MARGIN)
#define EXIT_BAR_W (CAT_LCD_SCREEN_W-MARGIN*2)

static float exit_progress = 0;

void render_exit_bar()
{
	CAT_strokeberry(EXIT_BAR_X-EXIT_BAR_W/2, EXIT_BAR_Y-EXIT_BAR_H/2, EXIT_BAR_W, EXIT_BAR_H, CRISIS_YELLOW);
	if(exit_progress > 0)
	{
		float t = CAT_ease_inout_sine(exit_progress);
		CAT_fillberry(EXIT_BAR_X-EXIT_BAR_W/2+2, EXIT_BAR_Y-EXIT_BAR_H/2+2, (EXIT_BAR_W-4)*t, EXIT_BAR_H-4, CRISIS_GREEN);
	}
}

void CAT_monitor_render_gameplay()
{
	if(CAT_AQ_is_crisis_ongoing())
	{
		CAT_monitor_colour_bg(CAT_BLACK);
		CAT_monitor_colour_fg(CRISIS_YELLOW);

		int cursor_y = 44;

		CAT_set_text_mask(MARGIN, -1, CAT_LCD_SCREEN_W-MARGIN, -1);
		CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
		CAT_set_text_scale(2);
		CAT_set_text_colour(CRISIS_YELLOW);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, "%s CRISIS\n", CAT_AQ_get_crisis_title());
		CAT_set_text_colour(CRISIS_RED);
		cursor_y = CAT_draw_textf(MARGIN, cursor_y, "%s SEVERITY\n", CAT_AQ_get_crisis_severity_string());

		cursor_y += 6;
		CAT_lineberry(MARGIN, cursor_y, CAT_LCD_SCREEN_W-MARGIN, cursor_y, CRISIS_GREEN);
		cursor_y += 10;

		int countdown = CAT_AQ_get_crisis_primetime() - CAT_AQ_get_crisis_uptime();
		if(countdown > 60 || CAT_pulse(0.25f))
		{
			CAT_set_text_colour(CRISIS_YELLOW);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf
			(
				MARGIN,
				cursor_y,
				"DISASTER IN\n"
			);
			CAT_set_text_colour(CRISIS_RED);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf
			(
				MARGIN,
				cursor_y,
				"%.2dm %.2ds\n",
				countdown / 60, countdown % 60
			);
		}

		CAT_set_text_colour(CRISIS_GREEN);
		CAT_draw_textf
		(
			EXIT_BAR_X-EXIT_BAR_W/2,
			EXIT_BAR_Y-EXIT_BAR_H/2-16,
			"[A] TO CRISIS REPORT >>>>>>"
		);
		render_exit_bar();
	}
}

void CAT_monitor_MS_gameplay(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			exit_progress = 0;
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_released(CAT_BUTTON_START))
				CAT_monitor_exit();
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				CAT_monitor_retreat();
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				CAT_monitor_advance();
			
			if(CAT_AQ_is_crisis_ongoing())
			{
				if(CAT_input_held(CAT_BUTTON_A, 0))
					exit_progress += CAT_get_delta_time_s();
				else
					exit_progress -= CAT_get_delta_time_s() * 2;
				exit_progress = clamp(exit_progress, 0, 1);
				if(exit_progress >= 1.0f && input.time[CAT_BUTTON_A] >= 1.25f)
					CAT_monitor_exit();
			}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}