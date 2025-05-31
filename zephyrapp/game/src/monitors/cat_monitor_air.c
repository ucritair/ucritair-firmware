#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_text.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include "cat_aqi.h"

static void render()
{
	CAT_frameberry(RGB8882565(35, 157, 235));

	int cursor_y = 12;
	CAT_push_text_scale(2);
	CAT_push_text_colour(CAT_WHITE);
	CAT_draw_text(12, cursor_y, "Air Quality");
	cursor_y += 48;

	if(!CAT_is_AQ_initialized())
	{
		CAT_push_text_scale(2);
		CAT_push_text_flags(CAT_TEXT_FLAG_WRAP);
		CAT_push_text_line_width(CAT_LCD_SCREEN_W-24);
		CAT_push_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_text(12, cursor_y, "Air quality sensors are coming online. Please wait...");
	}
	else
	{
		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "CO2  %.0f ppm", readings.sunrise.ppm_filtered_compensated);
		cursor_y += 14;

		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "PM2.5  %.0f \4g/m\5", readings.sen5x.pm2_5);
		cursor_y += 14;

		if (readings.sen5x.nox_index && readings.sen5x.voc_index)
		{
			CAT_push_text_colour(CAT_WHITE);
			CAT_draw_textf(12, cursor_y, "NOX %.0f / VOC %.0f", readings.sen5x.nox_index, readings.sen5x.voc_index);
			cursor_y += 14;
		}

		float deg_c = readings.sen5x.temp_degC;
		float deg_mapped = CAT_AQ_map_celsius(deg_c);
		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "%.0f %s / %.0f%% RH", deg_mapped, CAT_AQ_get_temperature_unit_string(), readings.sen5x.humidity_rhpct);
		cursor_y += 28;

		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "%.1f%% rebreathed", ((((double) readings.sunrise.ppm_filtered_compensated)-420.)/38000.)*100.);
		cursor_y += 14;

		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "uCritAQI %.1f%%", CAT_AQI_aggregate());
		cursor_y += 14;
		
		CAT_datetime t;
		CAT_get_datetime(&t);
		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_textf(12, cursor_y, "at %2d:%02d:%02d", t.hour, t.minute, t.second);
		cursor_y += 14;
	}

	CAT_discberry(-32, 320, 116, RGB8882565(220, 220, 220));
	CAT_discberry(92, 320, 48, CAT_WHITE);
	CAT_discberry(200, 320, 92, CAT_WHITE);

	CAT_push_text_colour(RGB8882565(35, 157, 235));
	CAT_draw_text(64, 304, "[START] to enter game");
}

void CAT_MS_monitor_air(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(render);
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}