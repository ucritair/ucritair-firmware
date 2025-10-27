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
#include "cat_poly.h"
#include "cat_gizmos.h"

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
static float times[PHASE_COUNT];

static int arrow_direction;
static int arrow_position;
static bool arrow_request;
static int arrow_count = 0;
static int arrow_pool[4];
static bool arrow_missed = false;

static CAT_timed_latch error_latch = CAT_TIMED_LATCH_INIT(1.0f);
static CAT_timed_latch finish_latch = CAT_TIMED_LATCH_INIT(0.25f);
static CAT_timed_latch distraction_latch = CAT_TIMED_LATCH_INIT(2.0f);

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

static float angle_map[] =
{
	0.0f,
	M_PI / 2.0f,
	M_PI,
	3 * M_PI / 2.0f
};

static float x_mesh[] =
{
	-1.0f, -1.0f,
	1.0f, 1.0f,
	1.0f, -1.0f,
	-1.0f, 1.0f
};

static float arrow_mesh[] =
{
	-1.0f, 0.0f,
	1.0f, 0.0f,
	0.8f, 0.25f,
	1.0f, 0.0f,
	0.8f, -0.25f,
	1.0f, 0.0f
};

#define ARROWS_PER_PHASE 8

static void phase_transition(int _phase)
{
	if(_phase == PHASE_COUNT)
		CAT_pushdown_pop();
	phase = _phase;
	arrow_count = 0;
	perfects[phase] = 0;
	times[phase] = 0;
	CAT_FSM_transition(&fsm, MS_tutorial);
}

static void generate_arrow()
{
	int arrow_idx = arrow_count % 4;

	if(arrow_idx == 0)
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
			times[phase] += CAT_get_delta_time_s();
			CAT_timed_latch_tick(&error_latch);
			CAT_timed_latch_tick(&finish_latch);
			CAT_timed_latch_tick(&distraction_latch);

			if(CAT_timed_latch_get(&error_latch))
				return;
			if(CAT_timed_latch_get(&finish_latch))
				return;

			if(!CAT_timed_latch_get(&distraction_latch))
			{
				if(CAT_rand_chance(2))
					CAT_timed_latch_raise(&distraction_latch);
			}

			if(arrow_request)
			{
				generate_arrow();
				if(arrow_count == ARROWS_PER_PHASE)
				{
					if(phase == PHASE_COUNT-1)
						CAT_FSM_transition(&fsm, MS_performance);
					else
						phase_transition(phase+1);
				}					
			}

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
						CAT_timed_latch_raise(&finish_latch);
					}
					else
					{
						arrow_missed = true;
						CAT_timed_latch_raise(&error_latch);
					}
				}
			}
 		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

#define CENTER_X 120
#define CENTER_Y 160
#define CENTER_R 24
#define ARROW_R 64

static void draw_arrow(int x, int y)
{
	CAT_poly_clear_transformation();
	float t = CAT_timed_latch_get(&finish_latch) ?
	CAT_timed_latch_t(&finish_latch) * 4.0f : 0;

	float s = CAT_LCD_SCREEN_W/4;
	if(phase == PHASE_ARROWS_INCONGRUENT)
		s *= 0.5f;
	CAT_poly_set_scale(s, s);

	CAT_poly_set_rotation(angle_map[arrow_direction]);

	float dx = dxdy[arrow_direction*2+0] * t * 128;
	float dy = dxdy[arrow_direction*2+1] * t * 128;
	if(phase == PHASE_ARROWS_INCONGRUENT)
	{
		dx += dxdy[arrow_position*2+0] * ARROW_R;
		dy += dxdy[arrow_position*2+1] * ARROW_R;
	}
	CAT_poly_set_translation(x+dx, y+dy);
	
	CAT_poly_draw(arrow_mesh, CAT_POLY_VERTEX_COUNT(arrow_mesh), CAT_WHITE);
}

static void draw_error()
{
	CAT_poly_clear_transformation();
	float t = CAT_clamp(CAT_timed_latch_t(&error_latch) * 4.0f, 0, 1);
	float s = lerp(20, CAT_LCD_SCREEN_W/4, t);
	float r = lerp(-M_PI/3, 0, t);
	CAT_poly_set_scale(s, s);
	CAT_poly_set_translation(CAT_LCD_SCREEN_W/2, CAT_LCD_SCREEN_H/2);
	CAT_poly_set_rotation(r);
	CAT_poly_draw(x_mesh, CAT_POLY_VERTEX_COUNT(x_mesh), CAT_RED);
}

static void draw_distraction()
{

}

static void draw_gameplay()
{
	CAT_frameberry(CAT_BLACK);

	CAT_draw_regular_polygon(4, CAT_LCD_SCREEN_W/2, CAT_LCD_SCREEN_H/2, CAT_LCD_SCREEN_W/2, 0.25f, CAT_GREY);

	if(phase == PHASE_ARROWS_INCONGRUENT)
	{
		CAT_circberry(CAT_LCD_SCREEN_W/2, CAT_LCD_SCREEN_H/2, CENTER_R, CAT_160_GREY);
		CAT_draw_regular_polygon(4, CAT_LCD_SCREEN_W/2, CAT_LCD_SCREEN_H/2, CENTER_R, 0.25f, CAT_GREY);
	}

	draw_arrow(CENTER_X, CENTER_Y);

	if(CAT_timed_latch_get(&error_latch))
		draw_error();
}

static void MS_tutorial(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(draw_tutorial);
		break;

		case CAT_FSM_SIGNAL_TICK:
			int button = phase == PHASE_ARROWS_OPPOSITE ? CAT_BUTTON_LEFT : CAT_BUTTON_RIGHT;
			if(CAT_input_pressed(button))
				CAT_FSM_transition(&fsm, MS_gameplay);	
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

static void draw_tutorial_diagram(int y)
{
	bool pulse = CAT_pulse(0.5f);

	int x = CAT_LCD_SCREEN_W/2;

	CAT_poly_clear_transformation();
	CAT_poly_set_scale(32, 32);
	CAT_poly_set_translation(x, y);
	CAT_poly_draw(arrow_mesh, CAT_POLY_VERTEX_COUNT(arrow_mesh), pulse ? CAT_WHITE : CAT_GREY);
	y += 52;

	CAT_draw_dpad
	(
		x, y, 24,
		pulse ? (phase == PHASE_ARROWS_OPPOSITE ? CAT_LEFT_BIT : CAT_RIGHT_BIT) : 0,
		CAT_WHITE, CAT_BLACK
	);
	y += 52;
}

static void draw_tutorial()
{
	CAT_frameberry(CAT_BLACK);

	int cursor_y = 12;
	CAT_draw_page_markers(cursor_y, PHASE_COUNT, phase, CAT_WHITE);
	cursor_y += 32;

	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_textf(12, cursor_y, "PHASE %d\n", phase+1) + 12;

	CAT_set_text_mask(12, -1, CAT_LCD_SCREEN_W-12, -1);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	switch (phase)
	{
		case PHASE_ARROWS:
			cursor_y = CAT_draw_text(12, cursor_y, "Press the D-pad button corresponding to the arrow's direction.\n");
		break;
		case PHASE_ARROWS_OPPOSITE:
			cursor_y = CAT_draw_text(12, cursor_y, "Press the D-pad button corresponding to the OPPOSITE of the arrow's direction.\n");
		break;
		case PHASE_ARROWS_INCONGRUENT:
			cursor_y = CAT_draw_text(12, cursor_y, "Press the D-pad button corresponding to the arrow's direction, NOT its position.\n");
		break;
	}
	cursor_y += 32;

	draw_tutorial_diagram(cursor_y);
	cursor_y += 100;
	
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_mask(12, -1, CAT_LCD_SCREEN_W-12, -1);
	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER | CAT_TEXT_FLAG_WRAP);
	cursor_y = CAT_draw_text(CAT_LCD_SCREEN_W/2, cursor_y, "Press the indicated D-pad button to continue!\n");
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
			if(CAT_input_pressed(CAT_BUTTON_A) || CAT_input_dismissal())
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

	for(int i = 0; i < PHASE_COUNT; i++)
	{
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf
		(
			12, cursor_y,
			"PHASE %d:\n"
			"  Correct: %d/%d\n"
			"  Time: "CAT_FLOAT_FMT"\n"
			"\n",
			i+1,
			perfects[i], ARROWS_PER_PHASE, CAT_FMT_FLOAT(times[i])
		);
	}
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