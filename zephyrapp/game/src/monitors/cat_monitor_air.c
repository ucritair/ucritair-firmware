#include "cat_monitors.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_core.h"
#include "cat_aqi.h"
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include "cat_curves.h"

enum
{
	CO2,
	PM2_5,
	NOX,
	VOC,
	TEMP,
	RH,
	SUBSCORE_COUNT
};

static float score;
static float score_t;
static float subscores[SUBSCORE_COUNT];

static char textf_buf[32];

static int center_textf(int x, int y, int scale, uint16_t c, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(textf_buf, 32, fmt, args);
	va_end(args);

	int text_width = strlen(textf_buf) * CAT_GLYPH_WIDTH * scale;
	int center_x = x - text_width / 2;
	
	int text_height = CAT_GLYPH_HEIGHT * scale;
	int center_y = y - text_height / 2;

	CAT_push_text_scale(scale);
	CAT_push_text_colour(c);
	CAT_draw_text(center_x, center_y, textf_buf);

	return y + text_height;
}

static void vert_text(int x, int y, uint16_t c, const char* text)
{
	const char* g = text;
	x -= CAT_GLYPH_WIDTH/2;
	
	while(*g != '\0')
	{
		CAT_push_draw_colour(CAT_WHITE);
		CAT_draw_sprite(&glyph_sprite, *g, x, y);
		g++;
		y += CAT_GLYPH_HEIGHT + 1;
	}
}

#define SCORE_Y 76
#define SCORE_R1 40
#define SCORE_W 6
#define SCORE_R0 (SCORE_R1 - SCORE_W)

static uint16_t score_colours[3] =
{
	0xb985, // BAD
	0xf5aa, // MID
	0xd742, // GOOD
};
static uint16_t colour_score(float t)
{
	t = CAT_ease_inout_quad(t);

	float x = t * 2;
	int idx = (int) x;
	float frac = x - idx;
	uint16_t colour = CAT_RGB24216
	(
		CAT_RGB24_lerp
		(
			CAT_RGB16224(score_colours[idx]),
			CAT_RGB16224(score_colours[idx+1]),
			frac
		)
	);
	return colour;
}

#define BAR_Y 140
#define BAR_MARGIN 12
#define BAR_W (CAT_LCD_SCREEN_W - BAR_MARGIN*2)

#define DOT_MARGIN 4
#define DOT_D ((BAR_W - (SUBSCORE_COUNT-1) * DOT_MARGIN) / SUBSCORE_COUNT)

const char* subscore_titles[SUBSCORE_COUNT] =
{
	[CO2] = "CO2",
	[PM2_5] = "PM25",
	[NOX] = "NOX",
	[VOC] = "VOC",
	[TEMP] = "TEMP",
	[RH] = "RH"
};

static void draw_uninit_notif()
{
	CAT_push_text_colour(CAT_WHITE);
	CAT_push_text_scale(2);
	CAT_draw_text(12, 30, "Please wait...");
	CAT_push_text_colour(CAT_WHITE);
	CAT_push_text_line_width(CAT_LCD_SCREEN_W-24);
	CAT_push_text_flags(CAT_TEXT_FLAG_WRAP);
	CAT_draw_text(12, 64, "Air quality sensors are coming online.");
}

void CAT_monitor_render_summary()
{
	if(!CAT_is_AQ_initialized())
	{
		draw_uninit_notif();
		return;
	}

	int cursor_y = SCORE_Y;
	CAT_ringberry(120, cursor_y, SCORE_R1, SCORE_R0, colour_score(score_t), score_t, 1.25f-score_t*0.5f);
	cursor_y = center_textf(120, cursor_y, 3, CAT_WHITE, "%.0f", score);
	cursor_y += score_t >= 0.875 ? 20 : (score_t >= 0.65 ? 16 : 8);
	cursor_y = center_textf(120, cursor_y, 1, CAT_WHITE, "\4CritAQ Score");

	for(int i = 0; i < SUBSCORE_COUNT; i++)
	{
		int x = BAR_MARGIN + (DOT_D + DOT_MARGIN) * i + DOT_D/2;
		int y = BAR_Y + DOT_D/2;
		CAT_circberry(x, y, DOT_D/3, CAT_WHITE);
		vert_text(x, BAR_Y+DOT_D-1, CAT_WHITE, subscore_titles[i]);

		uint16_t dot_colour = colour_score(subscores[i]);
		CAT_discberry(x, y, DOT_D/4, dot_colour);
	}
}

static void score_bar(int x, int y, int score_idx)
{
	float subscore = 1-subscores[score_idx];
	int total_width = 16*4;
	int filled_width = total_width * subscore;
	uint16_t colour = colour_score(1-subscore);

	CAT_discberry(x, y, 4, colour);
	CAT_lineberry(x+4, y, x+4+filled_width, y, colour);
	CAT_lineberry(x+4+filled_width, y, x+4+total_width, y, CAT_WHITE);
	CAT_discberry(x+4+total_width+4, y, 4, colour);
}

static int labeled_scoref(int x, int y, uint16_t c, int score_idx, const char* stuff, const char* unit, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(textf_buf, 32, fmt, args);
	va_end(args);

	CAT_push_text_colour(c);
	CAT_draw_text(x, y-CAT_GLYPH_HEIGHT, stuff);
	x += strlen(stuff) * CAT_GLYPH_WIDTH + 8;
	CAT_push_text_scale(2);
	CAT_push_text_colour(c);
	CAT_draw_text(x, y-CAT_GLYPH_HEIGHT*2, textf_buf);
	x += strlen(textf_buf) * CAT_GLYPH_WIDTH*2 + 8;
	CAT_push_text_colour(c);
	CAT_draw_text(x, y-CAT_GLYPH_HEIGHT, unit);
	x += strlen(unit) == 0 ? 4 : strlen(unit) * CAT_GLYPH_WIDTH + 12;

	int y_off = strlen(unit) == 0 ? -CAT_GLYPH_HEIGHT : -CAT_GLYPH_HEIGHT/2;
	score_bar(x, y+y_off, score_idx);

	return y + CAT_GLYPH_HEIGHT*2 + 6;
}

void CAT_monitor_render_details()
{
	if(!CAT_is_AQ_initialized())
	{
		draw_uninit_notif();
		return;
	}

	int cursor_y = 64;	
	cursor_y = labeled_scoref(12, cursor_y, CAT_WHITE, CO2, "CO2", "ppm", "%.0f", readings.sunrise.ppm_filtered_compensated);
	cursor_y = labeled_scoref(12, cursor_y, CAT_WHITE, PM2_5, "PM2.5", "\4g/m\5", "%.0f", readings.sen5x.pm2_5);
	cursor_y = labeled_scoref(12, cursor_y, CAT_WHITE, NOX, "NOX", "", "%.0f", readings.sen5x.nox_index);
	cursor_y = labeled_scoref(12, cursor_y, CAT_WHITE, VOC, "VOC", "", "%.0f", readings.sen5x.voc_index);
	cursor_y = labeled_scoref(12, cursor_y, CAT_WHITE, TEMP, "TEMP", CAT_AQ_get_temperature_unit_string(), "%.0f", CAT_AQ_map_celsius(readings.sen5x.temp_degC));
	cursor_y = labeled_scoref(12, cursor_y, CAT_WHITE, RH, "RH", "\%", "%.0f", readings.sen5x.humidity_rhpct);
	cursor_y += 172;

	float pct_rebreathed = ((((double) readings.sunrise.ppm_filtered_compensated)-420.)/38000.)*100.;
	CAT_push_text_colour(CAT_WHITE);
	CAT_draw_textf(12, cursor_y, "%.1f%% rebreathed air", pct_rebreathed);
}

#define SPARKLINE_X 12
#define SPARKLINE_Y 32
#define SPARKLINE_PAD 4
#define SPARKLINE_WIDTH (CAT_LCD_SCREEN_W - 2 * SPARKLINE_X - 4 * CAT_GLYPH_WIDTH)
#define SPARKLINE_HEIGHT 36
#define SPARKLINE_MAX_X (SPARKLINE_X + SPARKLINE_WIDTH - 1)
#define SPARKLINE_MAX_Y (SPARKLINE_Y + SPARKLINE_HEIGHT - 1)
#define SPARKLINE_STEP_SIZE (round(SPARKLINE_WIDTH / 6.0f))
#define SPARKLINE_LABEL_X (SPARKLINE_MAX_X + SPARKLINE_PAD)

#define SPLN_I_Y(i) (SPARKLINE_Y + (SPARKLINE_HEIGHT + SPARKLINE_PAD) * (i))
#define SPLN_I_MAX_Y(i) (SPLN_I_Y(i) + SPARKLINE_HEIGHT-1)

#define SPARKLINE_BG_COLOUR 0xe7bf
#define SPARKLINE_FG_COLOUR 0x6b11
#define SPARKLINE_RETICLE_COLOUR 0xadfa

uint8_t get_sparkline_value(int idx, CAT_AQ_score_block* block)
{
	idx -= 1;
	switch(idx)
	{
		case CO2: return block->CO2;
		case PM2_5: return block->PM2_5;
		case NOX: return block->NOX;
		case VOC: return block->VOC;
		case TEMP: return block->temp;
		case RH : return block->rh;
		default: return block->aggregate;
	}
}

int sparkline_value_to_y(int idx, uint8_t value)
{
	float t = inv_lerp(value, 0, 255);
	int h = t * SPARKLINE_HEIGHT;
	return SPLN_I_MAX_Y(idx) - h;
}

void CAT_monitor_render_sparklines()
{
	if(CAT_AQ_get_stored_scores_count() < 2)
	{
		CAT_push_text_colour(CAT_WHITE);
		CAT_push_text_scale(2);
		CAT_push_text_line_width(CAT_LCD_SCREEN_W-24);
		CAT_push_text_flags(CAT_TEXT_FLAG_WRAP);
		CAT_draw_text(12, 30, "Not enough stored samples!");
		CAT_push_text_colour(CAT_WHITE);
		CAT_push_text_line_width(CAT_LCD_SCREEN_W-24);
		CAT_push_text_flags(CAT_TEXT_FLAG_WRAP);
		CAT_draw_text(12, 64, "At least 2 24-hour samples must be taken before the sparkline can be shown.");
		return;
	}

	CAT_fillberry(0, SPARKLINE_Y, 240, 320-SPARKLINE_Y, RGB8882565(35, 157, 235));

	for(int i = 0; i < SUBSCORE_COUNT+1; i++)
	{
		CAT_fillberry(SPARKLINE_X, SPLN_I_Y(i), SPARKLINE_WIDTH, SPARKLINE_HEIGHT, SPARKLINE_BG_COLOUR);
		CAT_CSCLIP_set_rect(SPARKLINE_X, SPLN_I_Y(i), SPARKLINE_MAX_X, SPLN_I_MAX_Y(i));
		for(int j = 0; j < CAT_AQ_get_stored_scores_count()-1; j++)
		{
			CAT_AQ_score_block s0;
			CAT_AQ_read_stored_scores(j, &s0);
			CAT_AQ_score_block s1;
			CAT_AQ_read_stored_scores(j+1, &s1);

			int x0 = SPARKLINE_X + SPARKLINE_STEP_SIZE * j;
			int y0 = sparkline_value_to_y(i, get_sparkline_value(i, &s0));
			int x1 = SPARKLINE_X + SPARKLINE_STEP_SIZE * (j+1);
			int y1 = sparkline_value_to_y(i, get_sparkline_value(i, &s1));

			if(CAT_CSCLIP(&x0, &y0, &x1, &y1))
			{
				CAT_lineberry(x0, y0, x1, y1, SPARKLINE_FG_COLOUR);
				CAT_discberry(x0, y0, 2, SPARKLINE_FG_COLOUR);
				CAT_discberry(x1, y1, 2, SPARKLINE_FG_COLOUR);
			}
		}
		
		CAT_push_text_colour(CAT_WHITE);
		CAT_draw_text
		(
			SPARKLINE_LABEL_X, SPLN_I_Y(i),
			i == 0 ? "\4CAQ" :
			subscore_titles[i-1]
		);
	}
}

static void update_scores()
{
	score = CAT_AQI_aggregate();
	score_t = score / 100.0f;
	
	subscores[CO2] = CAT_co2_score(readings.sunrise.ppm_filtered_compensated);
	subscores[PM2_5] = CAT_pm25_score(readings.sen5x.pm2_5);
	subscores[VOC] = CAT_voc_score(readings.sen5x.voc_index);
	subscores[NOX] = CAT_nox_score(readings.sen5x.nox_index);
	subscores[TEMP] = CAT_temperature_score(CAT_canonical_temp());
	subscores[RH] = CAT_humidity_score(readings.sen5x.humidity_rhpct);

	for(int i = 0; i < SUBSCORE_COUNT; i++)
		subscores[i] = inv_lerp(subscores[i], 5, 0);
}

void CAT_monitor_MS_air(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			if(CAT_input_released(CAT_BUTTON_START))
				CAT_monitor_exit();
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				CAT_monitor_retreat();
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				CAT_monitor_advance();

			update_scores();
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}