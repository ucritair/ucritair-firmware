#include "cat_actions.h"

#include "cat_render.h"
#include "cat_input.h"
#include "cat_room.h"
#include <math.h>
#include "cowtools/cat_curves.h"
#include "sprite_assets.h"

#define SCREEN_DIAG 400
#define FADE_IN_DURATION 2.0f
#define FADE_OUT_DURATION 1.0f
#define BITE_MIN_WAIT 1.5f
#define BITE_MAX_WAIT 3.5f
#define VULN_TIMEOUT 0.35f

void MS_cast(CAT_machine_signal signal);
void render_MS_cast();
void MS_fish(CAT_machine_signal signal);
void render_MS_fish();
void MS_catch(CAT_machine_signal signal);
void render_MS_catch();

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

CAT_ivec2 point_on_bar(float t)
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

	CAT_ivec2 target = point_on_bar(pole.target);
	CAT_ivec2 guess = point_on_bar(pole.guess);
	CAT_discberry(target.x, target.y, pole.committed ? 3 : 4, CAT_RED);
	CAT_circberry(guess.x, guess.y, 6, CAT_WHITE);
	if(pole.committed)
		CAT_circberry(target.x, target.y, pole.error * pole.length, CAT_RED);
}

float cast_t = 0.0f;
float cast_dir = 1.0f;

bool fading_out = false;
float fade_out_t = 0.0f;

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
			fading_out = false;
			fade_out_t = 0;
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

					spawn_ring(CAT_iv2v(point_on_bar(pole.target)), SCREEN_DIAG, 320);
				}
				else if(!fading_out)
				{
					fading_out = true;
					customize_ring
					(
						spawn_ring
						(
							CAT_iv2v(point_on_bar(pole.target)),
							SCREEN_DIAG * 2, SCREEN_DIAG / FADE_OUT_DURATION
						),
						CAT_BLACK, true
					);
				}
			}

			if(!pole.committed)
			{
				cast_t += cast_dir * CAT_get_delta_time_s() / 3.0f;
				cast_t = clamp(cast_t, 0.2f, 0.8f);
				if(cast_t >= 0.79f)
					cast_dir = -1.0f;
				else if(cast_t <= 0.21f)
					cast_dir = 1.0f;
				pole.guess = CAT_ease_inout_elastic(cast_t);
			}
			else if(fading_out)
			{
				fade_out_t += CAT_get_delta_time_s();
				if(fade_out_t >= FADE_OUT_DURATION)
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
	CAT_vec2 target;

	bool bite_trigger;
	int bite_timer_id;

	bool vuln_trigger;
	int vuln_timer_id;
} fish =
{
	.bite_timer_id = -1,
	.vuln_timer_id = -1
};

void init_fish(CAT_vec2 lead_position, CAT_vec2 lead_heading, float lead_radius)
{
	fish.type = fish_types[CAT_rand_int(0, FISH_TYPE_COUNT-1)];
	fish.length = CAT_rand_float(fish.type->min_length, fish.type->max_length);
	fish.lustre = CAT_rand_float(fish.type->min_lustre, fish.type->max_lustre);
	fish.wisdom = CAT_rand_float(fish.type->min_wisdom, fish.type->max_wisdom);

	fish.positions[0] = lead_position;
	fish.headings[0] = lead_heading;
	fish.radii[0] = lead_radius;

	for(int i = 1; i < 3; i++)
	{
		fish.radii[i] = lerp(lead_radius, lead_radius * 0.25f, (float) i / (float) 2);
		CAT_vec2 offset = CAT_vec2_mul(lead_heading, -1 * (fish.radii[i-1] + lead_radius / 2 + fish.radii[i]));
		fish.positions[i] = CAT_vec2_add(fish.positions[i-1], offset);
		fish.headings[i] = lead_heading;	
	}

	fish.bite_trigger = false;
	CAT_timer_reinit(&fish.bite_timer_id, 3.0f);

	fish.vuln_trigger = false;
	CAT_timer_reinit(&fish.vuln_timer_id, VULN_TIMEOUT);
}

void fish_tick()
{
	CAT_vec2 axis = CAT_vec2_sub(hook, fish.positions[0]);

	fish.headings[0] = CAT_vec2_unit(axis);
	fish.positions[0] = CAT_vec2_add(fish.positions[0],  CAT_vec2_mul(fish.headings[0], 64 * CAT_get_delta_time_s()));

	for(int i = 1; i < 3; i++)
	{
		fish.headings[i] = CAT_vec2_unit(CAT_vec2_sub(fish.positions[i-1], fish.positions[i]));
		float interspace = fish.radii[i-1] + fish.radii[0] / 2 + fish.radii[i];
		fish.positions[i] = CAT_vec2_add(fish.positions[i-1], CAT_vec2_mul(fish.headings[i], -interspace));
	}

	if(!fish.bite_trigger)
	{
		if(CAT_vec2_mag2(axis) < fish.radii[0])
		{
			fish.bite_trigger = true;
			CAT_timer_reinit(&fish.bite_timer_id, CAT_rand_float(BITE_MIN_WAIT, BITE_MAX_WAIT));
		}
	}
	else
	{
		if(CAT_vec2_mag2(axis) >= fish.radii[0] + 4)
		{
			fish.bite_trigger = false;
		}
		else
		{
			if(CAT_timer_tick(fish.bite_timer_id))
			{
				if(CAT_rand_chance(3))
				{
					fish.vuln_trigger = true;
					CAT_timer_reinit(&fish.vuln_timer_id, VULN_TIMEOUT);

					for(int i = 0; i < 3; i++)
					{
						int ring_idx = spawn_ring(hook, 400, 64);
						rings[ring_idx].radius += i * 12;
					}
				}
				else
				{
					spawn_ring(hook, 400, 64);
				}
				CAT_timer_reinit(&fish.bite_timer_id, CAT_rand_float(BITE_MIN_WAIT, BITE_MAX_WAIT));
			}
		}
	}

	if(fish.vuln_trigger)
	{
		if(CAT_timer_tick(fish.vuln_timer_id))
		{
			fish.vuln_trigger = false;
		}
	}
}

void render_fish()
{
	for(int i = 0; i < 3; i++)
		CAT_discberry(fish.positions[i].x, fish.positions[i].y, fish.radii[i], CAT_RED);
	for(int i = 1; i < 3; i++)
		CAT_lineberry(fish.positions[i].x, fish.positions[i].y, fish.positions[i-1].x, fish.positions[i-1].y, CAT_RED);
	CAT_vec2 left = CAT_vec2_unit((CAT_vec2) {fish.headings[0].y, -fish.headings[0].x});
	CAT_vec2 right = CAT_vec2_mul(left, -1);
	CAT_vec2 anchor = CAT_vec2_add(fish.positions[0], CAT_vec2_mul(fish.headings[0], -fish.radii[0] * 1.5f));
	CAT_vec2 left_tip = CAT_vec2_add(anchor, CAT_vec2_mul(left, fish.radii[0] * 1.5f));
	CAT_vec2 right_tip = CAT_vec2_add(anchor, CAT_vec2_mul(right, fish.radii[0] * 1.5f));
	CAT_lineberry(fish.positions[0].x, fish.positions[0].y, left_tip.x, left_tip.y, CAT_RED);
	CAT_lineberry(fish.positions[0].x, fish.positions[0].y, right_tip.x, right_tip.y, CAT_RED);
}

void MS_fish(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(render_MS_fish);
			init_rings();
			init_fish((CAT_vec2) {120, 0}, (CAT_vec2) {0, 1.0f}, 16.0f);
			hook = (CAT_vec2) {120, 160};
		}
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);

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

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				if(fish.vuln_trigger)
				{
					CAT_machine_transition(MS_catch);
				}
			}
				
			hook.x = clamp(hook.x, 0, CAT_LCD_SCREEN_W-1);
			hook.y = clamp(hook.y, 0, CAT_LCD_SCREEN_H-1);

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

	render_fish();
	render_rings();

	CAT_push_draw_colour(CAT_WHITE);
	CAT_push_draw_flags(CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y);
	CAT_draw_sprite(&gizmo_target_17x17_sprite, 0, hook.x, hook.y);
}

void MS_catch(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(render_MS_catch);
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

void render_MS_catch()
{
	CAT_frameberry(CAT_BLACK);

	int cursor_y = 0;
	CAT_textfberry(12, (cursor_y += 12), CAT_WHITE, 2, fish.type->name);
	CAT_textfberry(12, (cursor_y += 36), CAT_WHITE, 1, fish.type->proverb);
	CAT_textfberry(12, (cursor_y += 52), CAT_WHITE, 1, "Length: %0.0f cm", fish.length * 100);
	CAT_lineberry(12, (cursor_y += 20), 12 + CAT_LCD_SCREEN_W * 0.75 * inv_lerp(fish.length, fish.type->min_length, fish.type->max_length), cursor_y, CAT_WHITE);
	CAT_textfberry(12, (cursor_y += 16), CAT_WHITE, 1, "Lustre: %0.2f", fish.lustre);
	CAT_lineberry(12, (cursor_y += 20), 12 + CAT_LCD_SCREEN_W * 0.75 * inv_lerp(fish.lustre, fish.type->min_lustre, fish.type->max_lustre), cursor_y, CAT_WHITE);
	CAT_textfberry(12, (cursor_y += 16), CAT_WHITE, 1, "Wisdom: %0.2f", fish.wisdom);
	CAT_lineberry(12, (cursor_y += 20), 12 + CAT_LCD_SCREEN_W * 0.75 * inv_lerp(fish.wisdom, fish.type->min_wisdom, fish.type->max_wisdom), cursor_y, CAT_WHITE);
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