#include "cat_monitors.h"

#include "cat_core.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_aqi.h"
#include "cat_monitor_graphics_utils.h"
#include "cat_graph.h"

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

#define SPARKLINE_BG_COLOUR 0xe7bf
#define SPARKLINE_FG_COLOUR 0x6b11

static int view = CAT_AQM_AGGREGATE;
static float samples[7];
static uint16_t colours[7];

void CAT_monitor_render_sparklines()
{
	draw_subpage_markers(32, CAT_AQM_COUNT, view);

	const char* title = CAT_AQ_get_title_string(view);
	int cursor_y = center_textf(TITLE_X, TITLE_Y, 2, CAT_WHITE, title) + 2;
	cursor_y = center_textf(TITLE_X, cursor_y, 1, CAT_WHITE, "Weekly Performance");

	float min_score = INFINITY;
	float max_score = -INFINITY;
	float mean_score = 0;
	float last_score = 0;
	for(int i = 0; i < 7; i++)
	{
		CAT_AQ_score_block block;
		CAT_AQ_read_scores(i, &block);
		float score = CAT_AQ_block_score_normalized(&block, view);
		samples[i] = score;
		colours[i] = CAT_AQ_get_grade_colour(score);

		min_score = min(min_score, score);
		max_score = max(max_score, score);
		mean_score += score;
		last_score = score;
	}
	mean_score /= 7.0f;

	CAT_graph_set_background(SPARKLINE_BG_COLOUR);
	CAT_graph_set_foreground(SPARKLINE_FG_COLOUR);
	CAT_graph_set_window(SPARKLINE_X, SPARKLINE_Y, SPARKLINE_MAX_X, SPARKLINE_MAX_Y);
	CAT_graph_set_auto_viewport(true);
	CAT_graph_set_point_size(2);
	CAT_graph_set_point_fill(true);
	CAT_graph_draw(samples, colours, 7);

	cursor_y = SPARKLINE_MAX_Y + 8;
	CAT_set_text_colour(CAT_WHITE);
	cursor_y = CAT_draw_textf(SPARKLINE_X, cursor_y, "Max score: %s (%.1f)\n", CAT_AQ_get_grade_string(max_score), max_score)+8;
	CAT_set_text_colour(CAT_WHITE);
	cursor_y = CAT_draw_textf(SPARKLINE_X, cursor_y, "  Mean score: %s (%.1f)\n", CAT_AQ_get_grade_string(max_score), mean_score)+8;
	CAT_set_text_colour(CAT_WHITE);
	cursor_y = CAT_draw_textf(SPARKLINE_X, cursor_y, "    Min score: %s (%.1f)\n", CAT_AQ_get_grade_string(max_score), min_score)+8;
}

void CAT_monitor_MS_sparklines(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_dismissal())
				CAT_monitor_soft_exit();
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				CAT_monitor_retreat();
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				CAT_monitor_advance();
			
			if(CAT_input_touch_rect(SPARKLINE_X, SPARKLINE_Y, SPARKLINE_WIDTH, SPARKLINE_HEIGHT))
				view = (view+1) % CAT_AQM_COUNT;
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}