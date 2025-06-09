#include "cat_monitors.h"

#include "cat_core.h"
#include "cat_render.h"
#include "cat_gui.h"
#include <stdarg.h>
#include "cat_aqi.h"

#define TEXT_Y 44

static char textf_buf[32];

static void score_bar(int x, int y, int aqm)
{
	float subscore = 1-CAT_AQ_normalized_scores[aqm];
	int total_width = 16*4;
	int filled_width = total_width * subscore;
	uint16_t colour = colour_score(1-subscore);

	CAT_discberry(x, y, 4, colour);
	CAT_lineberry(x+4, y, x+4+filled_width, y, colour);
	CAT_lineberry(x+4+filled_width, y, x+4+total_width, y, CAT_WHITE);
	CAT_discberry(x+4+total_width+4, y, 4, colour);
}

static int labeled_scoref(int x, int y, uint16_t c, int aqm, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(textf_buf, 32, fmt, args);
	va_end(args);

	CAT_set_text_colour(c);
	CAT_draw_text(x, y-CAT_GLYPH_HEIGHT, CAT_AQM_titles[aqm]);
	x += strlen(CAT_AQM_titles[aqm]) * CAT_GLYPH_WIDTH + 8;

	CAT_set_text_scale(2);
	CAT_set_text_colour(c);
	CAT_draw_text(x, y-CAT_GLYPH_HEIGHT*2, textf_buf);
	x += strlen(textf_buf) * CAT_GLYPH_WIDTH*2 + 8;

	const char* unit = aqm == CAT_AQM_TEMP ? CAT_AQ_get_temperature_unit_string() : CAT_AQM_units[aqm];
	CAT_set_text_colour(c);
	CAT_draw_text(x, y-CAT_GLYPH_HEIGHT, unit);
	x += strlen(unit) == 0 ? 4 : strlen(unit) * CAT_GLYPH_WIDTH + 12;

	int y_off = strlen(unit) == 0 ? -CAT_GLYPH_HEIGHT : -CAT_GLYPH_HEIGHT/2;
	score_bar(x, y+y_off, aqm);

	return y + CAT_GLYPH_HEIGHT*2 + 6;
}

void CAT_monitor_render_details()
{
	int cursor_y = TEXT_Y;
	cursor_y = center_textf(120, cursor_y, 1, CAT_WHITE, "Live Readings");
	cursor_y = underline(120, cursor_y, 1, CAT_WHITE, "Live Readings") + 44;

	cursor_y = labeled_scoref(12, cursor_y, CAT_WHITE, CAT_AQM_CO2, "%.0f", readings.sunrise.ppm_filtered_compensated);
	cursor_y = labeled_scoref(12, cursor_y, CAT_WHITE, CAT_AQM_PM2_5, "%.0f", readings.sen5x.pm2_5);
	cursor_y = labeled_scoref(12, cursor_y, CAT_WHITE, CAT_AQM_NOX, "%.0f", readings.sen5x.nox_index);
	cursor_y = labeled_scoref(12, cursor_y, CAT_WHITE, CAT_AQM_VOC, "%.0f", readings.sen5x.voc_index);
	cursor_y = labeled_scoref(12, cursor_y, CAT_WHITE, CAT_AQM_TEMP, "%.0f", CAT_AQ_map_celsius(readings.sen5x.temp_degC));
	cursor_y = labeled_scoref(12, cursor_y, CAT_WHITE, CAT_AQM_RH, "%.0f", readings.sen5x.humidity_rhpct);

	float pct_rebreathed = ((((double) readings.sunrise.ppm_filtered_compensated)-420.)/38000.)*100.;
	CAT_set_text_colour(CAT_WHITE);
	CAT_draw_textf(12, cursor_y, "%.1f%% rebreathed air", pct_rebreathed);
}