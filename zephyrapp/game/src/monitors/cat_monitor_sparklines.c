#include "cat_monitors.h"

#include "cat_core.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_aqi.h"
#include "cat_monitor_graphics_utils.h"

#define TITLE_X 120
#define TITLE_Y 64

#define GRAPH_X 12
#define GRAPH_Y (TITLE_Y + 40)
#define GRAPH_PAD 4
#define GRAPH_WIDTH (CAT_LCD_SCREEN_W - 2 * GRAPH_X)
#define GRAPH_HEIGHT 120
#define GRAPH_MAX_X (GRAPH_X + GRAPH_WIDTH - 1)
#define GRAPH_MAX_Y (GRAPH_Y + GRAPH_HEIGHT - 1)
#define GRAPH_STEP_SIZE (round(GRAPH_WIDTH / 6.0f))
#define GRAPH_LABEL_X (GRAPH_MAX_X + GRAPH_PAD)

#define GRAPH_BG_COLOUR 0xe7bf
#define GRAPH_FG_COLOUR 0x6b11
#define GRAPH_RETICLE_COLOUR 0xadfa

static int view = CAT_AQM_AGGREGATE;

static uint8_t get_sparkline_value(int idx, CAT_AQ_score_block* block)
{
	idx -= 1;
	switch(idx)
	{
		case CAT_AQM_CO2: return block->CO2;
		case CAT_AQM_PM2_5: return block->PM2_5;
		case CAT_AQM_NOX: return block->NOX;
		case CAT_AQM_VOC: return block->VOC;
		case CAT_AQM_TEMP: return block->temp;
		case CAT_AQM_RH : return block->rh;
		case CAT_AQM_AGGREGATE: return block->aggregate;
		default: return 0;
	}
}

static int sparkline_value_to_y(int idx, uint8_t value)
{
	float t = inv_lerp(value, 0, 255);
	int h = t * GRAPH_HEIGHT;
	return GRAPH_MAX_Y - h;
}

void CAT_monitor_render_sparklines()
{
	draw_subpage_markers(32, CAT_AQM_COUNT, view);

	int cursor_y = center_textf(TITLE_X, TITLE_Y, 2, CAT_WHITE, CAT_AQM_titles[view]) + 2;
	cursor_y = center_textf(TITLE_X, cursor_y, 1, CAT_WHITE, "Weekly Score Trend");

	CAT_fillberry(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, GRAPH_BG_COLOUR);
	CAT_CSCLIP_set_rect(GRAPH_X, GRAPH_Y, GRAPH_MAX_X, GRAPH_MAX_Y);
	for(int j = 0; j < 6; j++)
	{
		CAT_AQ_score_block s0;
		CAT_AQ_read_stored_scores(j, &s0);
		CAT_AQ_score_block s1;
		CAT_AQ_read_stored_scores(j+1, &s1);

		int x0 = GRAPH_X + GRAPH_STEP_SIZE * j;
		int y0 = sparkline_value_to_y(view, get_sparkline_value(view, &s0));
		int x1 = GRAPH_X + GRAPH_STEP_SIZE * (j+1);
		int y1 = sparkline_value_to_y(view, get_sparkline_value(view, &s1));

		if(CAT_CSCLIP(&x0, &y0, &x1, &y1))
		{
			CAT_lineberry(x0, y0, x1, y1, GRAPH_FG_COLOUR);
			CAT_discberry(x0, y0, 2, GRAPH_FG_COLOUR);
			CAT_discberry(x1, y1, 2, GRAPH_FG_COLOUR);
		}
	}

	CAT_AQ_score_block last;
	CAT_AQ_read_stored_scores(6, &last);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_mask(GRAPH_X, -1, GRAPH_MAX_X, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	cursor_y = CAT_draw_textf
	(
		GRAPH_X, GRAPH_MAX_Y + 8,
		"The last recorded score for this week is %.1f."
		" Scores this week have ranged from %.1f to %.1f,"
		" with a mean of %.1f."
		,
		get_sparkline_value(view, &last),
		0.0f, 1.0f, 0.5f
	);
}

void CAT_monitor_MS_sparklines(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_released(CAT_BUTTON_START))
				CAT_monitor_exit();
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				CAT_monitor_retreat();
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				CAT_monitor_advance();
			
			if(CAT_input_touch_rect(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT))
				view = (view+1) % CAT_AQM_COUNT;

			CAT_AQ_store_normalized_scores();
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}