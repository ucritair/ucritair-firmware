#include "cat_play.h"

#include "cat_render.h"
#include "cat_input.h"
#include "cat_room.h"
#include <math.h>
#include "cowtools/cat_curves.h"
#include "sprite_assets.h"
#include "cowtools/cat_structures.h"
#include "fish_assets.h"
#include "cat_gui.h"
#include "item_assets.h"
#include "cat_pet.h"
#include "cat_item.h"
#include "cat_gizmos.h"

#define SCREEN_DIAG 400
#define BLACK_OUT_DURATION 1.0f
#define NIBBLE_WAIT_MIN 1.5f
#define NIBBLE_WAIT_MAX 3.5f
#define VULN_FRAMES 10
#define BITE_CHANCE 5
#define UNFOCUSED_SPEED 48
#define FOCUSED_SPEED 64
#define ELLIPSE_MARGIN 12
#define EXIT_SPEED 128
#define ARENA_RADIUS 48
#define ARENA_FADE_DURATION 1.0f

// Lower lull, higher difficulty
static const float bar_lull_ranges[] =
{
	1, 2,
	1, 3,
	1, 4
};

// Lower margins, higher difficulty
static const float bar_margin_ranges[] =
{
	0.1, 0.125,
	0.075, 0.09,
	0.05, 0.075
};

// Larger jumps, higher difficulty
static const float bar_jump_ranges[] =
{
	0.1, 0.25,
	0.2, 0.5,
	0.25, 0.75
};

// Higher difficulty, higher ranges
static const float stat_ranges[] =
{
	0.1, 0.75,
	0.35, 0.85,
	0.5, 1.0
};

static const int item_rewards[] =
{
	fish_grade_0_item,
	fish_grade_1_item,
	fish_grade_2_item
};

static const int stat_rewards[] =
{
	2,
	3,
	4
};

static const int xp_rewards[] =
{
	2, 7,
	7, 15,
	20, 30 
};

static void MS_cast(CAT_FSM_signal signal);
static void render_MS_cast();
static void MS_fish(CAT_FSM_signal signal);
static void render_MS_fish();
static void MS_catch(CAT_FSM_signal signal);
static void render_MS_catch();
static void MS_fail(CAT_FSM_signal signal);
static void render_MS_fail();
static void MS_succeed(CAT_FSM_signal signal);
static void render_MS_succeed();
static void MS_summary(CAT_FSM_signal signal);
static void render_MS_summary();
static CAT_FSM fsm =
{
	.state = NULL,
	.next = NULL,
	.dirty = false
};

typedef struct
{
	CAT_vec2 center;
	float max_radius;
	float speed;

	uint16_t colour;
	bool fill;

	float radius;
} study_ring;
static study_ring rings[16];
static int ring_count = 0;

static void init_rings()
{
	ring_count = 0;
}

static int spawn_ring(CAT_vec2 center, float max_radius, float speed)
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

static void customize_ring(int idx, uint16_t colour, bool fill)
{
	if(idx < 0 || idx >= ring_count)
		return;
	study_ring* ring = &rings[idx];
	ring->colour = colour;
	ring->fill = fill;
}

static void swap_rings(int i, int j)
{
	study_ring temp = rings[i];
	rings[i] = rings[j];
	rings[j] = temp;
}

static void rings_tick()
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

static void render_rings()
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

static struct
{
	CAT_ivec2 center;
	int length;

	float target;
	float guess;
	bool committed;
	float error;

	int16_t vertices[10*2];
} pole;

static void init_pole(CAT_ivec2 center, int length)
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

static CAT_ivec2 point_on_pole(float t)
{
	int x = pole.center.x;
	int y =
	pole.center.y + pole.length / 2 -
	pole.length * t;
	return (CAT_ivec2) {x, y};
}

static void render_pole()
{
	CAT_draw_polyline
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

static float cast_t = 0.0f;
static float cast_dir = 1.0f;

static int cast_grade;

static bool blacking_out = false;
static float black_out_t = 0.0f;

static void MS_cast(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(render_MS_cast);
			init_rings();
			init_pole((CAT_ivec2){CAT_LCD_SCREEN_W/2, CAT_LCD_SCREEN_H/2}, CAT_LCD_SCREEN_H * 0.75f);

			cast_t = 0;
			cast_dir = 0;
			cast_grade = 0;

			blacking_out = false;
			black_out_t = 0;
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_FSM_transition(&fsm, NULL);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				if(!pole.committed)
				{
					pole.committed = true;
					pole.error = fabs(pole.target - pole.guess);
					if(pole.error < 0.05)
						cast_grade = 2;
					else if(pole.error < 0.15)
						cast_grade = 1;

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
				cast_t += cast_dir * CAT_get_delta_time_s() / 1.5f;
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
					CAT_FSM_transition(&fsm, MS_fish);
			}

			rings_tick();
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

static void render_MS_cast()
{
	CAT_frameberry(CAT_BLACK);
	render_pole();
	render_rings();

	if(!pole.committed)
	{
		CAT_set_text_scale(2);
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_text(12, 12, "CAST");
		CAT_set_text_scale(2);
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_text(12, 12+28, "A");
		CAT_set_text_scale(2);
		CAT_set_text_colour(CAT_WHITE);
		CAT_draw_text(12, 12+28+28, "LINE");
	}
	else if(!blacking_out)
	{
		CAT_set_text_scale(2);
		CAT_set_text_colour(CAT_RED);
		CAT_draw_text(12, 12, "GO");
		CAT_set_text_scale(2);
		CAT_set_text_colour(CAT_RED);
		CAT_draw_text(12, 12+28, "FISH!");
	}
}

static CAT_vec2 hook;

static int fish_pool_backing[FISH_COUNT*3];
static CAT_int_list fish_pool;

static struct
{
	const CAT_fish* type;
	int grade;
	float length;
	float lustre;
	float wisdom;

	CAT_vec2 positions[3];
	CAT_vec2 headings[3];
	float radii[3];

	bool focus_trigger;
	CAT_vec2 focus_heading;

	bool nibble_trigger;
	float nibble_timer;
	float nibble_time;

	bool vuln_trigger;
	int vuln_frame_counter;
	bool bite_trigger;
	float bite_timer;

	bool race_trigger;
} fish;

static int item_reward;
static int focus_reward;
static int xp_reward;

static void init_fish(CAT_vec2 lead_position, CAT_vec2 lead_heading, float lead_radius)
{
	CAT_ilist(&fish_pool, fish_pool_backing, FISH_COUNT*3);
	for(int i = 0; i < FISH_COUNT; i++)
	{
		const CAT_fish* fish_type = fish_list[i];
		if(fish_type->grade_constraint <= cast_grade)
		{
			for(int j = 0; j <= cast_grade; j++)
				CAT_ilist_push(&fish_pool, i);
		}
	}
	CAT_ilist_shuffle(&fish_pool);
	int choice = fish_pool.data[CAT_rand_int(0, fish_pool.length-1)];
	fish.type = fish_list[choice];
	fish.grade = CAT_rand_int(fish.type->grade_constraint, cast_grade);
	if(cast_grade == 2)
	{
		if(CAT_rand_chance(2))
			fish.grade = 2;
	}

	float stat_min = stat_ranges[fish.grade*2+0];
	float stat_max = stat_ranges[fish.grade*2+1];
	fish.length = lerp(fish.type->min_length, fish.type->max_length, CAT_rand_float(stat_min, stat_max));
	fish.lustre = lerp(fish.type->min_lustre, fish.type->max_lustre, CAT_rand_float(stat_min, stat_max));
	fish.wisdom = lerp(fish.type->min_wisdom, fish.type->max_wisdom, CAT_rand_float(stat_min, stat_max));

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

	fish.focus_trigger = false;

	fish.nibble_trigger = false;
	fish.nibble_timer = 0;
	
	fish.vuln_trigger = false;
	fish.vuln_frame_counter = 0;
	fish.bite_trigger = false;
	fish.bite_timer = 0;

	fish.race_trigger = false;
}

static void fish_pbd_default()
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

static void fish_pbd_waggle()
{
	for(int i = 1; i < 3; i++)
	{
		CAT_vec2 heading = fish.headings[i-1];
		float amplitude = 0.5f * sinf(CAT_get_uptime_ms() / 1000.0f + (2-i));
		heading = CAT_vec2_rotate(heading, amplitude);
		float interspace = fish.radii[i-1] + fish.radii[0] / 2 + fish.radii[i];
		CAT_vec2 position = CAT_vec2_add(fish.positions[i-1], CAT_vec2_mul(heading, -interspace));
		fish.headings[i] = heading;
		fish.positions[i] = position;
	}
}

static void fish_integrate_heading(CAT_vec2 heading, float speed)
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

static CAT_vec2 fish_ellipse_position(float t)
{
	t = t * 2 * M_PI;
	return (CAT_vec2)
	{
		(120 - ELLIPSE_MARGIN) * cosf(t) + 120,
		(160 - ELLIPSE_MARGIN) * sinf(t) + 160
	};
}

static CAT_vec2 fish_ellipse_heading(float t)
{
	t = t * 2 * M_PI;
	return CAT_vec2_unit((CAT_vec2) 
	{
		-(120 - ELLIPSE_MARGIN) * sinf(t),
		(160 - ELLIPSE_MARGIN) * cosf(t)
	});
}

static void fish_integrate_ellipse(float t)
{
	fish.headings[0] = fish_ellipse_heading(t);
	fish.positions[0] = fish_ellipse_position(t);
}

static int bite_prob_backing[BITE_CHANCE];
static CAT_int_list bite_prob = 
{
	.capacity = BITE_CHANCE,
	.data = bite_prob_backing,
	.length = 0
};

static void init_bite_probability()
{
	CAT_ilist(&bite_prob, bite_prob_backing, BITE_CHANCE);
	for(int i = 0; i < 5; i++)
		CAT_ilist_push(&bite_prob, i == 0);
	CAT_ilist_shuffle(&bite_prob);
}

static bool draw_bite_probability()
{
	if(bite_prob.length == 0)
		init_bite_probability();
	bool value = CAT_ilist_pop(&bite_prob);
	if(value)
		init_bite_probability();
	return value;
}

static void nibble_burst(int n)
{
	for(int i = n-1; i >= 0; i--)
	{
		int ring_idx = spawn_ring(hook, 400, 64);
		rings[ring_idx].radius += i * 12;
	}
}

static void fish_tick()
{
	CAT_vec2 hook_beeline = CAT_vec2_sub(hook, fish.positions[0]);
	float hook_distance = sqrtf(CAT_vec2_mag2(hook_beeline));
	CAT_vec2 hook_heading = CAT_vec2_div(hook_beeline, hook_distance);
	float hook_align = CAT_vec2_dot(fish.headings[0], hook_heading);

	if(fish.bite_trigger)
	{
		if(fish.vuln_trigger)
		{	
			if(fish.vuln_frame_counter > VULN_FRAMES)
				fish.vuln_trigger = false;
			fish.vuln_frame_counter += 1;
		}

		fish.bite_timer += CAT_get_delta_time_s();
	}
	else if(fish.nibble_trigger)
	{
		if(fish.nibble_timer >= fish.nibble_time)
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

			fish.nibble_timer = 0;
			fish.nibble_time = CAT_rand_float(NIBBLE_WAIT_MIN, NIBBLE_WAIT_MAX);
		}
		fish.nibble_timer += CAT_get_delta_time_s();
	}
	else if(fish.focus_trigger)
	{
		float hook_arena_dist = sqrtf(CAT_vec2_dist2((CAT_vec2){120, 160}, hook));
		if(hook_arena_dist < ARENA_RADIUS && hook_align > 0.7 && hook_distance <= (fish.radii[0] + 8))
		{
			fish.nibble_trigger = true;
			fish.nibble_timer = 0;
			fish.nibble_time = CAT_rand_float(NIBBLE_WAIT_MIN, NIBBLE_WAIT_MAX);
		}

		float fish_arena_dist = sqrtf(CAT_vec2_dist2((CAT_vec2){120, 160}, fish.positions[0]));
		if(fish_arena_dist < (ARENA_RADIUS + fish.radii[0]))
			fish.race_trigger = true;
		if(fish.race_trigger && fish_arena_dist > (ARENA_RADIUS + fish.radii[0]))
			CAT_FSM_transition(&fsm, MS_fail);
		fish_integrate_heading(fish.focus_heading, FOCUSED_SPEED);
	}
	else
	{
		if(hook_align > 0.7 && hook_distance < fish.radii[0] * 3)
		{
			fish.focus_trigger = true;
			fish.focus_heading = CAT_vec2_unit(CAT_vec2_sub((CAT_vec2) {120, 160}, fish.positions[0]));
		}
		fish_integrate_ellipse(CAT_get_uptime_ms() / 20000.0f);
	}

	if(fish.nibble_trigger)
		fish_pbd_waggle();
	else
		fish_pbd_default();
}

static void render_fish(uint16_t colour)
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

static float arena_fade_timer;
static CAT_ivec2 hook_jitter;

static bool quit_popup()
{
	if (CAT_gui_popup_is_open())
		return true;

	if (CAT_input_pressed(CAT_BUTTON_B))
		CAT_gui_open_popup("Quit fishing?\nYou will lose this\ncatch!\n");
	if (CAT_gui_consume_popup())
	{
		CAT_pushdown_pop();
		return true;
	}

	return false;
}

static void MS_fish(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(render_MS_fish);
			init_rings();

			init_fish((CAT_vec2) {120, 60}, (CAT_vec2) {CAT_rand_float(-1, 1), CAT_rand_float(-1, 1)}, 16.0f);
			item_reward = item_rewards[fish.grade];
			focus_reward = stat_rewards[fish.grade];
			xp_reward = CAT_rand_int(xp_rewards[fish.grade*2+0], xp_rewards[fish.grade*2+1]);

			hook = (CAT_vec2) {120, 160};
			init_bite_probability();

			arena_fade_timer = 0.0f;
			hook_jitter = (CAT_ivec2) {0, 0};
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(quit_popup())
				break;

			if(!fish.nibble_trigger)
			{
				if(CAT_input_touching())
				{
					int x, y;
					CAT_input_cursor(&x, &y);
					hook = (CAT_vec2) {x, y};
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
						CAT_FSM_transition(&fsm, MS_catch);
					else
						CAT_FSM_transition(&fsm, MS_fail);
				}

				if(fish.bite_trigger && fish.bite_timer >= 1.0f)
					CAT_FSM_transition(&fsm, MS_fail);
			}

			if(fish.focus_trigger)
			{
				if(fish.nibble_trigger)
					arena_fade_timer -= CAT_get_delta_time_s();
				else
					arena_fade_timer += CAT_get_delta_time_s();
				arena_fade_timer = clamp(arena_fade_timer, 0, ARENA_FADE_DURATION);
			}

			hook_jitter = (CAT_ivec2)
			{
				CAT_rand_int(-1, 1),
				CAT_rand_int(-1, 1)
			};

			fish_tick();
			rings_tick();
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

static void render_MS_fish()
{
	CAT_frameberry(CAT_BLACK);

	if(fish.focus_trigger)
		CAT_circberry(120, 160, ARENA_RADIUS, CAT_colour_lerp(CAT_BLACK, 0x6800, arena_fade_timer/ARENA_FADE_DURATION));
		
	render_fish(CAT_RED);
	render_rings();

	if(fish.nibble_trigger)
	{
		if(fish.bite_trigger)
		{
			CAT_set_text_colour(CAT_RED);
			CAT_set_text_scale(2);
			CAT_draw_text(12, 12, "GO!");
		}
		else
		{
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_scale(2);
			CAT_draw_text(12, 12, "WAIT...");
		}
	}
	else
	{
		if(CAT_pulse(
			fish.race_trigger ? 0.125f :
			fish.focus_trigger ? 0.25f :
			0.5f
		))
		{
			CAT_set_text_scale(2);
			CAT_set_text_colour(fish.race_trigger ? CAT_RED : CAT_WHITE);
			CAT_draw_text(12, 12, "INTERCEPT!");
		}
	}

	if(!fish.nibble_trigger)
	{
		CAT_set_sprite_colour(CAT_WHITE);
		CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
		CAT_draw_sprite(&gizmo_target_17x17_sprite, 0, hook.x, hook.y);

		CAT_animator_reset(&fishing_bobber_set_sprite);
	}
	else if(!fish.bite_trigger)
	{
		CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
		if(CAT_animator_get_frame(&fishing_bobber_set_sprite) < fishing_bobber_set_sprite.frame_count-1)
			CAT_draw_sprite(&fishing_bobber_set_sprite, -1, hook.x, hook.y);
		else
		{
			//CAT_vec2 bobber = CAT_vec2_add(hook, CAT_iv2v(hook_jitter));
			CAT_draw_sprite(&fishing_bobber_idle_sprite, -1, hook.x, hook.y);
		}

		CAT_animator_reset(&fishing_bobber_bite_sprite);
	}
	else
	{
		CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
		CAT_vec2 bobber = hook;
		CAT_draw_sprite(&fishing_bobber_bite_sprite, -1, bobber.x, bobber.y);
	}
}

static struct
{
	float margin;
	float target;
	
	float target_next;
	float margin_next;

	float wait;
	float waiter;
	float slider;

	float cursor;
	float progress;

	CAT_ivec2 center;
	int length;
	int16_t vertices[10*2];
} bar;

static void bar_retarget(bool hard)
{
	float min_margin = bar_margin_ranges[fish.grade*2+0];
	float max_margin = bar_margin_ranges[fish.grade*2+1];
	float min_jump = bar_jump_ranges[fish.grade*2+0];
	float max_jump = bar_jump_ranges[fish.grade*2+1];
	float min_lull = bar_lull_ranges[fish.grade*2+0];
	float max_lull = bar_lull_ranges[fish.grade*2+1];
	
	if(hard)
	{
		bar.margin = CAT_rand_float(min_margin, max_margin);
		bar.target = CAT_rand_float(bar.margin, 1-bar.margin);
	}

	bar.margin_next = CAT_rand_float(min_margin, max_margin);
	float ideal_jump = CAT_rand_float(min_jump, max_jump);
	ideal_jump *= CAT_rand_int(0, 1) ? 1 : -1;
	float candidate = clamp(bar.target + ideal_jump, bar.margin_next, 1-bar.margin_next);
	if(fabs(candidate - bar.target) < min_jump)
		bar.target_next = clamp(bar.target - ideal_jump, bar.margin_next, 1-bar.margin_next);
	else
		bar.target_next = candidate;

	bar.wait = CAT_rand_float(min_lull, max_lull);
	bar.waiter = 0;
	bar.slider = 0;
}

static void init_bar(CAT_ivec2 center, int length)
{
	bar_retarget(true);

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

static void bar_tick()
{
	if(bar.waiter >= bar.wait)
	{
		bar_retarget(false);
	}
	bar.waiter += CAT_get_delta_time_s();

	if(bar.slider < 1)
	{
		bar.target = lerp(bar.target, bar.target_next, bar.slider);
		bar.margin = lerp(bar.margin, bar.margin_next, bar.slider);
		bar.slider += CAT_get_delta_time_s();
	}

	bar.target = clamp(bar.target, 0, 1);
	bar.cursor = clamp(bar.cursor, 0, 1);
	bar.progress = clamp(bar.progress, 0, 1);
}

static CAT_ivec2 point_on_bar(float t)
{
	int y = bar.center.y;
	int x =
	bar.center.x - bar.length / 2 +
	bar.length * t;
	return (CAT_ivec2) {x, y};
}

static CAT_ivec2 bar_jitter;

static void render_bar()
{
	CAT_draw_polyline
	(
		bar.center.x, bar.center.y,
		bar.vertices, 10,
		CAT_WHITE,
		CAT_POLY_MODE_LINES
	);

	CAT_ivec2 target = point_on_bar(bar.target);
	target = CAT_ivec2_add(target, bar_jitter);

	int left = target.x - bar.length * bar.margin;
	int right = target.x + bar.length * bar.margin;
	CAT_lineberry(left, bar.center.y-4, left, bar.center.y+4, CAT_RED);
	CAT_lineberry(right, bar.center.y-4, right, bar.center.y+4, CAT_RED);
	CAT_lineberry(left, bar.center.y, right, bar.center.y, CAT_RED);

	CAT_ivec2 guess = point_on_bar(bar.cursor);
	float error = fabs(bar.cursor - bar.target);
	CAT_circberry(guess.x, guess.y, 6, (error < bar.margin) ? CAT_RED : CAT_WHITE);

	left = bar.center.x - ((bar.length - 16) * bar.progress) / 2;
	right = bar.center.x + ((bar.length - 16) * bar.progress) / 2;
	CAT_lineberry(left, bar.center.y - 8, right, bar.center.y - 8, CAT_RED);
}

static float struggle_timer;
static float struggle_wait;

static void MS_catch(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(render_MS_catch);
			init_bar((CAT_ivec2){120, 300}, 200);

			init_rings();

			struggle_timer = 0;
			struggle_wait = CAT_rand_float(1.0f, 3.0f);
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(quit_popup())
				break;

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				bar.cursor += 0.25f * CAT_get_delta_time_s();
			}
			else if(CAT_input_held(CAT_BUTTON_A, 0))
			{
				float time = CAT_input_since(CAT_BUTTON_A);
				float rise = 0.75f + CAT_ease_in_cubic(time);
				bar.cursor += rise * CAT_get_delta_time_s();
			}
			else
			{
				float time = CAT_input_since(CAT_BUTTON_A);
				float fall = 0.25f + CAT_ease_in_cubic(time) * 3.0f;
				bar.cursor -= fall * CAT_get_delta_time_s();
			}
			float error = fabs(bar.cursor - bar.target);
			if(error <= bar.margin)
			{
				bar.progress += CAT_get_delta_time_s() / 6.5f;
			}
			else
			{
				if(bar.progress < 0.15f)
					bar.progress -= CAT_get_delta_time_s() / 10.0f;
				else
					bar.progress -= CAT_get_delta_time_s() / 7.5f;
			}	

			if(bar.progress >= 1)	
				CAT_FSM_transition(&fsm, MS_succeed);
			else if(bar.progress <= 0)
				CAT_FSM_transition(&fsm, MS_fail);

			bar_jitter = (CAT_ivec2)
			{
				CAT_rand_int(-1, 1),
				CAT_rand_int(-1, 1)
			};

			if(struggle_timer >= struggle_wait)
			{
				spawn_ring(hook, CAT_rand_float(48, 72), 64);
				struggle_timer = 0;
				struggle_wait = CAT_rand_float(1.0f, 3.0f);
			}
			struggle_timer += CAT_get_delta_time_s();
			
			fish_tick();
			rings_tick();	
			bar_tick();
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

static void render_MS_catch()
{
	CAT_frameberry(CAT_BLACK);
	render_fish(CAT_RED);
	render_rings();
	render_bar();

	if(!CAT_input_held(CAT_BUTTON_A, 0))
	{
		CAT_set_sprite_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
		CAT_set_sprite_colour(CAT_RED);
		CAT_draw_sprite(&study_a_button_sprite, 0, 120, bar.center.y - 48);
	}
	CAT_set_text_colour(CAT_RED);

	CAT_draw_textf(120-8*2+4, bar.center.y - 26, "%d%%", (int)(bar.progress * 100));
}

static uint16_t fail_colour;

static void MS_fail(CAT_FSM_signal signal)
{
	static float progress;

	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(render_MS_fail);
			init_rings();
			progress = 0;
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			fish_integrate_heading(fish.headings[0], EXIT_SPEED);
			fish_pbd_default();

			fail_colour = CAT_colour_lerp(CAT_RED, CAT_BLACK, progress);
			progress += CAT_get_delta_time_s();
			if(progress >= 1)
				CAT_FSM_transition(&fsm, NULL);
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

static void render_MS_fail()
{
	CAT_frameberry(CAT_BLACK);
	render_fish(fail_colour);

	if(fish.bite_trigger)
	{
		CAT_set_text_colour(CAT_RED);
		CAT_set_text_scale(2);
		CAT_draw_text(12, 12, "TOO LATE...");
	}
	else if(fish.nibble_trigger)
	{
		CAT_set_text_colour(CAT_RED);
		CAT_set_text_scale(2);
		CAT_draw_text(12, 12, "TOO EARLY...");
	}
	else
	{
		CAT_set_text_colour(CAT_RED);
		CAT_set_text_scale(2);
		CAT_draw_text(12, 12, "GOT AWAY...");
	}
}

static uint16_t succeed_colour;

static void MS_succeed(CAT_FSM_signal signal)
{
	static float progress;

	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(render_MS_succeed);
			init_rings();
			int ring_idx = spawn_ring(fish.positions[0], 400, 64);
			rings[ring_idx].colour = CAT_RED;
			progress = 0;
		break;

		case CAT_FSM_SIGNAL_TICK:
			rings_tick();

			succeed_colour = CAT_colour_lerp(CAT_RED, CAT_WHITE, clamp(progress / 3.0f, 0, 1));
			if(progress >= 3.0f)
				CAT_FSM_transition(&fsm, MS_summary);
			progress += CAT_get_delta_time_s();
		break;

		case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

static void render_MS_succeed()
{
	CAT_frameberry(CAT_BLACK);
	render_fish(succeed_colour);
	render_rings();
}

static enum {
	FISH,
	PERFORMANCE,
	SUMMARY_PAGE_MAX
};
int summary_page = FISH;

static int8_t wave_buffer[48];
static int wave_phase = 0;

static void init_wave_buffer()
{
	for(int x = 0; x < 48; x++)
	{
		wave_buffer[x] = roundf(
		20.0f / 3.0f * sinf(M_PI * x / 20.0f + 2.2f) +
		sinf(2.0f * M_PI * x / 20.0f + 2.2f));
	}

	wave_phase = 0;
}

static float exit_progress = 0;

static void MS_summary(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_set_render_callback(render_MS_summary);
			summary_page = FISH;
			init_wave_buffer();
			exit_progress = 0;
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_input_down(CAT_BUTTON_A))
			{
				if(summary_page == PERFORMANCE)
					exit_progress += CAT_get_delta_time_s();
			}
			else if(CAT_input_released(CAT_BUTTON_A))
			{
				if(exit_progress >= 1)
					CAT_FSM_transition(&fsm, NULL);
				else
					exit_progress = 0;
			}
			exit_progress = clamp(exit_progress, 0, 1);

			int page_delta = 0;
			if (CAT_input_pressed(CAT_BUTTON_RIGHT))
				page_delta = 1;
			if (CAT_input_pressed(CAT_BUTTON_LEFT))
				page_delta = -1;
			if(page_delta != 0)
				exit_progress = 0;
			summary_page = (summary_page+page_delta+SUMMARY_PAGE_MAX) % SUMMARY_PAGE_MAX;

			wave_phase = (wave_phase + 1) % 240;
		}
		break;

		case CAT_FSM_SIGNAL_EXIT:
			CAT_inventory_add(item_rewards[fish.grade], 1);
			CAT_pet_change_focus(focus_reward);
			CAT_pet_change_XP(xp_reward);
		break;
	}
}

static void render_wave_buffer()
{
	uint16_t* framebuffer = CAT_LCD_get_framebuffer();
	uint16_t c = CAT_ADAPT_DESKTOP_COLOUR(0x4cb5);

	for(int x = 0; x < 240; x++)
	{
		int i = (x - wave_phase + 240) % 240;

		int y_f = 320 - (wave_buffer[i % 48] + 100);
		y_f -= CAT_LCD_FRAMEBUFFER_OFFSET;
		if(y_f < 0)
			y_f = 0;
		if(y_f > CAT_LCD_FRAMEBUFFER_H)
			continue;

		for(int y_w = y_f; y_w < CAT_LCD_FRAMEBUFFER_H; y_w++)
			framebuffer[y_w * CAT_LCD_FRAMEBUFFER_W + x] = c;
	}
}

static void render_score_line(int x, int y, int w, float t, float a, float b)
{
	t = inv_lerp(t, a, b);
	CAT_draw_score_line(x, y, w, t, 0, CAT_WHITE, CAT_RED, CAT_BLACK);
}

static void render_plus_line(int x, int y, int w, float min, float base, float plus, float max)
{
	float t = inv_lerp(base, min, max);
	float dt = inv_lerp(plus-base, min, max);
	CAT_draw_score_line(x, y, w, 0.5, 0.25, CAT_64_GREY, CAT_WHITE, CAT_160_GREY);
}

static void render_MS_summary()
{
	CAT_frameberry(CAT_BLACK);

	switch (summary_page)
	{
		case FISH:
		{		
			render_wave_buffer();
			CAT_draw_mesh2d(fish.type->mesh, 0, 208, CAT_WHITE);
			
			int cursor_y = 4;
			CAT_draw_page_markers(cursor_y, 2, summary_page, CAT_WHITE);
			cursor_y += 24;
			
			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_scale(2);
			cursor_y = CAT_draw_textf(12, cursor_y, "%s\n", fish.type->name);
			cursor_y += 8;

			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
			CAT_set_text_mask(12, -1, CAT_LCD_SCREEN_W-12, -1);
			cursor_y = CAT_draw_textf(12, cursor_y, "%s\n", fish.type->proverb);
			cursor_y += 8;

			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(12, cursor_y, "Length: %d cm", (int)(fish.length * 100));
			cursor_y += 16;
			render_score_line(12, cursor_y, CAT_LCD_SCREEN_W * 0.75, fish.length, fish.type->min_length, fish.type->max_length);
			cursor_y += 12;

			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(12, cursor_y, "Lustre: " CAT_FLOAT_FMT, CAT_FMT_FLOAT(fish.lustre));
			cursor_y += 16;
			render_score_line(12, cursor_y, CAT_LCD_SCREEN_W * 0.75, fish.lustre, 0, 1);
			cursor_y += 12;

			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(12, cursor_y, "Wisdom: " CAT_FLOAT_FMT, CAT_FMT_FLOAT(fish.wisdom));
			cursor_y += 16;
			render_score_line(12, cursor_y, CAT_LCD_SCREEN_W * 0.75, fish.wisdom, 0, 1);
		}
		break;

		case PERFORMANCE:
		{
			int cursor_y = 4;
			CAT_draw_page_markers(cursor_y, 2, summary_page, CAT_WHITE);
			cursor_y += 24;

			CAT_set_text_colour(CAT_WHITE);
			CAT_set_text_scale(2);
			CAT_draw_text(12, cursor_y, "Performance");
			cursor_y += 36;

			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(12, cursor_y, "Casting skill:");
			cursor_y += 20;
			render_score_line(12, cursor_y, CAT_LCD_SCREEN_W * 0.75, cast_grade+1, 0, 3);
			cursor_y += 16;

			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(12, cursor_y, "Fish quality:");
			cursor_y += 20;
			render_score_line(12, cursor_y, CAT_LCD_SCREEN_W * 0.75, fish.grade+1, 0, 3);
			cursor_y += 32;

			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(12, cursor_y, "+ Focus: %d", focus_reward);
			cursor_y += 20;
			render_plus_line(12, cursor_y, CAT_LCD_SCREEN_W * 0.75, 0, pet.focus, focus_reward, 12);
			cursor_y += 16;

			CAT_set_text_colour(CAT_WHITE);
			CAT_draw_textf(12, cursor_y, "+ XP: %d", xp_reward);
			cursor_y += 20;
			render_plus_line(12, cursor_y, CAT_LCD_SCREEN_W * 0.75, 0, pet.xp, xp_reward, level_cutoffs[pet.level]);
			cursor_y += 16;

			CAT_draw_lock(120, cursor_y+40, 36, exit_progress, CAT_WHITE);
		}
		break;

		default:
		break;
	}
}

void CAT_MS_study(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
			CAT_FSM_transition(&fsm, MS_cast);
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