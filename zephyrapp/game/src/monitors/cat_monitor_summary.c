#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include "cat_aqi.h"
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include "cat_curves.h"
#include "sprite_assets.h"
#include "cat_monitor_graphics_utils.h"

#define TITLE_Y 44

#define SCORE_R1 40
#define SCORE_W 6
#define SCORE_R0 (SCORE_R1 - SCORE_W)

#define BAR_MARGIN 12
#define BAR_W (CAT_LCD_SCREEN_W - BAR_MARGIN*2)

#define DOT_COUNT (CAT_AQM_COUNT-1)
#define DOT_MARGIN 6
#define DOT_D ((BAR_W - (DOT_COUNT-1) * DOT_MARGIN) / DOT_COUNT)

enum
{
	SUMMARY,
	DETAILS
} mode;

void CAT_monitor_render_summary()
{
	int cursor_y = TITLE_Y;
	cursor_y = center_textf(120, cursor_y, 1, CAT_WHITE, "Summary");
	cursor_y = underline(120, cursor_y, 1, CAT_WHITE, "Summary");

	cursor_y += 56;

	CAT_ringberry
	(
		120, cursor_y,
		SCORE_R1, SCORE_R0,
		colour_score(CAT_AQ_normalized_scores[CAT_AQM_AGGREGATE]),
		CAT_AQ_normalized_scores[CAT_AQM_AGGREGATE],
		1.25f-CAT_AQ_normalized_scores[CAT_AQM_AGGREGATE]*0.5f
	);
	cursor_y = center_textf(120, cursor_y, 3, CAT_WHITE, "%.0f", CAT_aq_aggregate_score());
	if(CAT_aq_aggregate_score() >= 78)
		cursor_y += 4;
	cursor_y = center_textf(120, cursor_y, 1, CAT_WHITE, "\4CritAQ Score");
	
	cursor_y += 32;

	for(int i = 0; i < CAT_AQM_AGGREGATE; i++)
	{
		int x = BAR_MARGIN + (DOT_D + DOT_MARGIN) * i + DOT_D/2;
		int y = cursor_y + DOT_D/2;
		CAT_circberry(x, y, DOT_D/3, CAT_WHITE);
		vert_text(x, cursor_y+DOT_D-1, CAT_WHITE, CAT_AQM_titles[i]);

		uint16_t dot_colour = colour_score(CAT_AQ_normalized_scores[i]);
		CAT_discberry(x, y, DOT_D/4, dot_colour);

		center_textf(x, y - 20, 1, CAT_WHITE, "%.0f", CAT_AQ_raw_scores[i]);
	}
}

void CAT_monitor_MS_summary(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_monitor_exit();
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				CAT_monitor_retreat();
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				CAT_monitor_advance();

			CAT_AQ_store_raw_scores();
			CAT_AQ_store_normalized_scores();
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}