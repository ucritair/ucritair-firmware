#include "cat_arcade.h"
#include <stddef.h>
#include "cat_render.h"
#include "cat_gizmos.h"
#include "cat_gui.h"
#include "cat_effects.h"
#include "cat_curves.h"
#include "cat_colours.h"
#include "cat_input.h"
#include "cat_poly.h"
#include "cat_gizmos.h"
#include "cat_air.h"
#include "cat_math.h"
#include "cat_wifi.h"
#include "cat_leaderboard.h"
#include "cat_crypto.h"

#define CHALLENGES_PER_PHASE 8

enum
{
	PHASE_ARROWS,
	PHASE_ARROWS_OPPOSITE,
	PHASE_ARROWS_INCONGRUENT,
	PHASE_WORDS,
	PHASE_WORDS_COLOUR,
	PHASE_WORDS_WORD,
	PHASE_COUNT
};

#define TOTAL_CHALLENGES (PHASE_COUNT * CHALLENGES_PER_PHASE)

#define SURVEY_COUNT 2

static void MS_survey(CAT_FSM_signal);
static void draw_survey();
static void draw_tutorial();
static void MS_tutorial(CAT_FSM_signal);
static void draw_gameplay();
static void MS_gameplay(CAT_FSM_signal);
static void draw_performance();
static void MS_performance(CAT_FSM_signal);
static void MS_upload(CAT_FSM_signal);
static void draw_upload();

static void load_co2();
static void change_phase(int _phase);

static const char* survey_questions[SURVEY_COUNT] =
{
	"Overall Question",
	"Depression Question"
};

static const char** survey_answers[SURVEY_COUNT] =
{
	(const char*[])
	{
		"1 - Poor",
		"2 - Fair",
		"3 - Okay",
		"4 - Good",
		"5 - Excellent"
	},
	(const char*[])
	{
		"1 - Not at all",
		"2 - A little",
		"3 - Some",
		"4 - A lot",
		"5 - Almost always"
	}
};

static int survey_idx = 0;

static void survey_set_answer(int idx, int score)
{
	uint8_t mask = 0b1111 << ((SURVEY_COUNT-1-idx) * 4);
	survey_field &= mask;
	mask = CAT_clamp(score, 0, 15);
	mask <<= (idx*4);
	survey_field |= mask;
}

static int survey_get_answer(int idx)
{
	uint8_t mask = survey_field & (0b1111 << (idx*4));
	return mask >> (idx*4);
}

static void MS_survey(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(draw_survey);
			survey_field = 0;
			survey_idx = 0;
			CAT_gui_menu_force_reset();
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_gui_begin_menu("SURVEY"))
			{
				CAT_gui_menu_text(survey_questions[survey_idx]);
				for(int i = 0; i < 5; i++)
				{
					if
					(
						CAT_gui_menu_toggle
						(
							survey_answers[survey_idx][i],
							survey_get_answer(survey_idx) == i,
							CAT_GUI_TOGGLE_STYLE_RADIO_BUTTON
						)
					)
					{
						survey_set_answer(survey_idx, i);
					}
				}
				if(CAT_gui_menu_item("CONTINUE"))
				{
					if(survey_idx < SURVEY_COUNT-1)
						survey_idx += 1;
					else
					{
						load_co2();
						change_phase(PHASE_ARROWS);
					}
				}
				CAT_gui_end_menu();
			}
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		{
			
		}
		break;
	}
}

static void draw_survey()
{

}

static CAT_FSM fsm = {};

static int phase;
static float response_time[PHASE_COUNT][CHALLENGES_PER_PHASE];
static bool incongruent[PHASE_COUNT][CHALLENGES_PER_PHASE];
static bool errored[PHASE_COUNT][CHALLENGES_PER_PHASE];

static int arrow_direction;
static int arrow_position;

static const char* words[] =
{
	"RED",
	"GREEN",
	"BLUE",
	"YELLOW"
};

static uint16_t word_colours[] =
{
	CAT_RED,
	CAT_GREEN,
	CAT_BLUE,
	CAT_CRISIS_YELLOW,
};

static int word;
static int word_colour;
static int input_direction;

static bool challenge_request;
static int challenge_idx = 0;

static CAT_timed_latch error_latch = CAT_TIMED_LATCH_INIT(1.0f);
static CAT_timed_latch finish_latch = CAT_TIMED_LATCH_INIT(0.25f);
static CAT_timed_latch countdown_latch = CAT_TIMED_LATCH_INIT(1.5f);

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

static float cached_co2 = -1;

static void change_phase(int _phase)
{
	phase = _phase;
	challenge_idx = 0;

	for(int i = 0; i < CHALLENGES_PER_PHASE; i++)
	{
		response_time[phase][i] = 0;
		incongruent[phase][i] = false;
		errored[phase][i] = false;
	}

	challenge_request = true;
	CAT_FSM_transition(&fsm, MS_tutorial);
}

static void shuffle(int* arr, int n)
{
	for(int i = 0; i < n; i++)
	{
		int j = CAT_rand_int(0, n-1);
		int temp = arr[i];
		arr[i] = arr[j];
		arr[j] = temp;
	}
}

static void generate_arrow()
{
	static int arrow_pool[4];

	int arrow_idx = challenge_idx % 4;

	if(arrow_idx == 0)
	{
		for(int i = 0; i < 4; i++)
			arrow_pool[i] = i;
		shuffle(arrow_pool, 4);
	}

	arrow_direction = arrow_pool[arrow_idx];
	arrow_position = arrow_direction;
	if(phase == PHASE_ARROWS_INCONGRUENT && CAT_rand_chance(2))
	{
		arrow_position += 2;
		arrow_position %= 4;
		if(arrow_position != arrow_direction)
			incongruent[phase][challenge_idx] = true;
	}
}

static void generate_word()
{
	static int word_pool[4];

	int idx = challenge_idx % 4;

	if(idx == 0)
	{
		for(int i = 0; i < 4; i++)
			word_pool[i] = i;
		shuffle(word_pool, 4);
	}

	word = word_pool[idx];
	if(phase > PHASE_WORDS)
	{
		word_colour = word_pool[(idx+CAT_rand_int(1, 3))%4];
		if(word_colour != word)
			incongruent[phase][challenge_idx] = true;
	}
	else
		word_colour = word;
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
			response_time[phase][challenge_idx] += CAT_get_delta_time_s();

			CAT_timed_latch_tick(&error_latch);
			if(CAT_timed_latch_get(&error_latch))
				return;

			CAT_timed_latch_tick(&finish_latch);
			if(CAT_timed_latch_get(&finish_latch))
				return;
			else if(CAT_timed_latch_flipped(&finish_latch))
			{
				challenge_idx += 1;
				if(challenge_idx < CHALLENGES_PER_PHASE)
				{
					challenge_request = true;
				}
				else
				{
					if(phase >= PHASE_COUNT-1)
						CAT_FSM_transition(&fsm, MS_performance);
					else
						change_phase(phase+1);
					return;
				}
			}

			if(challenge_request)
			{
				if(phase >= PHASE_WORDS)
					generate_word();
				else
					generate_arrow();
				challenge_request = false;

				CAT_timed_latch_reset(&countdown_latch);
				CAT_timed_latch_raise(&countdown_latch);
			}

			input_direction = -1;
			for(int i = 0; i < 4; i++)
			{
				if(CAT_input_pressed(key_map[i]))
				{
					input_direction = i;
					break;
				}
			}	
			bool response = input_direction != -1;

			bool correct =
			((phase == PHASE_ARROWS || phase == PHASE_ARROWS_INCONGRUENT) && input_direction == arrow_direction) ||
			(phase == PHASE_ARROWS_OPPOSITE && input_direction == (arrow_direction+2)%4) ||
			(phase == PHASE_WORDS && input_direction == word) ||
			(phase == PHASE_WORDS_COLOUR && input_direction == word_colour) ||
			(phase == PHASE_WORDS_WORD && input_direction == word);

			CAT_timed_latch_tick(&countdown_latch);
			bool timeout =
			CAT_timed_latch_flipped(&countdown_latch) &&
			!CAT_timed_latch_get(&countdown_latch);

			if(response)
			{
				if(correct)
					CAT_timed_latch_raise(&finish_latch);
				else
				{
					errored[phase][challenge_idx] = true;
					CAT_timed_latch_raise(&error_latch);
					countdown_latch.timer = CAT_max(countdown_latch.timer, countdown_latch.timeout * 0.75f);
				}
			}
			if(timeout)
			{
				errored[phase][challenge_idx] = true;
				CAT_timed_latch_raise(&error_latch);
				CAT_timed_latch_raise(&finish_latch);
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

static void draw_arrow()
{
	float dt = CAT_get_uptime_ms() / 48000.0f;
	CAT_draw_regular_polygon(4, CAT_LCD_SCREEN_W/2, CAT_LCD_SCREEN_H/2, CAT_LCD_SCREEN_W/2, dt, CAT_64_GREY);
	CAT_draw_regular_polygon(4, CAT_LCD_SCREEN_W/2, CAT_LCD_SCREEN_H/2, CAT_LCD_SCREEN_W/2, 0.25f, CAT_96_GREY);
	if(phase == PHASE_ARROWS_INCONGRUENT)
		CAT_draw_regular_polygon(4, CAT_LCD_SCREEN_W/2, CAT_LCD_SCREEN_H/2, CENTER_R, -dt, CAT_GREY);

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
	CAT_poly_set_translation(CENTER_X+dx, CENTER_Y+dy);
	
	CAT_poly_draw(arrow_mesh, CAT_POLY_VERTEX_COUNT(arrow_mesh), CAT_WHITE);
}

#define CARD_W (3*CAT_LCD_SCREEN_W/4)
#define CARD_H CARD_W
#define CARD_X (CENTER_X-CARD_W/2)
#define CARD_Y (CENTER_Y-CARD_H/2)

static void draw_word()
{
	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER | CAT_TEXT_FLAG_VERTICAL);
	CAT_set_text_colour(word_colours[0]);
	CAT_draw_text(CARD_X + CARD_W + 7, CARD_Y + CARD_H/2, words[0]);

	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
	CAT_set_text_colour(word_colours[1]);
	CAT_draw_text(CARD_X + CARD_W/2, CARD_Y - 24, words[1]);

	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER | CAT_TEXT_FLAG_VERTICAL);
	CAT_set_text_colour(word_colours[2]);
	CAT_draw_text(CARD_X - 20, CARD_Y + CARD_H/2, words[2]);

	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
	CAT_set_text_colour(word_colours[3]);
	CAT_draw_text(CARD_X + CARD_W/2, CARD_Y + CARD_H + 12, words[3]);

	float t = CAT_timed_latch_get(&finish_latch) ?
	CAT_timed_latch_t(&finish_latch) * 4.0f : 0;

	int dx = dxdy[input_direction*2+0] * t * 128;
	int dy = dxdy[input_direction*2+1] * t * 128;
	
	CAT_fillberry(CARD_X+dx, CARD_Y+dy, CARD_W+1, CARD_H+1, CAT_BLACK);
	CAT_strokeberry(CARD_X+dx, CARD_Y+dy, CARD_W+1, CARD_H+1, CAT_GREY);
	CAT_strokeberry(CARD_X+dx, CARD_Y+dy, CARD_W, CARD_H, CAT_WHITE);
	
	CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
	CAT_set_text_scale(2);
	CAT_set_text_colour(word_colours[word_colour]);
	CAT_draw_text(CARD_X+dx+CARD_W/2, CARD_Y+dy+CARD_H/2-CAT_GLYPH_HEIGHT, words[word]);
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

static void draw_countdown()
{
	CAT_draw_progress_bar(CAT_LCD_SCREEN_W/2, 12, CAT_LCD_SCREEN_W-24, 12, CAT_WHITE, CAT_WHITE, 1.0f-CAT_timed_latch_t(&countdown_latch));
}

static void draw_gameplay()
{
	CAT_frameberry(CAT_BLACK);

	if(fsm.dirty)
		return;

	if(phase >= PHASE_WORDS)
		draw_word();
	else
		draw_arrow();

	if(!CAT_timed_latch_get(&error_latch) && !CAT_timed_latch_get(&finish_latch))
		draw_countdown();
	
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
		{
			int button = (phase == PHASE_ARROWS_OPPOSITE) ? CAT_BUTTON_LEFT : CAT_BUTTON_RIGHT;
			if(CAT_input_pressed(button))
				CAT_FSM_transition(&fsm, MS_gameplay);
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

static void draw_tutorial_diagram(int y)
{
	bool pulse = CAT_pulse(0.5f);
	int x = CAT_LCD_SCREEN_W/2;
	y -= 16;

	if(phase >= PHASE_WORDS)
	{
		int cursor_y = y;

		const char* text[] =
		{
			"<c%d>RED</c> \11 <c%d>RED</c>\n<c%d>BLUE</c> \11 <c%d>BLUE</c>\n",
			"<c%d>BLUE</c> \11 <c%d>RED</c>\n<c%d>RED</c> \11 <c%d>BLUE</c>\n",
			"<c%d>RED</c> \11 <c%d>RED</c>\n<c%d>BLUE</c> \11 <c%d>BLUE</c>\n",
		};
		const uint16_t colours[] =
		{
			CAT_RED, CAT_RED, CAT_BLUE, CAT_BLUE,
			CAT_RED, CAT_RED, CAT_BLUE, CAT_BLUE,
			CAT_BLUE, CAT_RED, CAT_RED, CAT_BLUE
		};

		int idx = phase-PHASE_WORDS;
		CAT_set_text_flags(CAT_TEXT_FLAG_CENTER);
		CAT_set_text_colour(CAT_WHITE);
		cursor_y = CAT_draw_textf
		(
			120, cursor_y,
			text[idx],
			colours[idx*4+0], colours[idx*4+1],
			colours[idx*4+2], colours[idx*4+3]
		);
		cursor_y += CAT_TEXT_LINE_HEIGHT * 3;

		CAT_draw_dpad
		(
			x, cursor_y, 20,
			pulse ? CAT_RIGHT_BIT : 0,
			CAT_WHITE, CAT_BLACK
		);
	}
	else
	{
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
	}
}

static void draw_tutorial()
{
	CAT_frameberry(CAT_BLACK);

	if(fsm.dirty)
		return;

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
		case PHASE_WORDS:
			cursor_y = CAT_draw_text(12, cursor_y, "Press the D-pad button corresponding to the COLOUR of the text at the box's center.\n");
		break;
		case PHASE_WORDS_COLOUR:
			cursor_y = CAT_draw_text(12, cursor_y, "Press the D-pad button corresponding to the COLOUR of the text at the box's center.\n");
		break;
		case PHASE_WORDS_WORD:
			cursor_y = CAT_draw_text(12, cursor_y, "Press the D-pad button corresponding to the MEANING of the text at the box's center.\n");
		break;
		default:
			cursor_y = CAT_draw_text(12, cursor_y, "Hello, world!\n");
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

static float co2 = 0;
static float pm25 = 0;
static float temp = 0;

static int perfect = 0;
static int stars = 0;

static void MS_performance(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(draw_performance);

			co2 = cached_co2 < 0 ? CAT_AQ_value(CAT_AQM_CO2) : cached_co2;
			pm25 = CAT_AQ_value(CAT_AQM_PM2_5);
			temp = CAT_AQ_value(CAT_AQM_TEMP);
			
			int incong_challenges = 0;
			float total_time = 0;

			stroop_data.mean_time_cong = 0;
			stroop_data.mean_time_incong = 0;
			stroop_data.throughput = 0;
			
			perfect = 0;
			stars = 0;
			
			for(int i = 0; i < PHASE_COUNT; i++)
			{
				for(int j = 0; j < CHALLENGES_PER_PHASE; j++)
				{
					if(incongruent[i][j])
					{
						incong_challenges += 1;
						stroop_data.mean_time_incong += response_time[i][j];
					}
					else
						stroop_data.mean_time_cong += response_time[i][j];
					total_time += response_time[i][j];

					if(!errored[i][j])
						perfect += 1;
				}
			}
			stroop_data.mean_time_cong /= (TOTAL_CHALLENGES - incong_challenges);
			stroop_data.mean_time_incong /= incong_challenges;
			stroop_data.throughput = (TOTAL_CHALLENGES / total_time) * 60.0f;
			stroop_data_valid = true;
			stroop_correctness = perfect / (float) TOTAL_CHALLENGES;
			
			stars = (int)(stroop_correctness >= 0.5f) + (int)(stroop_correctness >= 0.85f) + (int)((stroop_data.mean_time_incong/stroop_data.mean_time_cong) <= 1.25f);
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_A) || CAT_input_dismissal())
				CAT_FSM_transition(&fsm, MS_upload);
		}			
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

static void draw_performance()
{
	CAT_frameberry(CAT_BLACK);

	if(fsm.dirty)
		return;

	int cursor_y = 12;
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_text(12, cursor_y, "PERFORMANCE\n");
	cursor_y += CAT_TEXT_LINE_HEIGHT;

	float ratio = stroop_data.mean_time_incong / stroop_data.mean_time_cong;
	CAT_set_text_mask(12, -1, CAT_LCD_SCREEN_W-12, -1);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	cursor_y = CAT_draw_textf
	(
		12, cursor_y,
		"%d/%d tasks performed perfectly. "
		"Tricky tasks solved "CAT_FLOAT_FMT"x as %s as typical tasks. "
		"Overall cognitive performence rated"
		"\n\n",
		perfect, TOTAL_CHALLENGES,
		CAT_FMT_FLOAT(ratio > 1.0f ? ratio : 1.0f/ratio), ratio > 1.0f ? "slow" : "fast"
	);

	CAT_set_text_colour(CAT_CRISIS_YELLOW);
	CAT_set_text_scale(2);
	cursor_y = CAT_draw_textf
	(
		12, cursor_y,
		"%d %s\n",
		stars, stars == 1 ? "STAR" : "STARS"
	);
	cursor_y += 32;
	for(int i = 0; i < 3; i++)
	{
		CAT_draw_star(12+ 24 + i * (48 + 8), cursor_y, 24, i < stars ? CAT_CRISIS_YELLOW : CAT_64_GREY);
	}
	cursor_y += 32;

	CAT_set_text_mask(12, -1, CAT_LCD_SCREEN_W-12, -1);
	CAT_set_text_colour(CAT_WHITE);
	CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
	cursor_y = CAT_draw_textf
	(
		12, cursor_y,
		"Air quality can affect performance. "
		"Recent CO2 is %d %s, current PM 2.5 is "CAT_FLOAT_FMT" %s, "
		"and the temperature is %d %s."
		"\n",
		(int) co2, CAT_AQ_unit_string(CAT_AQM_CO2),
		CAT_FMT_FLOAT(pm25), CAT_AQ_unit_string(CAT_AQM_PM2_5),
		(int) CAT_AQ_map_celsius(temp), CAT_AQ_unit_string(CAT_AQM_TEMP)
	);
}

static bool upload_complete = false;

static void MS_upload(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(draw_upload);
			upload_complete = false;
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(!upload_complete)
			{
				uint32_t data[CAT_WIFI_DATUM_COUNT] = 
				{
					CAT_ZK_CO2(),
					CAT_ZK_PM2_5(),
					CAT_ZK_temp(),
					CAT_ZK_stroop(),
					CAT_ZK_survey()
				};
				CAT_wifi_send_data(data, CAT_WIFI_DATUM_COUNT, 120000);
				upload_complete = true;
			}
			else
			{
				if(CAT_input_pressed(CAT_BUTTON_A) || CAT_input_dismissal())
					CAT_FSM_transition(&fsm, NULL);
			}
		}			
		break;

		case CAT_FSM_SIGNAL_EXIT:
		{

		}
		break;
	}
}

static void draw_upload()
{
	CAT_frameberry(CAT_BLACK);

	if(!upload_complete)
	{
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_text(12, 12, "Uploading...");
	}
	else
	{
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_text(12, 12, "Upload complete!");
	}
}

static void load_co2()
{
	cached_co2 = -1;
	if(CAT_logs_initialized())
	{
		uint64_t now = CAT_get_RTC_now();
		int awake = CAT_get_uptime_ms() / 1000;
		awake = CAT_min(awake, CAT_MINUTE_SECONDS * 15);
		uint64_t picked_up = now - awake;
		uint64_t half_before = picked_up - CAT_HOUR_SECONDS/2;

		CAT_log_cell cell;
		int idx = CAT_read_log_cell_before_time(-1, half_before, &cell);
		int count = 0;

		if(idx < 3)
			cached_co2 = -1;
		else
		{
			for(int i = idx+1; i < CAT_get_log_cell_count()-1; i++)
			{
				cached_co2 += cell.co2_ppmx1;
				count++;

				CAT_read_log_cell_at_idx(i, &cell);
				if(cell.timestamp > picked_up)
					break;
			}
		}
		cached_co2 /= count;
	}
}

void CAT_MS_stroop(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(CAT_render_stroop);
			CAT_FSM_transition(&fsm, MS_survey);
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_gui_popup_is_open())
				return;
			if(CAT_gui_consume_popup())
				CAT_pushdown_pop();
			
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_gui_open_popup("Quit this Stroop test?", CAT_POPUP_STYLE_YES_NO);

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
	CAT_frameberry(CAT_BLACK);
}