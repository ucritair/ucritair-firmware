#include "cat_monitors.h"

#include "cat_core.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_air.h"
#include "cat_monitor_graphics_utils.h"
#include "cat_graph.h"
#include "cat_persist.h"
#include "cat_colours.h"

#define TITLE_X 120
#define TITLE_Y 64

#define SPARKLINE_X 12
#define SPARKLINE_Y (TITLE_Y + 40)
#define SPARKLINE_PAD 4
#define SPARKLINE_WIDTH (CAT_LCD_SCREEN_W - 2 * SPARKLINE_X)
#define SPARKLINE_HEIGHT 120
#define SPARKLINE_MAX_X (SPARKLINE_X + SPARKLINE_WIDTH - 1)
#define SPARKLINE_MAX_Y (SPARKLINE_Y + SPARKLINE_HEIGHT - 1)
#define SPARKLINE_LABEL_X (SPARKLINE_MAX_X + SPARKLINE_PAD)

static int view = CAT_AQM_AGGREGATE;
static float samples[7];
static uint16_t colours[7];

void CAT_monitor_render_sparklines()
{
	CAT_draw_subpage_markers(32, CAT_AQM_COUNT, view, CAT_WHITE);

	const char* title = CAT_AQ_title_string(view);
	int cursor_y = center_textf(TITLE_X, TITLE_Y, 2, CAT_WHITE, title) + 2;
	cursor_y = center_textf(TITLE_X, cursor_y, 1, CAT_WHITE, "Weekly Performance");

	float min_score = INFINITY;
	float max_score = -INFINITY;
	float mean_score = 0;
	float last_score = 0;
	for(int i = 0; i < 7; i++)
	{
		CAT_AQ_score_block* block = CAT_AQ_get_weekly_scores(i);
		float score = CAT_AQ_block_score(block, view);
		if(fabs(score) <= __FLT_EPSILON__)
			score = 0;
		samples[i] = score;
		colours[i] = CAT_AQ_grade_colour(score);

		min_score = CAT_min(min_score, score);
		max_score = CAT_max(max_score, score);
		mean_score += score;
		last_score = score;
	}
	mean_score /= 7.0f;

	bool ready = !is_persist_fresh || CAT_AQ_sensors_initialized();

	CAT_graph_reset();
	CAT_graph_set_background(CAT_GRAPH_BG);
	CAT_graph_set_foreground(CAT_GRAPH_FG);
	CAT_graph_set_window(SPARKLINE_X, SPARKLINE_Y, SPARKLINE_MAX_X, SPARKLINE_MAX_Y);
	CAT_graph_set_viewport(0, -0.25, 1, 1.25);
	CAT_graph_set_point_size(3);
	CAT_graph_set_point_fill(true);
	CAT_graph_draw(samples, colours, ready ? 7 : 0);

	cursor_y = SPARKLINE_MAX_Y + 8;
	if(ready)
	{
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(SPARKLINE_X, cursor_y, "Max score: %s \1 %d/100\n", CAT_AQ_grade_string(max_score), (int)(max_score*100))+8;
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(SPARKLINE_X, cursor_y, "  Mean score: %s \1 %d/100\n", CAT_AQ_grade_string(mean_score), (int)(mean_score*100))+8;
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(SPARKLINE_X, cursor_y, "    Min score: %s \1 %d/100\n", CAT_AQ_grade_string(min_score), (int)(min_score*100))+8;
	}
	else
	{
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(SPARKLINE_X, cursor_y, "Please wait for sensors\nto come online.\n");
	}

	CAT_set_text_colour(CAT_96_GREY);
	cursor_y = CAT_draw_textf(SPARKLINE_X, CAT_LCD_SCREEN_H-16, "Tap graph to change metric");
}

void CAT_monitor_MS_sparklines(CAT_FSM_signal signal)
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
			
			if(CAT_input_touch_rect(SPARKLINE_X, SPARKLINE_Y, SPARKLINE_WIDTH, SPARKLINE_HEIGHT))
				view = (view+1) % CAT_AQM_COUNT;
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}