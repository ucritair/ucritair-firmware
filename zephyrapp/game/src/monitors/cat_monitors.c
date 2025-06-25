#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include "sprite_assets.h"
#include "cat_monitor_graphics_utils.h"

static int page = CAT_MONITOR_PAGE_SUMMARY;

static struct
{
	CAT_machine_state state;
	CAT_render_callback render;
} routines[CAT_MONITOR_PAGE_COUNT] =
{
	[CAT_MONITOR_PAGE_SUMMARY] =
	{
		.state = CAT_monitor_MS_summary,
		.render = CAT_monitor_render_summary
	},
	[CAT_MONITOR_PAGE_DETAILS] =
	{
		.state = CAT_monitor_MS_summary,
		.render = CAT_monitor_render_details
	},
	[CAT_MONITOR_PAGE_SPARKLINES] =
	{
		.state = CAT_monitor_MS_sparklines,
		.render = CAT_monitor_render_sparklines
	},
	[CAT_MONITOR_PAGE_CALENDAR] =
	{
		.state = CAT_monitor_MS_calendar,
		.render = CAT_monitor_render_calendar
	},
	[CAT_MONITOR_PAGE_LOGS] =
	{
		.state = CAT_monitor_MS_logs,
		.render = CAT_monitor_render_logs
	},
	[CAT_MONITOR_PAGE_CLOCK] =
	{
		.state = CAT_monitor_MS_clock,
		.render = CAT_monitor_render_clock
	},
	[CAT_MONITOR_PAGE_GAMEPLAY] =
	{
		.state = CAT_monitor_MS_gameplay,
		.render = CAT_monitor_render_gameplay
	},
};

static void draw_uninit_warning()
{
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_scale(2);
	CAT_draw_text(12, 30, "Please wait...");
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_mask(12, -1, CAT_LCD_SCREEN_W-12, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_draw_text(12, 64, "Air quality sensors are coming online.");
}

static uint16_t bg_colour = CAT_MONITOR_BG_BLUE;
static uint16_t fg_colour = CAT_WHITE;

static void render_monitor()
{
	if(page != CAT_MONITOR_PAGE_GAMEPLAY)
		bg_colour = CAT_MONITOR_BG_BLUE;
	if(page != CAT_MONITOR_PAGE_GAMEPLAY)
		fg_colour = CAT_WHITE;
		
	CAT_frameberry(bg_colour);
	draw_page_markers(8, CAT_MONITOR_PAGE_COUNT, page);

	if
	(
		(page == CAT_MONITOR_PAGE_SUMMARY ||
		page == CAT_MONITOR_PAGE_DETAILS) &&
		!CAT_is_AQ_initialized()
	
	)
	{
		draw_uninit_warning();
	}
	else
		routines[page].render();
}

void CAT_monitor_advance()
{
	page = (page+1) % CAT_MONITOR_PAGE_COUNT;
	CAT_machine_transition(routines[page].state);
}

void CAT_monitor_retreat()
{
	page = (page-1+CAT_MONITOR_PAGE_COUNT) % CAT_MONITOR_PAGE_COUNT;
	CAT_machine_transition(routines[page].state);
}

void CAT_monitor_seek(int target)
{
	page = clamp(target, CAT_MONITOR_PAGE_SUMMARY, CAT_MONITOR_PAGE_COUNT-1);
	CAT_machine_transition(routines[page].state);
}

int CAT_monitor_tell()
{
	return page;
}

void CAT_monitor_exit()
{
	if(CAT_AQ_is_crisis_report_posted())
		CAT_machine_transition(CAT_MS_crisis_report);
	else
		CAT_machine_transition(CAT_MS_room);
}

void CAT_monitor_colour_bg(uint16_t colour)
{
	bg_colour = colour;
}

void CAT_monitor_colour_fg(uint16_t colour)
{
	fg_colour = colour;
}

uint16_t CAT_monitor_bg_colour()
{
	return bg_colour;
}

uint16_t CAT_monitor_fg_colour()
{
	return fg_colour;
}

void CAT_MS_monitor(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(render_monitor);
			page = CAT_MONITOR_PAGE_SUMMARY;
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			CAT_machine_transition(routines[page].state);
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}