#include "cat_monitors.h"

#include "cat_core.h"
#include "cat_render.h"
#include "cat_gui.h"
#include <stdarg.h>
#include "cat_aqi.h"
#include "cat_monitor_graphics_utils.h"

#define TITLE_Y 44

void CAT_monitor_render_details()
{
	int cursor_y = TITLE_Y;
	cursor_y = center_textf(120, cursor_y, 1, CAT_WHITE, "Details");
	cursor_y = underline(120, cursor_y, 1, CAT_WHITE, "Details") + 44;

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