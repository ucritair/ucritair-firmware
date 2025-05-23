#include "cat_actions.h"

#include "cat_render.h"
#include "cat_input.h"
#include "cat_room.h"
#include <math.h>
#include "cowtools/cat_curves.h"
#include "sprite_assets.h"

#define SCREEN_DIAG 400
#define FADE_DURATION 1.0f

void MS_cast(CAT_machine_signal signal);
void render_MS_cast();
void MS_fish(CAT_machine_signal signal);
void render_MS_fish();

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

CAT_vec2 cursor;

struct
{
	CAT_vec2 positions[3];
	CAT_vec2 headings[3];
	float radii[3];
	CAT_vec2 target;
} fish;

int fish_turn_timer_id = -1;
bool fish_bite_trigger = true;

void init_fish(CAT_vec2 lead_position, CAT_vec2 lead_heading, float lead_radius)
{
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

	CAT_timer_reinit(&fish_turn_timer_id, 3.0f);
}

void fish_tick()
{
	CAT_vec2 axis = CAT_vec2_sub(cursor, fish.positions[0]);
	if(CAT_vec2_mag2(axis) < fish.radii[0])
	{
		if(fish_bite_trigger)
		{
			spawn_ring(cursor, SCREEN_DIAG, 64);
			fish_bite_trigger = false;
		}
		return;
	}
	else
	{
		fish_bite_trigger = true;
	}
		
	fish.headings[0] = CAT_vec2_unit(axis);
	fish.positions[0] = CAT_vec2_add(fish.positions[0],  CAT_vec2_mul(fish.headings[0], 64 * CAT_get_delta_time_s()));
		
	for(int i = 1; i < 3; i++)
	{
		fish.headings[i] = CAT_vec2_unit(CAT_vec2_sub(fish.positions[i-1], fish.positions[i]));
		float interspace = fish.radii[i-1] + fish.radii[0] / 2 + fish.radii[i];
		fish.positions[i] = CAT_vec2_add(fish.positions[i-1], CAT_vec2_mul(fish.headings[i], -interspace));
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

struct
{
	CAT_ivec2 center;
	int length;

	float target;
	float guess;
	bool committed;
	float error;

	int vertices[10*2];
} casting_bar;

void init_casting_bar(CAT_ivec2 center, int length)
{
	casting_bar.center = center;
	casting_bar.length = length;

	casting_bar.target = CAT_rand_float(0.2, 0.8);
	casting_bar.guess = 0;
	casting_bar.committed = false;
	casting_bar.error = 0;

	int top_x = 0; int top_y = -length/2;
	int bottom_x = 0; int bottom_y = length/2;
	int top_left_x = top_x - 8; int top_left_y = top_y + 8;
	int top_right_x = top_x + 8; int top_right_y = top_y + 8;
	int bottom_left_x = bottom_x - 8; int bottom_left_y = bottom_y - 8;
	int bottom_right_x = bottom_x + 8; int bottom_right_y = bottom_y - 8;
	casting_bar.vertices[0] = top_x;
	casting_bar.vertices[1] = top_y;
	casting_bar.vertices[2] = top_left_x;
	casting_bar.vertices[3] = top_left_y;
	casting_bar.vertices[4] = top_x;
	casting_bar.vertices[5] = top_y;
	casting_bar.vertices[6] = top_right_x;
	casting_bar.vertices[7] = top_right_y;
	casting_bar.vertices[8] = top_x;
	casting_bar.vertices[9] = top_y;
	casting_bar.vertices[10] = bottom_x;
	casting_bar.vertices[11] = bottom_y;
	casting_bar.vertices[12] = bottom_x;
	casting_bar.vertices[13] = bottom_y;
	casting_bar.vertices[14] = bottom_left_x;
	casting_bar.vertices[15] = bottom_left_y;
	casting_bar.vertices[16] = bottom_x;
	casting_bar.vertices[17] = bottom_y;
	casting_bar.vertices[18] = bottom_right_x;
	casting_bar.vertices[19] = bottom_right_y;
}

CAT_ivec2 point_on_bar(float t)
{
	int x = casting_bar.center.x;
	int y =
	casting_bar.center.y + casting_bar.length / 2 -
	casting_bar.length * t;
	return (CAT_ivec2) {x, y};
}

void render_casting_bar()
{
	CAT_polyberry
	(
		casting_bar.center.x, casting_bar.center.y,
		casting_bar.vertices, 10,
		CAT_WHITE,
		CAT_POLY_MODE_LINES
	);

	CAT_ivec2 target = point_on_bar(casting_bar.target);
	CAT_ivec2 guess = point_on_bar(casting_bar.guess);
	CAT_discberry(target.x, target.y, casting_bar.committed ? 3 : 4, CAT_RED);
	CAT_circberry(guess.x, guess.y, 6, CAT_WHITE);
	if(casting_bar.committed)
		CAT_circberry(target.x, target.y, casting_bar.error * casting_bar.length, CAT_RED);
}

float cast_t = 0.0f;
float cast_dir = 1.0f;

bool fading = false;
float fade_t = 0.0f;

void MS_cast(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(render_MS_cast);
			init_rings();
			init_casting_bar((CAT_ivec2){CAT_LCD_SCREEN_W/2, CAT_LCD_SCREEN_H/2}, CAT_LCD_SCREEN_H * 0.75f);
			cast_t = 0;
			cast_dir = 0;
			fading = false;
			fade_t = 0;
		}
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				if(!casting_bar.committed)
				{
					casting_bar.committed = true;
					casting_bar.error = fabs(casting_bar.target - casting_bar.guess);

					spawn_ring(CAT_iv2v(point_on_bar(casting_bar.target)), SCREEN_DIAG, 320);
				}
				else if(!fading)
				{
					fading = true;
					customize_ring
					(
						spawn_ring
						(
							CAT_iv2v(point_on_bar(casting_bar.target)),
							SCREEN_DIAG * 2, SCREEN_DIAG / FADE_DURATION
						),
						CAT_BLACK, true
					);
				}
			}

			if(!casting_bar.committed)
			{
				cast_t += cast_dir * CAT_get_delta_time_s() / 3.0f;
				cast_t = clamp(cast_t, 0.2f, 0.8f);
				if(cast_t >= 0.79f)
					cast_dir = -1.0f;
				else if(cast_t <= 0.21f)
					cast_dir = 1.0f;
				casting_bar.guess = CAT_ease_inout_elastic(cast_t);
			}
			else if(fading)
			{
				fade_t += CAT_get_delta_time_s();
				if(fade_t >= FADE_DURATION)
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
	render_casting_bar();
	render_rings();
}

void MS_fish(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(render_MS_fish);
			init_rings();
			init_fish((CAT_vec2) {120, 160}, (CAT_vec2) {0, 1.0f}, 16.0f);
			cursor = (CAT_vec2) {120, 160};
		}
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);

			if(CAT_input_touching())
			{
				cursor = CAT_iv2v(CAT_input_cursor());
			}
			else
			{
				float speed = 96 * CAT_get_delta_time_s();
				if(CAT_input_held(CAT_BUTTON_UP, 0))
					cursor.y -= speed;
				if(CAT_input_held(CAT_BUTTON_DOWN, 0))
					cursor.y += speed;
				if(CAT_input_held(CAT_BUTTON_LEFT, 0))
					cursor.x -= speed;
				if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
					cursor.x += speed;
			}
				
			cursor.x = clamp(cursor.x, 0, CAT_LCD_SCREEN_W-1);
			cursor.y = clamp(cursor.y, 0, CAT_LCD_SCREEN_H-1);
			
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
	CAT_draw_sprite(&gizmo_target_17x17_sprite, 0, cursor.x, cursor.y);
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