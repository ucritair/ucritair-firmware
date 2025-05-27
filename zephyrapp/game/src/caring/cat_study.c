#include "cat_actions.h"

#include "cat_render.h"
#include "cat_input.h"
#include "cat_room.h"
#include <math.h>
#include "cowtools/cat_curves.h"
#include "sprite_assets.h"
#include "cowtools/cat_structures.h"

#define SCREEN_DIAG 400
#define BLACK_OUT_DURATION 1.0f
#define NIBBLE_MIN_WAIT 1.5f
#define NIBBLE_MAX_WAIT 3.5f
#define VULN_FRAMES 5
#define BITE_CHANCE 5
#define RETARGET_TIMEOUT 3
#define UNFOCUSED_SPEED 48
#define FOCUSED_SPEED 64
#define ELLIPSE_MARGIN 12
#define EXIT_SPEED 128
#define BAR_MIN_WAIT 2
#define BAR_MAX_WAIT 5
#define BAR_ERROR_MARGIN 0.1

void MS_cast(CAT_machine_signal signal);
void render_MS_cast();
void MS_fish(CAT_machine_signal signal);
void render_MS_fish();
void MS_catch(CAT_machine_signal signal);
void render_MS_catch();
void MS_fail(CAT_machine_signal signal);
void render_MS_fail();
void MS_succeed(CAT_machine_signal signal);
void render_MS_succeed();
void MS_summary(CAT_machine_signal signal);
void render_MS_summary();

typedef struct
{
	CAT_vec2 center;
	float max_radius;
	float speed;

	uint16_t colour;
	bool fill;

	float radius;
} study_ring;
study_ring rings[16];
int ring_count = 0;

void init_rings()
{
	ring_count = 0;
}

int spawn_ring(CAT_vec2 center, float max_radius, float speed)
{
	if(ring_count >= 16)
		return -1;
	int idx = ring_count;
	study_ring* ring = &rings[idx];
	ring->center = center;
	ring->max_radius = max_radius;
	ring->speed = speed;
	ring->colour = CAT_WHITE;
	ring->fill = false;
	ring->radius = 0;
	ring_count += 1;
	return idx;
}

void customize_ring(int idx, uint16_t colour, bool fill)
{
	if(idx < 0 || idx >= ring_count)
		return;
	study_ring* ring = &rings[idx];
	ring->colour = colour;
	ring->fill = fill;
}

void swap_rings(int i, int j)
{
	study_ring temp = rings[i];
	rings[i] = rings[j];
	rings[j] = temp;
}

void rings_tick()
{
	for(int i = 0; i < ring_count; i++)
	{
		rings[i].radius += CAT_get_delta_time_s() * rings[i].speed;
		if(rings[i].radius / rings[i].max_radius >= 1)
		{
			ring_count -= 1;
			swap_rings(ring_count, i);
		}
	}
}

void render_rings()
{
	for(int i = 0; i < ring_count; i++)
	{
		study_ring* ring = &rings[i];
		if(ring->fill)
			CAT_discberry(ring->center.x, ring->center.y, ring->radius, ring->colour);
		else
			CAT_circberry(ring->center.x, ring->center.y, ring->radius, ring->colour);
	}
}

/*void render_beach_curve()
{
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();
	uint16_t base = ADAPT_DESKTOP_COLOUR(BEACH_BASE_COLOUR);
	uint16_t edge = ADAPT_DESKTOP_COLOUR(BEACH_EDGE_COLOUR);

	for(int x = 0; x < 240; x++)
	{
		int y_f = 320 - (beach_base_curve[x]);
		y_f -= FRAMEBUFFER_ROW_OFFSET;
		if(y_f < 0)
			y_f = 0;
		if(y_f > CAT_LCD_FRAMEBUFFER_H)
			continue;

		int y_m = y_f + beach_edge_curve[x];
		for(int y_w = y_f; y_w < y_m; y_w++)
			framebuffer[y_w * CAT_LCD_FRAMEBUFFER_W + x] = edge;
		for(int y_w = y_m; y_w < CAT_LCD_FRAMEBUFFER_H; y_w++)
			framebuffer[y_w * CAT_LCD_FRAMEBUFFER_W + x] = base;
	}
}*/

struct
{
	CAT_ivec2 center;
	int length;

	float target;
	float guess;
	bool committed;
	float error;

	int vertices[10*2];
} pole;

void init_pole(CAT_ivec2 center, int length)
{
	pole.center = center;
	pole.length = length;

	pole.target = CAT_rand_float(0.2, 0.8);
	pole.guess = 0;
	pole.committed = false;
	pole.error = 0;

	int top_x = 0; int top_y = -length/2;
	int bottom_x = 0; int bottom_y = length/2;
	int top_left_x = top_x - 8; int top_left_y = top_y + 8;
	int top_right_x = top_x + 8; int top_right_y = top_y + 8;
	int bottom_left_x = bottom_x - 8; int bottom_left_y = bottom_y - 8;
	int bottom_right_x = bottom_x + 8; int bottom_right_y = bottom_y - 8;
	pole.vertices[0] = top_x;
	pole.vertices[1] = top_y;
	pole.vertices[2] = top_left_x;
	pole.vertices[3] = top_left_y;
	pole.vertices[4] = top_x;
	pole.vertices[5] = top_y;
	pole.vertices[6] = top_right_x;
	pole.vertices[7] = top_right_y;
	pole.vertices[8] = top_x;
	pole.vertices[9] = top_y;
	pole.vertices[10] = bottom_x;
	pole.vertices[11] = bottom_y;
	pole.vertices[12] = bottom_x;
	pole.vertices[13] = bottom_y;
	pole.vertices[14] = bottom_left_x;
	pole.vertices[15] = bottom_left_y;
	pole.vertices[16] = bottom_x;
	pole.vertices[17] = bottom_y;
	pole.vertices[18] = bottom_right_x;
	pole.vertices[19] = bottom_right_y;
}

CAT_ivec2 point_on_pole(float t)
{
	int x = pole.center.x;
	int y =
	pole.center.y + pole.length / 2 -
	pole.length * t;
	return (CAT_ivec2) {x, y};
}

void render_pole()
{
	CAT_polyberry
	(
		pole.center.x, pole.center.y,
		pole.vertices, 10,
		CAT_WHITE,
		CAT_POLY_MODE_LINES
	);

	CAT_ivec2 target = point_on_pole(pole.target);
	CAT_ivec2 guess = point_on_pole(pole.guess);
	CAT_discberry(target.x, target.y, pole.committed ? 3 : 4, CAT_RED);
	CAT_circberry(guess.x, guess.y, 6, CAT_WHITE);
	if(pole.committed)
		CAT_circberry(target.x, target.y, pole.error * pole.length, CAT_RED);
}

float cast_t = 0.0f;
float cast_dir = 1.0f;

bool blacking_out = false;
float black_out_t = 0.0f;

void MS_cast(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(render_MS_cast);
			init_rings();
			init_pole((CAT_ivec2){CAT_LCD_SCREEN_W/2, CAT_LCD_SCREEN_H/2}, CAT_LCD_SCREEN_H * 0.75f);

			cast_t = 0;
			cast_dir = 0;

			blacking_out = false;
			black_out_t = 0;
		}
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				if(!pole.committed)
				{
					pole.committed = true;
					pole.error = fabs(pole.target - pole.guess);

					spawn_ring(CAT_iv2v(point_on_pole(pole.target)), SCREEN_DIAG, 320);
				}
				else if(!blacking_out)
				{
					blacking_out = true;
					customize_ring
					(
						spawn_ring
						(
							CAT_iv2v(point_on_pole(pole.target)),
							SCREEN_DIAG * 2, SCREEN_DIAG / BLACK_OUT_DURATION
						),
						CAT_BLACK, true
					);
				}
			}

			if(!pole.committed)
			{
				cast_t += cast_dir * CAT_get_delta_time_s() / 3.0f;
				if(cast_t >= 0.99f)
					cast_dir = -1.0f;
				else if(cast_t <= 0.01f)
					cast_dir = 1.0f;
				pole.guess = lerp(0.1, 0.9, CAT_ease_inout_back(cast_t));
			}
			else if(blacking_out)
			{
				black_out_t += CAT_get_delta_time_s();
				if(black_out_t >= BLACK_OUT_DURATION)
					CAT_machine_transition(MS_fish);
			}

			rings_tick();
		}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void render_MS_cast()
{
	CAT_frameberry(CAT_BLACK);
	render_pole();
	render_rings();
}

CAT_vec2 hook;

typedef struct
{
	const char* name;
	const char* proverb;

	float min_length;
	float max_length;
	float min_lustre;
	float max_lustre;
	float min_wisdom;
	float max_wisdom;
} study_fish_type;

const study_fish_type horse_mackerel =
{
	.name = "Horse Mackerel",
	.proverb = "I can do all things\nthrough Christ who\nstrengthens me.",
	.min_length = 0.15, .max_length = 0.4,
	.min_lustre = 0.5, .max_lustre = 0.8,
	.min_wisdom = 0.1, .max_wisdom = 0.3
};

const study_fish_type* fish_types[1] =
{
	&horse_mackerel
};
#define FISH_TYPE_COUNT (sizeof(fish_types)/sizeof(fish_types[0]))

struct
{
	const study_fish_type* type;
	float length;
	float lustre;
	float wisdom;

	CAT_vec2 positions[3];
	CAT_vec2 headings[3];
	float radii[3];

	int retarget_timer_id;
	bool focus_trigger;

	bool nibble_trigger;
	int nibble_timer_id;
	bool bite_trigger;

	bool vuln_trigger;
	int vuln_frame_counter;
} fish =
{
	.nibble_timer_id = -1,
};

void init_fish(CAT_vec2 lead_position, CAT_vec2 lead_heading, float lead_radius)
{
	fish.type = fish_types[CAT_rand_int(0, FISH_TYPE_COUNT-1)];
	fish.length = CAT_rand_float(fish.type->min_length, fish.type->max_length);
	fish.lustre = CAT_rand_float(fish.type->min_lustre, fish.type->max_lustre);
	fish.wisdom = CAT_rand_float(fish.type->min_wisdom, fish.type->max_wisdom);

	fish.positions[0] = lead_position;
	fish.headings[0] = CAT_vec2_unit(lead_heading);
	fish.radii[0] = lead_radius;
	for(int i = 1; i < 3; i++)
	{
		CAT_vec2 heading = fish.headings[i-1];
		float interspace = fish.radii[i-1] + fish.radii[0] / 2 + fish.radii[i];
		CAT_vec2 position = CAT_vec2_add(fish.positions[i-1], CAT_vec2_mul(heading, -interspace));
		fish.headings[i] = heading;
		fish.positions[i] = position;
	}

	for(int i = 1; i < 3; i++)
	{
		fish.radii[i] = lerp(lead_radius, lead_radius * 0.25f, (float) i / (float) 2);
		CAT_vec2 offset = CAT_vec2_mul(lead_heading, -1 * (fish.radii[i-1] + lead_radius / 2 + fish.radii[i]));
		fish.positions[i] = CAT_vec2_add(fish.positions[i-1], offset);
		fish.headings[i] = lead_heading;	
	}

	CAT_timer_reinit(&fish.retarget_timer_id, RETARGET_TIMEOUT);
	fish.focus_trigger = false;

	fish.nibble_trigger = false;
	CAT_timer_reinit(&fish.nibble_timer_id, 3.0f);
	fish.bite_trigger = false;

	fish.vuln_trigger = false;
	fish.vuln_frame_counter = 0;
}

void fish_pbd_default()
{
	for(int i = 1; i < 3; i++)
	{
		CAT_vec2 heading = CAT_vec2_unit(CAT_vec2_sub(fish.positions[i-1], fish.positions[i]));
		float interspace = fish.radii[i-1] + fish.radii[0] / 2 + fish.radii[i];
		CAT_vec2 position = CAT_vec2_add(fish.positions[i-1], CAT_vec2_mul(heading, -interspace));
		fish.headings[i] = heading;
		fish.positions[i] = position;
	}
}

void fish_pbd_waggle()
{
	for(int i = 1; i < 3; i++)
	{
		CAT_vec2 heading = fish.headings[i-1];
		float amplitude = 0.5f * sin(CAT_get_uptime_ms() / 1000.0f + (2-i));
		heading = CAT_vec2_rotate(heading, amplitude);
		float interspace = fish.radii[i-1] + fish.radii[0] / 2 + fish.radii[i];
		CAT_vec2 position = CAT_vec2_add(fish.positions[i-1], CAT_vec2_mul(heading, -interspace));
		fish.headings[i] = heading;
		fish.positions[i] = position;
	}
}

void fish_integrate_heading(CAT_vec2 heading, float speed)
{
	fish.headings[0] = heading;
	fish.positions[0] = CAT_vec2_add
	(
		fish.positions[0],
		CAT_vec2_mul
		(
			fish.headings[0], 
			speed * CAT_get_delta_time_s()
		)
	);
}

CAT_vec2 fish_ellipse_position(float t)
{
	t = t * 2 * M_PI;
	return (CAT_vec2)
	{
		(120 - ELLIPSE_MARGIN) * cos(t) + 120,
		(160 - ELLIPSE_MARGIN) * sin(t) + 160
	};
}

CAT_vec2 fish_ellipse_heading(float t)
{
	t = t * 2 * M_PI;
	return CAT_vec2_unit((CAT_vec2) 
	{
		-(120 - ELLIPSE_MARGIN) * sin(t),
		(160 - ELLIPSE_MARGIN) * cos(t)
	});
}

void fish_integrate_ellipse(float t)
{
	fish.headings[0] = fish_ellipse_heading(t);
	fish.positions[0] = fish_ellipse_position(t);
}

int bite_prob_backing[BITE_CHANCE];
CAT_int_list bite_prob = 
{
	.capacity = BITE_CHANCE,
	.data = bite_prob_backing,
	.length = 0
};

void init_bite_probability()
{
	CAT_ilist(&bite_prob, bite_prob_backing, BITE_CHANCE);
	for(int i = 0; i < 5; i++)
		CAT_ilist_push(&bite_prob, i == 0);
	CAT_ilist_shuffle(&bite_prob);
}

bool draw_bite_probability()
{
	if(bite_prob.length == 0)
		init_bite_probability();
	bool value = CAT_ilist_pop(&bite_prob);
	if(value)
		init_bite_probability();
	return value;
}

void nibble_burst(int n)
{
	for(int i = n-1; i >= 0; i--)
	{
		int ring_idx = spawn_ring(hook, 400, 64);
		rings[ring_idx].radius += i * 12;
	}
}

void fish_tick()
{
	CAT_vec2 beeline = CAT_vec2_sub(hook, fish.positions[0]);
	float distance = sqrt(CAT_vec2_mag2(beeline));
	CAT_vec2 axis = CAT_vec2_div(beeline, distance);
	float align = CAT_vec2_dot(fish.headings[0], axis);

	if(fish.nibble_trigger)
	{
		if(CAT_timer_tick(fish.nibble_timer_id))
		{
			if(draw_bite_probability())
			{
				fish.vuln_trigger = true;
				fish.vuln_frame_counter = 0;
				fish.bite_trigger = true;
				nibble_burst(3);
			}
			else
			{
				nibble_burst(CAT_rand_int(1, 3));
			}
			CAT_timer_reinit(&fish.nibble_timer_id, CAT_rand_float(NIBBLE_MIN_WAIT, NIBBLE_MAX_WAIT));
		}
	}
	else if(fish.focus_trigger)
	{
		if(distance > fish.radii[0])
			fish_integrate_heading(axis, FOCUSED_SPEED);
		else
		{
			fish.nibble_trigger = true;
			CAT_timer_reinit(&fish.nibble_timer_id, CAT_rand_float(NIBBLE_MIN_WAIT, NIBBLE_MAX_WAIT));
		}
	}
	else
	{
		if(align > 0.7 && distance < fish.radii[0] * 3)
			fish.focus_trigger = true;
		fish_integrate_ellipse(CAT_get_uptime_ms() / 20000.0f);
	}

	if(fish.vuln_trigger)
	{	
		if(fish.vuln_frame_counter > VULN_FRAMES)
			fish.vuln_trigger = false;
		fish.vuln_frame_counter += 1;
	}

	if(fish.nibble_trigger)
		fish_pbd_waggle();
	else
		fish_pbd_default();
}

void render_fish(uint16_t colour)
{
	for(int i = 0; i < 3; i++)
		CAT_discberry(fish.positions[i].x, fish.positions[i].y, fish.radii[i], colour);
	for(int i = 1; i < 3; i++)
		CAT_lineberry(fish.positions[i].x, fish.positions[i].y, fish.positions[i-1].x, fish.positions[i-1].y, colour);
	CAT_vec2 left = CAT_vec2_unit((CAT_vec2) {fish.headings[0].y, -fish.headings[0].x});
	CAT_vec2 right = CAT_vec2_mul(left, -1);
	CAT_vec2 anchor = CAT_vec2_add(fish.positions[0], CAT_vec2_mul(fish.headings[0], -fish.radii[0] * 1.5f));
	CAT_vec2 left_tip = CAT_vec2_add(anchor, CAT_vec2_mul(left, fish.radii[0] * 1.5f));
	CAT_vec2 right_tip = CAT_vec2_add(anchor, CAT_vec2_mul(right, fish.radii[0] * 1.5f));
	CAT_lineberry(fish.positions[0].x, fish.positions[0].y, left_tip.x, left_tip.y, colour);
	CAT_lineberry(fish.positions[0].x, fish.positions[0].y, right_tip.x, right_tip.y, colour);
}

void MS_fish(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(render_MS_fish);
			init_rings();
			init_fish((CAT_vec2) {120, 60}, (CAT_vec2) {CAT_rand_float(-1, 1), CAT_rand_float(-1, 1)}, 16.0f);

			hook = (CAT_vec2) {120, 160};
			init_bite_probability();
		}
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);

			if(!fish.nibble_trigger)
			{
				if(CAT_input_touching())
				{
					hook = CAT_iv2v(CAT_input_cursor());
				}
				else
				{
					float speed = 96 * CAT_get_delta_time_s();
					if(CAT_input_held(CAT_BUTTON_UP, 0))
						hook.y -= speed;
					if(CAT_input_held(CAT_BUTTON_DOWN, 0))
						hook.y += speed;
					if(CAT_input_held(CAT_BUTTON_LEFT, 0))
						hook.x -= speed;
					if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
						hook.x += speed;
				}
				hook.x = clamp(hook.x, 0, CAT_LCD_SCREEN_W-1);
				hook.y = clamp(hook.y, 0, CAT_LCD_SCREEN_H-1);
			}
			else
			{
				if(CAT_input_pressed(CAT_BUTTON_A))
				{
					if(fish.vuln_trigger)
						CAT_machine_transition(MS_catch);
					else
						CAT_machine_transition(MS_fail);
				}
			}

			fish_tick();
			rings_tick();	
		}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void render_MS_fish()
{
	CAT_frameberry(CAT_BLACK);
	render_fish(CAT_RED);
	render_rings();

	if(!fish.bite_trigger)
	{
		CAT_push_draw_colour(CAT_WHITE);
		CAT_push_draw_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
		CAT_draw_sprite(&gizmo_target_17x17_sprite, 0, hook.x, hook.y);
	}
}

struct
{
	float target;
	float next;
	float wait;
	float waiter;
	float cursor;
	float progress;

	CAT_ivec2 center;
	int length;
	int vertices[10*2];
} bar;

void init_bar(CAT_ivec2 center, int length)
{
	bar.target = CAT_rand_float(0, 1);
	bar.next = CAT_rand_float(0, 1);
	bar.wait = CAT_rand_float(BAR_MIN_WAIT, BAR_MAX_WAIT);
	bar.waiter = 0;
	bar.cursor = 0.5f;
	bar.progress = 0.15f;

	bar.center = center;
	bar.length = length;

	int left_y = 0; int left_x = -length/2;
	int right_y = 0; int right_x = length/2;
	int left_top_y = left_y - 8; int left_top_x = left_x + 8;
	int left_bottom_y = left_y + 8; int left_bottom_x = left_x + 8;
	int right_top_y = right_y - 8; int right_top_x = right_x - 8;
	int right_bottom_y = right_y + 8; int right_bottom_x = right_x - 8;
	bar.vertices[0] = left_x;
	bar.vertices[1] = left_y;
	bar.vertices[2] = left_top_x;
	bar.vertices[3] = left_top_y;
	bar.vertices[4] = left_x;
	bar.vertices[5] = left_y;
	bar.vertices[6] = left_bottom_x;
	bar.vertices[7] = left_bottom_y;
	bar.vertices[8] = left_x;
	bar.vertices[9] = left_y;
	bar.vertices[10] = right_x;
	bar.vertices[11] = right_y;
	bar.vertices[12] = right_x;
	bar.vertices[13] = right_y;
	bar.vertices[14] = right_top_x;
	bar.vertices[15] = right_top_y;
	bar.vertices[16] = right_x;
	bar.vertices[17] = right_y;
	bar.vertices[18] = right_bottom_x;
	bar.vertices[19] = right_bottom_y;
}

void bar_tick()
{
	if(bar.waiter >= bar.wait)
	{
		bar.next = CAT_rand_float(0, 1);
		bar.wait = CAT_rand_float(BAR_MIN_WAIT, BAR_MAX_WAIT);
		bar.waiter = 0;
	}
	bar.waiter += CAT_get_delta_time_s();

	float delta = bar.next - bar.target;
	if(fabs(delta) >= CAT_get_delta_time_s())
	{
		delta = sgn(delta);
		bar.target += delta * CAT_get_delta_time_s();
	}

	bar.target = clamp(bar.target, 0, 1);
	bar.cursor = clamp(bar.cursor, 0, 1);
	bar.progress = clamp(bar.progress, 0, 1);
}

CAT_ivec2 point_on_bar(float t)
{
	int y = bar.center.y;
	int x =
	bar.center.x - bar.length / 2 +
	bar.length * t;
	return (CAT_ivec2) {x, y};
}

void render_bar()
{
	CAT_polyberry
	(
		bar.center.x, bar.center.y,
		bar.vertices, 10,
		CAT_WHITE,
		CAT_POLY_MODE_LINES
	);

	CAT_ivec2 target = point_on_bar(bar.target);
	CAT_ivec2 guess = point_on_bar(bar.cursor);
	CAT_discberry(target.x, target.y, 4, CAT_RED);
	CAT_circberry(guess.x, guess.y, 6, CAT_WHITE);

	int left = bar.center.x - bar.length / 2 + 16;
	int right = left + (bar.length - 32) * bar.progress;
	CAT_lineberry(left, bar.center.y - 8, right, bar.center.y - 8, CAT_RED);
}

void MS_catch(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(render_MS_catch);
			init_bar((CAT_ivec2){120, 300}, 200);
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				bar.cursor += 0.25f * CAT_get_delta_time_s();
			}
			else if(CAT_input_held(CAT_BUTTON_A, 0))
			{
				float time = input.time[CAT_BUTTON_A];
				float rise = 0.75f + CAT_ease_in_cubic(time);
				bar.cursor += rise * CAT_get_delta_time_s();
			}
			else
			{
				float time = input.since[CAT_BUTTON_A];
				float fall = 0.25f + CAT_ease_in_cubic(time) * 3.0f;
				bar.cursor -= fall * CAT_get_delta_time_s();
			}
			float error = fabs(bar.cursor - bar.target);
			if(error < BAR_ERROR_MARGIN)
				bar.progress += CAT_get_delta_time_s() / 10.0f;
			else
				bar.progress -= CAT_get_delta_time_s() / 15.0f;

			if(bar.progress >= 1)	
				CAT_machine_transition(MS_succeed);
			else if(bar.progress <= 0)
				CAT_machine_transition(MS_fail);
			
			fish_tick();
			rings_tick();	
			bar_tick();
		}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void render_MS_catch()
{
	CAT_frameberry(CAT_BLACK);
	render_fish(CAT_RED);
	// render_rings();
	render_bar();
}

CAT_RGB888 fail_colour;

void MS_fail(CAT_machine_signal signal)
{
	static float progress;

	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(render_MS_fail);
			progress = 0;
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			fish_integrate_heading(fish.headings[0], EXIT_SPEED);
			fish_pbd_default();

			fail_colour = CAT_RGB888_lerp(CAT_RGB24(255,0,0), CAT_RGB24(0,0,0), progress);
			progress += CAT_get_delta_time_s();
			if(progress >= 1)
				CAT_machine_transition(CAT_MS_room);
		}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}
void render_MS_fail()
{
	CAT_frameberry(CAT_BLACK);
	render_fish(CAT_RGB8882565(fail_colour));
}

CAT_RGB888 succeed_colour;

void catch_burst()
{
	ring_count = 0;
	for(int i = 5; i >= 0; i--)
	{
		int ring_idx = spawn_ring(fish.positions[0], 400, 64);
		rings[ring_idx].radius += i * 12;
		rings[ring_idx].colour = CAT_RED;
	}
}

void MS_succeed(CAT_machine_signal signal)
{
	static float progress;

	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(render_MS_succeed);
			catch_burst();
			progress = 0;
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			rings_tick();

			succeed_colour = CAT_RGB888_lerp(CAT_RGB24(255,0,0), CAT_RGB24(255,255,255), clamp(progress / 3.0f, 0, 1));
			if(progress >= 3.0f)
				CAT_machine_transition(MS_summary);
			progress += CAT_get_delta_time_s();
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void render_MS_succeed()
{
	CAT_frameberry(CAT_BLACK);
	render_fish(CAT_RGB8882565(succeed_colour));
	render_rings();
}

void MS_summary(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(render_MS_summary);
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
		}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void render_MS_summary()
{
	CAT_frameberry(CAT_BLACK);

	int cursor_y = 12;
	CAT_textfberry(12, cursor_y, CAT_WHITE, 2, fish.type->name);
	cursor_y += 36;
	CAT_textfberry(12, cursor_y, CAT_WHITE, 1, fish.type->proverb);
	cursor_y += 52;
	CAT_textfberry(12, cursor_y, CAT_WHITE, 1, "Length: %0.0f cm", fish.length * 100);
	cursor_y += 20;
	CAT_lineberry(12, cursor_y, 12 + CAT_LCD_SCREEN_W * 0.75 * inv_lerp(fish.length, fish.type->min_length, fish.type->max_length), cursor_y, CAT_WHITE);
	cursor_y += 16;
	CAT_textfberry(12, cursor_y, CAT_WHITE, 1, "Lustre: %0.2f", fish.lustre);
	cursor_y += 20;
	CAT_lineberry(12, cursor_y, 12 + CAT_LCD_SCREEN_W * 0.75 * inv_lerp(fish.lustre, fish.type->min_lustre, fish.type->max_lustre), cursor_y, CAT_WHITE);
	cursor_y += 16;
	CAT_textfberry(12, cursor_y, CAT_WHITE, 1, "Wisdom: %0.2f", fish.wisdom);
	cursor_y += 20;
	CAT_lineberry(12, cursor_y, 12 + CAT_LCD_SCREEN_W * 0.75 * inv_lerp(fish.wisdom, fish.type->min_wisdom, fish.type->max_wisdom), cursor_y, CAT_WHITE);
}

void CAT_MS_study(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_machine_transition(MS_cast);
		}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}