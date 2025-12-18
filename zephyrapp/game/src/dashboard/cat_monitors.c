#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include "sprite_assets.h"
#include "cat_monitor_graphics_utils.h"

static struct
{
	CAT_FSM_state state;
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
	[CAT_MONITOR_PAGE_ACH] =
	{
		.state = CAT_monitor_MS_ACH,
		.render = CAT_monitor_render_ACH
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
static int page = CAT_MONITOR_PAGE_SUMMARY;

static CAT_FSM fsm =
{
	.state = NULL,
	.next = NULL,
	.dirty = false
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

#define PAGE_MARKER_Y 8

static void render_monitor()
{	
	uint16_t bg = CAT_BLACK;
	uint16_t fg = CAT_WHITE;
	switch (page)
	{
		case CAT_MONITOR_PAGE_CLOCK:
			bg = CAT_SKY_BLUE;
		break;
		case CAT_MONITOR_PAGE_GAMEPLAY:
			fg = CAT_CRISIS_YELLOW;
		break;
		default:
		break;
	}
	CAT_frameberry(bg);
	CAT_draw_page_markers(PAGE_MARKER_Y, CAT_MONITOR_PAGE_COUNT, CAT_monitor_tell(), fg);
	if(CAT_AQ_is_crisis_ongoing())
		CAT_draw_page_alert(PAGE_MARKER_Y, CAT_MONITOR_PAGE_COUNT, CAT_MONITOR_PAGE_GAMEPLAY, CAT_RED);

	if
	(
		(page == CAT_MONITOR_PAGE_SUMMARY ||
		page == CAT_MONITOR_PAGE_DETAILS) &&
		!CAT_AQ_sensors_initialized() && !CAT_logs_initialized()
	)
	{
		draw_uninit_warning();
	}
	else
	{
		routines[page].render();
	}
}

void CAT_monitor_advance()
{
	page = (page+1) % CAT_MONITOR_PAGE_COUNT;
	CAT_FSM_transition(&fsm, routines[page].state);
}

void CAT_monitor_retreat()
{
	page = (page-1+CAT_MONITOR_PAGE_COUNT) % CAT_MONITOR_PAGE_COUNT;
	CAT_FSM_transition(&fsm, routines[page].state);
}

void CAT_monitor_seek(int target)
{
	page = CAT_clamp(target, CAT_MONITOR_PAGE_SUMMARY, CAT_MONITOR_PAGE_COUNT-1);
	CAT_FSM_transition(&fsm, routines[page].state);
}

int CAT_monitor_tell()
{
	return page;
}

void CAT_monitor_exit()
{
	if(CAT_AQ_is_crisis_report_posted())
		CAT_pushdown_rebase(CAT_MS_crisis_report);
	else
		CAT_pushdown_rebase(CAT_MS_room);
}

void CAT_monitor_dismiss()
{
	if(CAT_AQ_is_crisis_ongoing())
		CAT_monitor_seek(CAT_MONITOR_PAGE_GAMEPLAY);
	else
		CAT_pushdown_rebase(CAT_MS_room);
}

void CAT_MS_monitor(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(render_monitor);
			CAT_monitor_seek(CAT_MONITOR_PAGE_SUMMARY);
		break;

		case CAT_FSM_SIGNAL_TICK:
			CAT_FSM_tick(&fsm);
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}