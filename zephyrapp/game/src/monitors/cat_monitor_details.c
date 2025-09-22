#include "cat_monitors.h"

#include "cat_core.h"
#include "cat_render.h"
#include "cat_gui.h"
#include <stdarg.h>
#include "cat_aqi.h"
#include "cat_monitor_graphics_utils.h"

#define TITLE_Y 44

int draw_title(int x, int y, const char* title)
{
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_text(x, y-CAT_GLYPH_HEIGHT, title);
	return x + strlen(title) * CAT_GLYPH_WIDTH + 8;
}

int draw_value(int x, int y, uint16_t c, char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(textf_buf, 32, fmt, args);
	va_end(args);

	CAT_set_text_scale(2);
	CAT_set_text_colour(c);
	CAT_draw_text(x, y-CAT_GLYPH_HEIGHT*2, textf_buf);
	return x + strlen(textf_buf) * CAT_GLYPH_WIDTH*2 + 8;
}

int draw_unit(int x, int y, const char* unit)
{
	int l = strlen(unit);
	if(l == 0)
		return x;
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_text(x, y-CAT_GLYPH_HEIGHT, unit);
	return x + l * CAT_GLYPH_WIDTH + 12;
}

int draw_sparkline(int x, int y, int aqm)
{
	if(true)
	{
		float samples[7];
		uint16_t colours[7];
		for(int i = 0; i < 7; i++)
		{
			CAT_AQ_score_block* block = CAT_AQ_score_buffer_get(i);
			float score = CAT_AQ_block_score_normalized(block, aqm);
			if(fabs(score) <= __FLT_EPSILON__)
				score = 0;
			samples[i] = 1.0f-score;
			colours[i] = CAT_AQ_get_grade_colour(score);
		}
		CAT_graph_set_background(CAT_TRANSPARENT);
		CAT_graph_set_foreground(CAT_WHITE);
		CAT_graph_set_window(x, y-24+4, x+72, y-4);
		CAT_graph_set_viewport(0, 0, 1, 1);
		CAT_graph_set_point_size(2);
		CAT_graph_set_point_fill(true);
		CAT_graph_draw(samples, colours, 7);
	}

	return y + CAT_GLYPH_HEIGHT*2 + 6;
}

int draw_notice(int x, int y, int aqm)
{
	float score = CAT_AQ_live_score_normalized(aqm);
	if(score >= 0.9)
	{
		CAT_set_sprite_colour(CAT_GREEN);
		CAT_set_sprite_flags(CAT_DRAW_FLAG_BOTTOM);
		CAT_draw_sprite(&ui_aq_notice_icons, 0, x, y);
	}
	else if(score <= 0.6)
	{
		CAT_set_sprite_colour(CAT_RED);
		CAT_set_sprite_flags(CAT_DRAW_FLAG_BOTTOM);
		CAT_draw_sprite(&ui_aq_notice_icons, 1, x, y);
	}

	return y + CAT_GLYPH_HEIGHT*2 + 6;
}

int draw_delta(int x, int y, int aqm)
{
	x = CAT_LCD_SCREEN_W - 36;

	float delta = CAT_AQ_live_score_delta(aqm);
	if(delta != 0)
	{
		int good_sign = CAT_AQ_get_good_delta_sign(aqm);
		int sign = sgn(delta);
		uint16_t c = sign == good_sign ? CAT_GRADE_COLOUR_GOOD : CAT_GRADE_COLOUR_BAD;
		CAT_draw_arrow(x, y-4, 12, 12, sign == 1 ? CAT_ORIENTATION_NORTH : CAT_ORIENTATION_SOUTH, c);
	}	
	return y + CAT_GLYPH_HEIGHT*2 + 6;
}

int draw_typical(int cursor_y, int aqm)
{
	int cursor_x = 12;
	cursor_x = draw_title(cursor_x, cursor_y, CAT_AQ_get_title_string(aqm));
	uint16_t c = CAT_AQ_get_grade_colour(CAT_AQ_live_score_normalized(aqm));
	cursor_x = draw_value(cursor_x, cursor_y, c, "%d", (int) CAT_AQ_live_score_raw(aqm));
	cursor_x = draw_unit(cursor_x, cursor_y, CAT_AQ_get_unit_string(aqm));
	cursor_y = draw_delta(cursor_x, cursor_y, aqm);
	return cursor_y;
}

void CAT_monitor_render_details()
{
	int cursor_y = TITLE_Y;
	cursor_y = center_textf(120, cursor_y, 1, CAT_WHITE, "Details");
	cursor_y = underline(120, cursor_y, 1, CAT_WHITE, "Details") + 44;

	cursor_y = draw_typical(cursor_y, CAT_AQM_CO2);

	float rebreathed = (((readings.sunrise.ppm_filtered_compensated)-420.)/38000.);
	int cursor_x = 12;
	uint16_t c = CAT_AQ_get_grade_colour(1.0f-rebreathed);
	cursor_x = draw_value(cursor_x, cursor_y, c, "%d%%", (int)(rebreathed * 100));
	cursor_x = draw_title(cursor_x, cursor_y, "rebreathed");
	cursor_y = draw_delta(cursor_x, cursor_y, CAT_AQM_CO2);

	cursor_y = draw_typical(cursor_y, CAT_AQM_PM2_5);
	cursor_y = draw_typical(cursor_y, CAT_AQM_NOX);
	cursor_y = draw_typical(cursor_y, CAT_AQM_VOC);
	cursor_y = draw_typical(cursor_y, CAT_AQM_TEMP);
	cursor_y = draw_typical(cursor_y, CAT_AQM_RH);
}