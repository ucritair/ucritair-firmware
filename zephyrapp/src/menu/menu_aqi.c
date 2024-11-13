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

#include "menu_graph.h"


#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(menu_aqi, LOG_LEVEL_DBG);

#define AQI_VIEW_CELL_LATEST -1

int aqi_view_cell = AQI_VIEW_CELL_LATEST;
int last_fetched_aqi_view_cell = AQI_VIEW_CELL_LATEST;
struct flash_log_cell view_cell;

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
				CAT_machine_transition(CAT_MS_menu);

			if(CAT_input_pressed(CAT_BUTTON_A))
				CAT_machine_transition(CAT_MS_graph);

			if (CAT_input_pulse(CAT_BUTTON_LEFT))
			{
				if (aqi_view_cell == AQI_VIEW_CELL_LATEST)
					aqi_view_cell = next_log_cell_nr - 1;
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
	
	char buf[64];
#define textf(...) snprintf(buf, sizeof(buf), __VA_ARGS__); CAT_gui_text(buf);
#define textfnl(...) textf(__VA_ARGS__); CAT_gui_line_break();

	if ((viewing_latest && next_log_cell_nr != 0) || (!viewing_latest && aqi_view_cell != 0))
	{
		textf("<- ");
	}
	else
	{
		textf("   ");
	}

	if (viewing_latest)
	{
		textfnl("  Live Air Quality");
	}
	else
	{
		textfnl("Logged Air Quality ->");
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

	textf("@ %s ", month_names[t.tm_mon]);
	textf("%2d ", t.tm_mday);
	textf("%4d, ", t.tm_year);
	textf("%2d:", t.tm_hour);
	textf("%2d:", t.tm_min);
	textf("%2d", t.tm_sec);

	CAT_gui_line_break();
	
	textfnl(" ");
	
	if (view_cell.flags & FLAG_HAS_CO2)
	{
		textfnl("CO2: %dppm", (int)view_cell.co2_ppmx1);
		textfnl("    (%.1f%% rebreathed air)", ((((double)view_cell.co2_ppmx1)-420.)/38000.)*100.);
	}
	else
	{
		textfnl(viewing_latest?"CO2 sensor starting...":"CO2 not recorded");
		textfnl("");
	}

	textfnl(" ");
	
	if (view_cell.flags & FLAG_HAS_TEMP_RH_PARTICLES)
	{
		textfnl("PM0.5:            % 2.1f #/m\x7f", ((double)view_cell.pn_ugmx100[0])/100.);
		textfnl("PM1.0: % 2.1f ~g/m\x7f % 2.1f #/m\x7f", ((double)view_cell.pm_ugmx100[0])/100., ((double)view_cell.pn_ugmx100[1])/100.);
		textfnl("PM2.5: % 2.1f ~g/m\x7f % 2.1f #/m\x7f", ((double)view_cell.pm_ugmx100[1])/100., ((double)view_cell.pn_ugmx100[2])/100.);
		textfnl("PM4.0: % 2.1f ~g/m\x7f % 2.1f #/m\x7f", ((double)view_cell.pm_ugmx100[2])/100., ((double)view_cell.pn_ugmx100[3])/100.);
		textfnl("PM10 : % 2.1f ~g/m\x7f % 2.1f #/m\x7f", ((double)view_cell.pm_ugmx100[3])/100., ((double)view_cell.pn_ugmx100[4])/100.);
	}
	else
	{
		textfnl(viewing_latest?"PM sensor starting...":"PM not recorded");
		textfnl(" ");
		textfnl(" ");
		textfnl(" ");
		textfnl(" ");
	}

	textfnl(" ");
	
	if (view_cell.flags & FLAG_HAS_TEMP_RH_PARTICLES)
	{
		if (view_cell.pressure_hPax10 != 0)
		{
			textfnl("%2.1f`C    %2.0f%%RH    %.0fhPa", 
				((double)view_cell.temp_Cx1000)/1000.,
				((double)view_cell.rh_pctx100)/100.,
				((double)view_cell.pressure_hPax10)/10.);
		}
		else
		{
			textfnl("%2.1f`C    %2.0f%%RH", 
				((double)view_cell.temp_Cx1000)/1000.,
				((double)view_cell.rh_pctx100)/100.);
		}
	}
	else
	{
		textfnl(viewing_latest?"Temp/RH sensor starting...":"Temp/RH not recorded");
	}

	textfnl(" ");
	
	if (view_cell.nox_index && view_cell.voc_index)
	{
		textfnl("VOC %3.0f    NOX %3.0f", (double)view_cell.voc_index, (double)view_cell.nox_index);
	}
	else
	{
		textfnl(viewing_latest?"VOC/NOX sensor starting...":"VOC/NOX not recorded");
		textfnl(" ");
	}
}
