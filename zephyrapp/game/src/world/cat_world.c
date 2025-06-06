#include "cat_world.h"

#include "cat_render.h"
#include "sprite_assets.h"
#include "cat_input.h"
#include "cat_pet.h"

#define WORLD_MIN_X 0
#define WORLD_MIN_Y (wall_alley_sprite.height + 8)
#define WORLD_WIDTH 240
#define WORLD_HEIGHT (320 - WORLD_MIN_Y)
#define WORLD_MAX_X (WORLD_MIN_X + WORLD_WIDTH - 1)
#define WORLD_MAX_Y (WORLD_MIN_Y + WORLD_HEIGHT - 1)

CAT_vec2 pet_position;
CAT_vec2 pet_velocity;
enum {LEFT, RIGHT} pet_orientation = RIGHT;

static void draw_sky()
{
	CAT_datetime time;
	CAT_get_datetime(&time);
	int sky_row_offset = time.hour * (480 - wall_alley_sprite.height) / 23;

	for(int x = 16; x < 16+16*5; x += 8)
	{
		CAT_set_draw_mask(x, 0, 16+16*5, wall_alley_sprite.height);
		CAT_draw_sprite(&sky_gradient_sprite, 0, x, -sky_row_offset);
	}
}

static void draw_pet()
{
	CAT_anim_tick(&AM_pet);

	int draw_flags = CAT_DRAW_FLAG_BOTTOM | CAT_DRAW_FLAG_CENTER_X;
	draw_flags |= pet_orientation == RIGHT ? CAT_DRAW_FLAG_REFLECT_X : 0;
	CAT_set_draw_flags(draw_flags);
	CAT_draw_sprite(CAT_anim_read(&AM_pet), -1, pet_position.x, pet_position.y);
}

static void render_world()
{
	draw_sky();
	CAT_draw_sprite(&wall_alley_sprite, 0, 0, 0);
	CAT_draw_sprite(&floor_alley_sprite, 0, 0, wall_alley_sprite.height);
	draw_pet();
}

static bool wall_collision()
{
	bool colliding = false;
	if(pet_position.x < WORLD_MIN_X || pet_position.x > WORLD_MAX_X)
	{
		pet_position.x = clamp(pet_position.x, WORLD_MIN_X, WORLD_MAX_X);
		pet_velocity.x = 0;
		colliding = true;
	}
	if(pet_position.y < WORLD_MIN_Y || pet_position.y > WORLD_MAX_Y)
	{
		pet_position.y = clamp(pet_position.y, WORLD_MIN_Y, WORLD_MAX_Y);
		pet_velocity.y = 0;
		colliding = true;
	}
	return colliding;
}

void CAT_MS_world(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			CAT_set_render_callback(render_world);

			pet_position = (CAT_vec2) {120, 160};
			pet_velocity = (CAT_vec2) {0, 0};
		break;

		case CAT_MACHINE_SIGNAL_TICK:
			pet_velocity = (CAT_vec2) {0, 0};
			if(CAT_input_held(CAT_BUTTON_LEFT, 0))
				pet_velocity.x -= 1;
			if(CAT_input_held(CAT_BUTTON_RIGHT, 0))
				pet_velocity.x += 1;
			if(CAT_input_held(CAT_BUTTON_UP, 0))
				pet_velocity.y -= 1;
			if(CAT_input_held(CAT_BUTTON_DOWN, 0))
				pet_velocity.y += 1;
			
			if(pet_velocity.x > 0)
				pet_orientation = RIGHT;
			else if(pet_velocity.x < 0)
				pet_orientation = LEFT;

			pet_position = CAT_vec2_add(pet_position, CAT_vec2_mul(pet_velocity, 64 * CAT_get_delta_time_s()));
			wall_collision();

			if(pet_velocity.x != 0 || pet_velocity.y != 0)
				CAT_anim_transition(&AM_pet, &AS_walk);
			else
				CAT_anim_transition(&AM_pet, &AS_idle);
		break;

		case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}