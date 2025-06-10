#include "cat_monitors.h"

#include "cat_core.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_aqi.h"
#include "cat_monitor_graphics_utils.h"

#define TITLE_X 120
#define TITLE_Y 64

#define SPARKLINE_X 12
#define SPARKLINE_Y (TITLE_Y + 40)
#define SPARKLINE_PAD 4
#define SPARKLINE_WIDTH (CAT_LCD_SCREEN_W - 2 * SPARKLINE_X)
#define SPARKLINE_HEIGHT 120
#define SPARKLINE_MAX_X (SPARKLINE_X + SPARKLINE_WIDTH - 1)
#define SPARKLINE_MAX_Y (SPARKLINE_Y + SPARKLINE_HEIGHT - 1)
#define SPARKLINE_STEP_SIZE (round(SPARKLINE_WIDTH / 6.0f))
#define SPARKLINE_LABEL_X (SPARKLINE_MAX_X + SPARKLINE_PAD)

#define SPARKLINE_BG_COLOUR 0xe7bf
#define SPARKLINE_FG_COLOUR 0x6b11
#define SPARKLINE_RETICLE_COLOUR 0xadfa

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
	int h = t * SPARKLINE_HEIGHT;
	return SPARKLINE_MAX_Y - h;
}

void CAT_monitor_render_sparklines()
{
	draw_subpage_markers(32, CAT_AQM_COUNT, view);

	int cursor_y = center_textf(TITLE_X, TITLE_Y, 2, CAT_WHITE, CAT_AQM_titles[view]) + 2;
	cursor_y = center_textf(TITLE_X, cursor_y, 1, CAT_WHITE, "Weekly Score Trend");

	CAT_fillberry(SPARKLINE_X, SPARKLINE_Y, SPARKLINE_WIDTH, SPARKLINE_HEIGHT, SPARKLINE_BG_COLOUR);
	CAT_CSCLIP_set_rect(SPARKLINE_X, SPARKLINE_Y, SPARKLINE_MAX_X, SPARKLINE_MAX_Y);
	for(int j = 0; j < 6; j++)
	{
		CAT_AQ_score_block s0;
		CAT_AQ_read_stored_scores(j, &s0);
		CAT_AQ_score_block s1;
		CAT_AQ_read_stored_scores(j+1, &s1);

		int x0 = SPARKLINE_X + SPARKLINE_STEP_SIZE * j;
		int y0 = sparkline_value_to_y(view, get_sparkline_value(view, &s0));
		int x1 = SPARKLINE_X + SPARKLINE_STEP_SIZE * (j+1);
		int y1 = sparkline_value_to_y(view, get_sparkline_value(view, &s1));

		if(CAT_CSCLIP(&x0, &y0, &x1, &y1))
		{
			CAT_lineberry(x0, y0, x1, y1, SPARKLINE_FG_COLOUR);
			CAT_discberry(x0, y0, 2, SPARKLINE_FG_COLOUR);
			CAT_discberry(x1, y1, 2, SPARKLINE_FG_COLOUR);
		}
	}

	CAT_AQ_score_block last;
	CAT_AQ_read_stored_scores(6, &last);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_mask(SPARKLINE_X, -1, SPARKLINE_MAX_X, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	
	cursor_y = CAT_draw_textf
	(
		SPARKLINE_X, SPARKLINE_MAX_Y + 8,
		"The last recorded score for this week is %.1f."
		" Scores this week have ranged from %.1f to %.1f,"
		" with a mean of %.1f."
		,
		inv_lerp(get_sparkline_value(view, &last), 0, 255),
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
			
			if(CAT_input_touch_rect(SPARKLINE_X, SPARKLINE_Y, SPARKLINE_WIDTH, SPARKLINE_HEIGHT))
				view = (view+1) % CAT_AQM_COUNT;

			CAT_AQ_store_normalized_scores();
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}