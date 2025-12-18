#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include "cat_air.h"
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include "cat_curves.h"
#include "sprite_assets.h"
#include "cat_monitor_graphics_utils.h"
#include "cat_crisis.h"

#define TITLE_Y 44

#define SCORE_R 40

#define BAR_MARGIN 12
#define BAR_W (CAT_LCD_SCREEN_W - BAR_MARGIN*2)

#define DOT_COUNT (CAT_AQM_COUNT-1)
#define DOT_MARGIN 8
#define DOT_D ((BAR_W - (DOT_COUNT-1) * DOT_MARGIN) / DOT_COUNT)

void CAT_monitor_render_summary()
{
	int cursor_y = TITLE_Y;
	cursor_y = center_textf(120, cursor_y, 1, CAT_WHITE, "Summary");
	cursor_y = underline(120, cursor_y, 1, CAT_WHITE, "Summary");
	cursor_y += 48;

	float score = CAT_AQ_score(CAT_AQM_AGGREGATE);
	uint16_t colour = CAT_AQ_grade_colour(score);
	CAT_circberry(120, cursor_y, SCORE_R, colour);
	const char* grade = CAT_AQ_grade_string(score);
	cursor_y = center_textf(120 + ((strlen(grade) == 1) ? 0 : 4), cursor_y, 3, colour, "%s", grade);
	cursor_y += 24;

	cursor_y = center_textf(120, cursor_y, 1, CAT_WHITE, "\4CritAQ Grade");
	cursor_y += 44;

	for(int i = 0; i < CAT_AQM_AGGREGATE; i++)
	{
		score = CAT_AQ_score(i);
		colour = CAT_AQ_grade_colour(score);

		int x = BAR_MARGIN + (DOT_D + DOT_MARGIN) * i + DOT_D/2;
		int y = cursor_y + DOT_D/2;
		
		vert_text(x, cursor_y+DOT_D+DOT_MARGIN, CAT_WHITE, CAT_AQ_title_string(i));

		CAT_circberry(x, y, DOT_D/2, colour);
		CAT_set_text_colour(colour);
		grade = CAT_AQ_grade_string(score);
		CAT_draw_text(x - (strlen(grade) == 1 ? 4 : 6), y-6, CAT_AQ_grade_string(score));

		if(i == CAT_AQM_TEMP)
			center_textf(x, y - 20 - DOT_MARGIN, 1, CAT_WHITE, "%d", (int) (CAT_AQ_map_celsius(CAT_AQ_value(i))));
		else
			center_textf(x, y - 20 - DOT_MARGIN, 1, CAT_WHITE, "%d", (int) (CAT_AQ_value(i)));
	}
}

void CAT_monitor_MS_summary(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		break;

		case CAT_FSM_SIGNAL_TICK:
			if(CAT_input_dismissal())
				CAT_monitor_dismiss();
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				CAT_monitor_retreat();
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				CAT_monitor_advance();
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}