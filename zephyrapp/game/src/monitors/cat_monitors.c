#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include "sprite_assets.h"
#include "cat_monitor_graphics_utils.h"

enum
{
	SUMMARY,
	DETAILS,
	SPARKLINES,
	CALENDAR,
	LOGS,
	// GAMEPLAY,
	CLOCK,
	PAGE_COUNT
};
static int page = SUMMARY;

static struct
{
	CAT_machine_state state;
	CAT_render_callback render;
} routines[PAGE_COUNT] =
{
	[SUMMARY] =
	{
		.state = CAT_monitor_MS_summary,
		.render = CAT_monitor_render_summary
	},
	[DETAILS] =
	{
		.state = CAT_monitor_MS_summary,
		.render = CAT_monitor_render_details
	},
	[SPARKLINES] =
	{
		.state = CAT_monitor_MS_sparklines,
		.render = CAT_monitor_render_sparklines
	},
	[CALENDAR] =
	{
		.state = CAT_monitor_MS_calendar,
		.render = CAT_monitor_render_calendar
	},
	[LOGS] =
	{
		.state = CAT_monitor_MS_logs,
		.render = CAT_monitor_render_logs
	},
	/*[GAMEPLAY] =
	{
		.state = CAT_monitor_MS_gameplay,
		.render = CAT_monitor_render_gameplay
	},*/
	[CLOCK] =
	{
		.state = CAT_monitor_MS_clock,
		.render = CAT_monitor_render_clock
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

static void render_monitor()
{
	CAT_frameberry(RGB8882565(35, 157, 235));
	draw_page_markers(8, PAGE_COUNT, page);

	if
	(
		(page == SUMMARY ||
		page == DETAILS) &&
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
	page = (page+1) % PAGE_COUNT;
	CAT_machine_transition(routines[page].state);
}

void CAT_monitor_retreat()
{
	page = (page-1+PAGE_COUNT) % PAGE_COUNT;
	CAT_machine_transition(routines[page].state);
}

void CAT_monitor_exit()
{
	CAT_machine_transition(CAT_MS_room);
}

void CAT_MS_monitor(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(render_monitor);
			page = SUMMARY;
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			CAT_machine_transition(routines[page].state);
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}