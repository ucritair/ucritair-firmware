#include "cat_arcade.h"
#include <stddef.h>
#include "cat_render.h"
#include "cat_gizmos.h"
#include "cat_gui.h"
#include "cat_effects.h"
#include "cat_curves.h"
#include "cat_colours.h"
#include "cat_input.h"
#include "cat_concepts.h"

static void draw_tutorial();
static void MS_tutorial(CAT_FSM_signal);
static void draw_gameplay();
static void MS_gameplay(CAT_FSM_signal);
static void draw_performance();
static void MS_performance(CAT_FSM_signal);

static CAT_FSM fsm = {};

static enum
{
	PHASE_ARROWS,
	PHASE_ARROWS_OPPOSITE,
	PHASE_ARROWS_INCONGRUENT,
	PHASE_COUNT
};
static int phase = PHASE_ARROWS;
static int perfects[PHASE_COUNT];
static int misses[PHASE_COUNT];
static float times[PHASE_COUNT];

static int arrow_direction;
static int arrow_position;
static bool arrow_request;
static int arrow_count = 0;
static int arrow_pool[4];
static bool arrow_missed = false;

static CAT_timed_latch error_latch = CAT_TIMED_LATCH_INIT(1.0f);

static int key_map[] =
{
	CAT_BUTTON_RIGHT,
	CAT_BUTTON_UP,
	CAT_BUTTON_LEFT,
	CAT_BUTTON_DOWN
};

static int dxdy[] =
{
	1, 0,
	0, -1,
	-1, 0,
	0, 1
};

static void phase_transition(int _phase)
{
	if(_phase == PHASE_COUNT)
		CAT_pushdown_pop();
	phase = _phase;
	arrow_count = 0;
	perfects[phase] = 0;
	misses[phase] = 0;
	times[phase] = 0;
	CAT_FSM_transition(&fsm, MS_tutorial);
}

static void generate_arrow()
{
	int arrow_idx = arrow_count % 4;

	if(arrow_count % 4 == 0)
	{
		for(int i = 0; i < 4; i++)
			arrow_pool[i] = i;
		for(int i = 0; i < 4; i++)
		{
			int j = CAT_rand_int(0, 3);
			int temp = arrow_pool[i];
			arrow_pool[i] = arrow_pool[j];
			arrow_pool[j] = temp;
		}
	}

	arrow_direction = arrow_pool[arrow_idx];
	arrow_position = arrow_direction;
	if(phase == PHASE_ARROWS_INCONGRUENT && CAT_rand_chance(2))
	{
		arrow_position += 2;
		arrow_position %= 4;
	}

	arrow_count += 1;
	arrow_request = false;
	arrow_missed = false;
}

static void MS_gameplay(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(draw_gameplay);
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{			
			CAT_timed_latch_tick(&error_latch);
			if(CAT_timed_latch_get(&error_latch))
				return;

			if(arrow_request)
				generate_arrow();

			for(int i = 0; i < 4; i++)
			{
				if(CAT_input_pressed(key_map[i]))
				{
					if
					(
						(phase != PHASE_ARROWS_OPPOSITE && i == arrow_direction) ||
						(phase == PHASE_ARROWS_OPPOSITE && i == (arrow_direction+2)%4)
					)
					{
						if(!arrow_missed)
							perfects[phase] += 1;
						arrow_request = true;
						if(arrow_count == 16)
						{
							CAT_FSM_transition(&fsm, MS_performance);
						}
					}
					else
					{
						arrow_missed = true;
						misses[phase] += 1;
						CAT_timed_latch_raise(&error_latch);
					}
				}
			}

			times[phase] += CAT_get_delta_time_s();
 		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

#define CENTER_X 120
#define CENTER_Y 160
#define CENTER_R 24

#define ARROW_R 92
#define ARROW_W 24
#define ARROW_H 16

static void draw_arrow(int x, int y, int l, int w, int h, uint16_t c, int d)
{
	int dx = dxdy[d*2+0];
	int dy = dxdy[d*2+1];

	int x0 = x-dx*l/2;
	int y0 = y-dy*l/2;
	int x1 = x+dx*l/2;
	int y1 = y+dy*l/2;

	CAT_lineberry(x0, y0, x1, y1, c);
	CAT_draw_arrow(x1-dx*h, y1-dy*h, w, h, d, c);
}

static void draw_gameplay()
{
	CAT_frameberry(CAT_BLACK);

	switch (phase)
	{
		case PHASE_ARROWS_INCONGRUENT:
		{
			CAT_circberry(CENTER_X, CENTER_Y, CENTER_R, CAT_WHITE);
			int dx = dxdy[arrow_position*2+0];
			int dy = dxdy[arrow_position*2+1];
			int x = CENTER_X + dx * (ARROW_R*0.75f);
			int y = CENTER_Y + dy * (ARROW_R*0.75f);
			draw_arrow(x, y, ARROW_R/2, ARROW_W, ARROW_H, CAT_WHITE, arrow_direction);
		}
		break;

		default:
		{
			draw_arrow(CENTER_X, CENTER_Y, ARROW_R, ARROW_W, ARROW_H, CAT_WHITE, arrow_direction);
		}
		break;
	}

	if(CAT_timed_latch_get(&error_latch))
	{
		CAT_lineberry(0, CENTER_Y-64, CAT_LCD_SCREEN_W, CENTER_Y+64, CAT_RED);
		CAT_lineberry(0, CENTER_Y+64, CAT_LCD_SCREEN_W, CENTER_Y-64, CAT_RED);
	}
}

static void MS_tutorial(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(draw_tutorial);
		break;

		case CAT_FSM_SIGNAL_TICK:
			if(CAT_input_pressed(CAT_BUTTON_A))
				CAT_FSM_transition(&fsm, MS_gameplay);	
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

static void draw_tutorial()
{
	CAT_frameberry(CAT_BLACK);

	int cursor_y = 12;
	CAT_draw_page_markers(cursor_y, PHASE_COUNT, phase, CAT_WHITE);
	cursor_y += 24;

	CAT_set_text_mask(12, -1, CAT_LCD_SCREEN_W-12, -1);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	switch (phase)
	{
		case PHASE_ARROWS:
			cursor_y = CAT_draw_text(12, cursor_y, "Press the D-pad button corresponding to the arrow's direction.\n");
		break;
		case PHASE_ARROWS_OPPOSITE:
			cursor_y = CAT_draw_text(12, cursor_y, "Press the D-pad button corresponding to the opposite of the arrow's direction.\n");
		break;
		case PHASE_ARROWS_INCONGRUENT:
			cursor_y = CAT_draw_text(12, cursor_y, "Press the D-pad button corresponding to the arrow's direction, NOT its position.\n");
		break;
	}
}

static void MS_performance(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(draw_performance);
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_A))
				phase_transition(phase+1);
		}			
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

static void draw_performance()
{
	CAT_frameberry(CAT_BLACK);

	int cursor_y = 12;
	CAT_draw_page_markers(cursor_y, PHASE_COUNT, phase, CAT_WHITE);
	cursor_y += 24;

	CAT_set_text_mask(12, -1, CAT_LCD_SCREEN_W-12, -1);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	cursor_y = CAT_draw_textf(12, cursor_y, "Perfect: %d\nMissed: %d\nTime: "CAT_FLOAT_FMT"\n", perfects[phase], misses[phase], CAT_FMT_FLOAT(times[phase]));
}

void CAT_MS_stroop(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			phase_transition(PHASE_ARROWS);
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			CAT_FSM_tick(&fsm);
			if(fsm.state == NULL)
				CAT_pushdown_pop();
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_stroop()
{
	
}