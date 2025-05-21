#include "cat_actions.h"

#include "cat_render.h"
#include "cat_input.h"

CAT_vec2 cursor;
bool cursor_committed = false;

typedef struct
{
	CAT_vec2 center;
	float max_radius;
	float speed;

	float radius;
} study_ring;
study_ring rings[16];
int ring_count = 0;

int spawn_ring(CAT_vec2 center, float max_radius, float speed)
{
	if(ring_count >= 16)
		return -1;
	int idx = ring_count;
	study_ring* ring = &rings[idx];
	ring->center = center;
	ring->max_radius = max_radius;
	ring->speed = speed;
	ring->radius = 0;
	ring_count += 1;
	return idx;
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
		CAT_circberry(ring->center.x, ring->center.y, ring->radius, CAT_WHITE);
	}
}

struct
{
	CAT_vec2 positions[3];
	CAT_vec2 headings[3];
	float radii[3];
	CAT_vec2 target;
} fish;

int fish_turn_timer_id = -1;
bool fish_bite_trigger = true;

void spawn_fish(CAT_vec2 lead_position, CAT_vec2 lead_heading, float lead_radius)
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
			spawn_ring(cursor, 400, 64);
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

void CAT_MS_study(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_study);
			
			spawn_fish((CAT_vec2){120, 160}, (CAT_vec2){0, 1.0f}, 16.0f);
			ring_count = 0;
		}
		break;

		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_back();

			cursor = (CAT_vec2) {input.touch.x, input.touch.y};
			
			fish_tick();
			rings_tick();
		}
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		{

		}
		break;
	}
}

void CAT_render_study()
{
	CAT_frameberry(CAT_BLACK);
	render_fish();
	render_rings();
}