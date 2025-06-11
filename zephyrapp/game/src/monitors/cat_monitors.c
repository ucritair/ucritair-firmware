#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include "sprite_assets.h"

void CAT_monitor_MS_air(CAT_machine_signal signal);
void CAT_monitor_render_summary();
void CAT_monitor_render_details();
void CAT_monitor_render_sparklines();
#include "cat_monitor_air.c"


void CAT_monitor_MS_clock(CAT_machine_signal signal);
void CAT_monitor_render_clock();
#include "cat_monitor_clock.c"

enum
{
	SUMMARY,
	DETAILS,
	/*SPARKLINES,*/
	CLOCK,
	PAGE_MAX
};
static int page = SUMMARY;

static struct
{
	CAT_machine_state state;
	CAT_render_callback render;
} routines[PAGE_MAX] =
{
	[SUMMARY] =
	{
		.state = CAT_monitor_MS_air,
		.render = CAT_monitor_render_summary
	},
	[DETAILS] =
	{
		.state = CAT_monitor_MS_air,
		.render = CAT_monitor_render_details
	},
	/*[SPARKLINES] =
	{
		.state = CAT_monitor_MS_air,
		.render = CAT_monitor_render_sparklines
	},*/
	[CLOCK] =
	{
		.state = CAT_monitor_MS_clock,
		.render = CAT_monitor_render_clock
	}
};

static void render_statics()
{
	CAT_frameberry(RGB8882565(35, 157, 235));
	CAT_set_draw_flags(CAT_DRAW_FLAG_BOTTOM);
	CAT_draw_sprite(&monitor_clouds_sprite, 0, 0, 320);
}

static void render_page_markers()
{
	int start_x = 120 - ((16 + 2) * PAGE_MAX) / 2;
	for(int i = 0; i < PAGE_MAX; i++)
	{
		int x = start_x + i * (16 + 2);
		CAT_draw_sprite(&ui_radio_button_diamond_sprite, page == i, x, 8);
	}
}

static void render_exit_notice()
{
	const char* text = "[START] to exit";
	int x = CAT_LCD_SCREEN_W - 6 - strlen(text) * CAT_GLYPH_WIDTH;
	int y = CAT_LCD_SCREEN_H - 4 - CAT_GLYPH_HEIGHT;
	CAT_set_text_colour(0x2455);
	CAT_draw_text(x, y, text);
}

static void render_monitor()
{
	render_statics();
	render_page_markers();
	routines[page].render();
	// render_exit_notice();
}

void CAT_monitor_advance()
{
	page = (page+1) % PAGE_MAX;
	CAT_machine_transition(routines[page].state);
}

void CAT_monitor_retreat()
{
	page = (page-1+PAGE_MAX) % PAGE_MAX;
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