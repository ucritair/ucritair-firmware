#include "menu_time.h"
#include "cat_room.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include <stdio.h>
#include "cat_deco.h"
#include "cat_actions.h"
#include "cat_bag.h"
#include "rtc.h"
#include "airquality.h"
#include "flash.h"
#include "cat_menu.h"

#include "menu_graph.h"


#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(menu_aqi, LOG_LEVEL_DBG);

#define AQI_VIEW_CELL_LATEST -1

int aqi_view_cell = AQI_VIEW_CELL_LATEST;
int last_fetched_aqi_view_cell = AQI_VIEW_CELL_LATEST;
struct flash_log_cell view_cell;

bool view_pn = false;

void CAT_MS_aqi(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();

			if(CAT_input_pressed(CAT_BUTTON_A))
				CAT_machine_transition(CAT_MS_graph);

			if (CAT_input_pressed(CAT_BUTTON_SELECT))
				view_pn = !view_pn;

			if (CAT_input_pulse(CAT_BUTTON_LEFT))
			{
				if (aqi_view_cell == AQI_VIEW_CELL_LATEST)
				{
					if (next_log_cell_nr > 0)
						aqi_view_cell = next_log_cell_nr - 1;
				}
				else if (aqi_view_cell != 0)
					aqi_view_cell--;
			}

			if (CAT_input_pulse(CAT_BUTTON_RIGHT))
			{
				if (aqi_view_cell == AQI_VIEW_CELL_LATEST)
					return;
				else if (aqi_view_cell == next_log_cell_nr-1)
					aqi_view_cell = AQI_VIEW_CELL_LATEST;
				else
					aqi_view_cell++;
			}

			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

uint64_t latch_most_recent = 0;

void CAT_render_aqi()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("AIR QUALITY ");
	CAT_gui_image(icon_a_sprite, 1);
	CAT_gui_image(icon_plot_sprite, 0);
	CAT_gui_image(icon_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);

	CAT_gui_line_break();

	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});

	bool viewing_latest = aqi_view_cell == AQI_VIEW_CELL_LATEST;

	if (viewing_latest)
	{
		populate_log_cell(&view_cell);
	}
	else if (aqi_view_cell != last_fetched_aqi_view_cell)
	{
		flash_get_cell_by_nr(aqi_view_cell, &view_cell);
		last_fetched_aqi_view_cell = aqi_view_cell;
	}

	if ((viewing_latest && next_log_cell_nr != 0) || (!viewing_latest && aqi_view_cell != 0))
	{
		CAT_gui_text("<  ");
	}
	else
	{
		CAT_gui_text("   ");
	}

	if (viewing_latest)
	{
		CAT_gui_text("  Live Air Quality\n");
	}
	else
	{
		CAT_gui_text("Logged Air Quality  >\n");
	}

	struct tm t;
	time_t now = view_cell.timestamp;

	// Annoying interaction between low-tick-rate RTC and sensor sample delay
	// makes the observed time move backwards sometimes
	if (viewing_latest && (latch_most_recent > now))
	{
		now = latch_most_recent;
	}
	latch_most_recent = now;

	gmtime_r(&now, &t);

	// LOG_DBG("at gmtime_r now=%lld; year=%d", now, t.tm_year);

	CAT_gui_textf("\2 %s ", month_names[t.tm_mon]);
	CAT_gui_textf("%2d ", t.tm_mday);
	CAT_gui_textf("%4d, ", t.tm_year);
	CAT_gui_textf("%2d:", t.tm_hour);
	CAT_gui_textf("%2d:", t.tm_min);
	CAT_gui_textf("%2d", t.tm_sec);
	CAT_gui_line_break();
	CAT_gui_line_break();
	
	if (view_cell.flags & FLAG_HAS_CO2)
	{
		CAT_gui_textf("CO2: %dppm\n", (int)view_cell.co2_ppmx1);
		CAT_gui_textf("    (%.1f%% rebreathed air)\n", ((((double)view_cell.co2_ppmx1)-420.)/38000.)*100.);
	}
	else
	{
		CAT_gui_textf(viewing_latest?"CO2 sensor starting...\n":"CO2 not recorded\n");
		CAT_gui_line_break();
	}
	CAT_gui_line_break();
	
	if (view_cell.flags & FLAG_HAS_TEMP_RH_PARTICLES)
	{
		if (view_pn)
		{
			CAT_gui_textf("PN0.5: % 2.01f \4/cm\x7f\n", ((double)view_cell.pn_ugmx100[0])/100.);
			CAT_gui_textf("PN1.0: % 2.01f \4/cm\x7f\n", ((double)view_cell.pn_ugmx100[1])/100.);
			CAT_gui_textf("PN2.5: % 2.01f \4/cm\x7f\n", ((double)view_cell.pn_ugmx100[2])/100.);
			CAT_gui_textf("PN4.0: % 2.01f \4/cm\x7f\n", ((double)view_cell.pn_ugmx100[3])/100.);
			CAT_gui_textf("PN10 : % 2.01f \4/cm\x7f\n", ((double)view_cell.pn_ugmx100[4])/100.);
		}
		else
		{
			CAT_gui_textf("PM1.0: % 2.01f ~g/m\x7f\n", ((double)view_cell.pm_ugmx100[0])/100.);
			CAT_gui_textf("PM2.5: % 2.01f ~g/m\x7f\n", ((double)view_cell.pm_ugmx100[1])/100.);
			CAT_gui_textf("PM4.0: % 2.01f ~g/m\x7f\n", ((double)view_cell.pm_ugmx100[2])/100.);
			CAT_gui_textf("PM10 : % 2.01f ~g/m\x7f\n", ((double)view_cell.pm_ugmx100[3])/100.);
			CAT_gui_line_break();
		}

		CAT_gui_image(icon_select_sprite, 1);
		CAT_gui_textf("to view %s\n", view_pn?"PM":"PN");
	}
	else
	{
		CAT_gui_textf(viewing_latest?"PM sensor starting...\n":"PM not recorded\n");
		CAT_gui_line_break();
		CAT_gui_line_break();
		CAT_gui_line_break();
	}
	CAT_gui_line_break();
	
	if (view_cell.flags & FLAG_HAS_TEMP_RH_PARTICLES)
	{
		if (view_cell.pressure_hPax10 != 0)
		{
			CAT_gui_textf("%2.1f\3C    %2.0f%%RH    %.0fhPa\n", 
				((double)view_cell.temp_Cx1000)/1000.,
				((double)view_cell.rh_pctx100)/100.,
				((double)view_cell.pressure_hPax10)/10.);
		}
		else
		{
			CAT_gui_textf("%2.1f\3C    %2.0f%%RH\n", 
				((double)view_cell.temp_Cx1000)/1000.,
				((double)view_cell.rh_pctx100)/100.);
		}
	}
	else
	{
		CAT_gui_textf(viewing_latest?"Temp/RH sensor starting...\n":"Temp/RH not recorded\n");
	}
	CAT_gui_line_break();
	
	if (view_cell.nox_index && view_cell.voc_index)
	{
		CAT_gui_textf("VOC %3.0f    NOX %3.0f\n", (double)view_cell.voc_index, (double)view_cell.nox_index);
	}
	else
	{
		CAT_gui_textf(viewing_latest?"VOC/NOX sensor starting...\n":"VOC/NOX not recorded\n");
		CAT_gui_line_break();
	}
}
