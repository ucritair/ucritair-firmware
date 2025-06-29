#include "cat_monitors.h"

#include "cat_core.h"
#include "cat_render.h"
#include "cat_gui.h"
#include <stdarg.h>
#include "cat_aqi.h"
#include "cat_monitor_graphics_utils.h"
#include "cat_input.h"

#define TITLE_Y 40
#define MARGIN_X 12

static bool focused;

static int idx;
static CAT_log_cell cell;
static CAT_datetime datetime;

void CAT_monitor_render_logs()
{
	if(!focused)
	{
		int cursor_y = center_textf(120, 60, 2, CAT_WHITE, "Logs");
		cursor_y = underline(120, cursor_y, 2, CAT_WHITE, "Logs");

		CAT_fillberry(120 - 60, 160 - 20, 120, 40, RGB8882565(35, 157, 235));
		center_textf(120, 160, CAT_input_held(CAT_BUTTON_A, 0) ? 3 : 2 ,CAT_WHITE, "Press A");
		return;
	}

	int cursor_y = TITLE_Y;
	cursor_y = center_textf(120, cursor_y, 1, CAT_WHITE, "Log %d", idx);
	cursor_y = underline(120, cursor_y, 1, CAT_WHITE, "Log %d", idx);
	cursor_y += 8;
	cursor_y = center_textf(120, cursor_y, 1, CAT_WHITE, "%.2d/%.2d/%.4d %d:%.2d", datetime.month, datetime.day, datetime.year, datetime.hour, datetime.second);
	cursor_y += 12;

	if(cell.flags & CAT_LOG_CELL_FLAG_HAS_CO2)
	{
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, "CO2       %d ppm\n", cell.co2_ppmx1);
		cursor_y += 6;
	}

	if(cell.voc_index && cell.nox_index)
	{
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, "VOC       %d\n", cell.voc_index);
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, "NOX       %d\n", cell.nox_index);
		cursor_y += 6;
	}

	if(cell.flags & CAT_LOG_CELL_FLAG_HAS_TEMP_RH_PARTICLES)
	{
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, "PM 1.0    %.1f \4g/m\5\n", cell.pm_ugmx100[0]/100.0f);
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, "PM 2.5    %.1f \4g/m\5\n", cell.pm_ugmx100[1]/100.0f);
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, "PM 4.0    %.1f \4g/m\5\n", cell.pm_ugmx100[2]/100.0f);
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, "PM10.0    %.1f \4g/m\5\n", cell.pm_ugmx100[3]/100.0f);
		cursor_y += 6;

		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, "PN 0.5    %.1f #/cm\5\n", cell.pn_ugmx100[0]/100.0f);
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, "PN 1.0    %.1f #/cm\5\n", cell.pn_ugmx100[1]/100.0f);
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, "PN 2.5    %.1f #/cm\5\n", cell.pn_ugmx100[2]/100.0f);
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, "PN 4.0    %.1f #/cm\5\n", cell.pn_ugmx100[3]/100.0f);
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf(MARGIN_X, cursor_y, "PN10.0    %.1f #/cm\5\n", cell.pn_ugmx100[4]/100.0f);
		cursor_y += 6;

		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf
		(
			MARGIN_X, cursor_y, "%.1f%s  %.1f %%RH  %.1f hPa",
			CAT_AQ_map_celsius(cell.temp_Cx1000 / 1000.0f), CAT_AQ_get_temperature_unit_string(),
			cell.rh_pctx100 / 100.0f, cell.pressure_hPax10 / 10.0f
		);
	}
}

void CAT_monitor_MS_logs(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			focused = false;
			idx = CAT_get_log_cell_count()-1;
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(!focused)
			{
				if(CAT_input_dismissal())
					CAT_monitor_soft_exit();
				if(CAT_input_pressed(CAT_BUTTON_LEFT))
					CAT_monitor_retreat();
				if(CAT_input_pressed(CAT_BUTTON_RIGHT))
					CAT_monitor_advance();
				
				if(CAT_input_released(CAT_BUTTON_A))
					focused = true;
			}
			else
			{
				if(CAT_input_pressed(CAT_BUTTON_B))
					focused = false;
				if(CAT_input_pulse(CAT_BUTTON_LEFT))
					idx -= 1;
				if(CAT_input_pulse(CAT_BUTTON_RIGHT))
					idx += 1;
				idx = (idx + CAT_get_log_cell_count()) % CAT_get_log_cell_count();

				CAT_read_log_cell_at_idx(idx, &cell);
				CAT_make_datetime(cell.timestamp, &datetime);
			}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}