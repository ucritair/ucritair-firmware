#include "cat_actions.h"

#include "cat_room.h"
#include "cat_input.h"
#include "cat_bag.h"
#include "cat_pet.h"
#include "cat_render.h"
#include <stdio.h>
#include <stddef.h>
#include "cat_item_dialog.h"
#include "sprite_assets.h"
#include <math.h>
#include "cat_math.h"
#include "cat_item.h"

// MECHANIC PROFILE

static CAT_tool_type tool_type;

bool tool_filter(int item_id)
{
	CAT_item *item = CAT_item_get(item_id);
	return item->type == CAT_ITEM_TYPE_TOOL &&
		   item->data.tool_data.type == tool_type;
}

static CAT_machine_state action_MS;
static CAT_anim_state *action_AS;
static CAT_anim_state *result_AS;

static uint8_t result_colour[3];

// MECHANIC STATE

static CAT_ivec2 cursor;
static int tool_id = -1;
static CAT_vec2 pet_anchor;
static CAT_vec2 tool_anchor;
static bool action_confirmed = false;
static bool action_complete = false;
static int timer_id = -1;

void action_enter()
{
	if (timer_id == -1)
		timer_id = CAT_timer_init(2.0f);
	else
		CAT_timer_reset(timer_id);

	if (tool_id == -1)
		cursor = CAT_largest_free_space();
	cursor = CAT_nearest_free_space(cursor);

	CAT_pet_settle();
}

static void control_cursor()
{
	if (CAT_input_pulse(CAT_BUTTON_UP))
		cursor.y -= 1;
	if (CAT_input_pulse(CAT_BUTTON_RIGHT))
		cursor.x += 1;
	if (CAT_input_pulse(CAT_BUTTON_DOWN))
		cursor.y += 1;
	if (CAT_input_pulse(CAT_BUTTON_LEFT))
		cursor.x -= 1;
	cursor.x = clamp(cursor.x, 0, CAT_GRID_WIDTH - 1);
	cursor.y = clamp(cursor.y, 0, CAT_GRID_HEIGHT - 1);
}

void apply_tool()
{
	CAT_item *tool = CAT_item_get(tool_id);
	if (tool == NULL)
		return;

	pet.vigour = clamp(pet.vigour + tool->data.tool_data.dv, 0, 12);
	pet.focus = clamp(pet.focus + tool->data.tool_data.df, 0, 12);
	pet.spirit = clamp(pet.spirit + tool->data.tool_data.ds, 0, 12);
}

void action_tick()
{
	if (CAT_input_pressed(CAT_BUTTON_B))
		CAT_machine_transition(CAT_MS_room);

	if (tool_id == -1)
	{
		CAT_filter_item_dialog(tool_filter);
		CAT_target_item_dialog(&tool_id, true);
		CAT_machine_transition(CAT_MS_item_dialog);
		return;
	}

	if (tool_id == toy_laser_pointer_item)
	{
		CAT_machine_transition(CAT_MS_laser);
		return;
	}

	if (!action_confirmed)
	{
		control_cursor();

		if (CAT_input_pressed(CAT_BUTTON_A))
		{
			CAT_rect action_rect = CAT_rect_place(cursor, (CAT_ivec2){1, 1});
			if (CAT_is_block_free(action_rect))
			{
				CAT_ivec2 world_cursor = CAT_grid2world(cursor);
				tool_anchor = (CAT_vec2){world_cursor.x + 8, world_cursor.y + 16};
				pet_anchor = (CAT_vec2){
					tool_anchor.x > pet.pos.x ? tool_anchor.x - 24 : tool_anchor.x + 24,
					tool_anchor.y};

				action_confirmed = true;
				CAT_anim_transition(&AM_pet, &AS_walk);
			}
		}

		return;
	}

	if (CAT_anim_is_in(&AM_pet, &AS_walk) && CAT_pet_seek(pet_anchor))
	{
		CAT_pet_face(tool_anchor);
		CAT_anim_transition(&AM_pet, action_AS);
	}
	if (CAT_anim_is_in(&AM_pet, action_AS) && CAT_anim_is_ticking(&AM_pet))
	{
		if (CAT_timer_tick(timer_id) || CAT_input_pressed(CAT_BUTTON_A))
		{
			apply_tool();

			CAT_item *item = CAT_item_get(tool_id);
			if (item->data.tool_data.type == CAT_TOOL_TYPE_FOOD)
				CAT_item_list_remove(&bag, tool_id, 1);
			action_complete = true;

			CAT_anim_transition(&AM_pet, result_AS);
		}
	}
	if (CAT_anim_is_in(&AM_pet, result_AS))
	{
		CAT_set_LEDs(
			result_colour[0],
			result_colour[1],
			result_colour[1]);

		CAT_anim_transition(&AM_pet, &AS_idle);
		if (CAT_input_pressed(CAT_BUTTON_A))
			CAT_anim_kill(&AM_pet);
	}
	if (CAT_anim_is_in(&AM_pet, &AS_idle))
	{
		CAT_machine_transition(CAT_MS_room);
	}
}

void action_exit()
{
	tool_id = -1;
	action_confirmed = false;
	action_complete = false;

	CAT_set_LEDs(0, 0, 0);
}

void CAT_MS_play(CAT_machine_signal signal)
{
	switch (signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_action);
			tool_type = CAT_TOOL_TYPE_TOY;
			action_MS = CAT_MS_play;
			action_AS = &AS_play;
			result_AS = &AS_spi_up;
			result_colour[0] = 76;
			result_colour[1] = 71;
			result_colour[2] = 255;
			action_enter();
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			action_tick();
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			action_exit();
			break;
		}
	}
}

void CAT_render_action()
{
	CAT_render_room();

	if (tool_id != -1)
	{
		CAT_item *item = CAT_item_get(tool_id);
		if (!action_confirmed)
		{
			int mode = CAT_DRAW_FLAG_BOTTOM;
			CAT_ivec2 place = CAT_grid2world(cursor);
			CAT_draw_queue_add(item->data.tool_data.cursor, 0, PROPS_LAYER, place.x, place.y + 16, mode);
			CAT_draw_queue_add(&tile_hl_sprite, 0, GUI_LAYER, place.x, place.y + 16, mode);
		}
		else if (!action_complete)
		{
			int mode = CAT_DRAW_FLAG_BOTTOM | CAT_DRAW_FLAG_CENTER_X;
			if (tool_anchor.x > pet_anchor.x)
				mode |= CAT_DRAW_FLAG_REFLECT_X;
			int tool_layer = item->data.tool_data.type == CAT_TOOL_TYPE_FOOD ? STATICS_LAYER : PROPS_LAYER;
			CAT_draw_queue_add(item->sprite, -1, tool_layer, tool_anchor.x, tool_anchor.y, mode);
		}
	}
}

static float laser_speed = 72.0f;
static CAT_vec2 laser_pos = {120, 180};
static CAT_vec2 laser_dir = {0, 0};
static float seek_dist = 0;
static CAT_vec2 pounce_start = {120, 180};
static CAT_vec2 pounce_end = {120, 180};
static int play_timer_id = -1;

enum
{
	SEEKING,
	POUNCING,
	PLAYING,
	BORED
} laser_state = BORED;

void CAT_MS_laser(CAT_machine_signal signal)
{
	switch (signal)
	{
	case CAT_MACHINE_SIGNAL_ENTER:
		CAT_set_render_callback(CAT_render_laser);
		CAT_pet_settle();
		play_timer_id = CAT_timer_init(CAT_rand_float(1.5f, 3.0f));
		break;
	case CAT_MACHINE_SIGNAL_TICK:
		if (CAT_input_pressed(CAT_BUTTON_B))
			CAT_machine_transition(CAT_MS_room);
		if (CAT_input_touching())
		{
			CAT_touch touch;
			CAT_get_touch(&touch);

			/* clamp to the room’s pixel bounds so the dot never escapes */
			laser_pos.x = clamp((float)touch.x,
								CAT_WORLD_MIN_X, CAT_WORLD_MAX_X);
			laser_pos.y = clamp((float)touch.y,
								CAT_WORLD_MIN_Y, CAT_WORLD_MAX_Y);

			if (play_timer_id != -1) /* stop boredom while touching */
				CAT_timer_reset(play_timer_id);

			/* skip the D‑pad block this frame */
			laser_dir = (CAT_vec2){0, 0};
		}
		else
		{
			if (CAT_input_held(CAT_BUTTON_RIGHT, 0))
				laser_dir.x += 1.0f;
			if (CAT_input_held(CAT_BUTTON_UP, 0))
				laser_dir.y -= 1.0f;
			if (CAT_input_held(CAT_BUTTON_LEFT, 0))
				laser_dir.x -= 1.0f;
			if (CAT_input_held(CAT_BUTTON_DOWN, 0))
				laser_dir.y += 1.0f;
		}

		laser_pos = CAT_vec2_add(
		laser_pos, CAT_vec2_mul(laser_dir, laser_speed * CAT_get_delta_time_s()));
		laser_pos.x = clamp(laser_pos.x, CAT_WORLD_MIN_X, CAT_WORLD_MAX_X);
		laser_pos.y = clamp(laser_pos.y, CAT_WORLD_MIN_Y, CAT_WORLD_MAX_Y);
		laser_dir = (CAT_vec2){0, 0};

		switch (laser_state)
		{
		case SEEKING:
			if (!CAT_anim_is_in(&AM_pet, &AS_walk))
				CAT_anim_transition(&AM_pet, &AS_walk);
			else
			{
				CAT_vec2 seek_ray = CAT_vec2_sub(laser_pos, pet.pos);
				seek_dist = sqrt(CAT_vec2_mag2(seek_ray));
				if(seek_dist < 32)
				{
					if(CAT_pet_seek(laser_pos))
						laser_state = PLAYING;
				}
				else
				{
					CAT_vec2 seek_dir = CAT_vec2_mul(seek_ray, 1.0f/seek_dist);
					CAT_vec2 trailing_point = CAT_vec2_sub(laser_pos, CAT_vec2_mul(seek_dir, 48));
					trailing_point.y = laser_pos.y;

					pounce_start = trailing_point;
					pounce_end = laser_pos;
					if(CAT_pet_seek(pounce_start))
						laser_state = POUNCING;
					CAT_pet_face(laser_pos);
				}
			}
			break;
		case POUNCING:
			if(!CAT_anim_is_in(&AM_pet, &AS_pounce))
			{
				pet.rot = pounce_end.x < pounce_start.x ? 1 : -1;
				CAT_anim_transition(&AM_pet, &AS_pounce);
				pet.pos.x -= 16 * pet.rot;
			}
			else if(CAT_anim_is_ending(&AM_pet))
			{
				pet.pos = pounce_end;
				CAT_anim_transition(&AM_pet, &AS_play);
				laser_state = PLAYING;
			}
			break;
		case PLAYING:
			if (!CAT_anim_is_in(&AM_pet, &AS_play))
				CAT_anim_transition(&AM_pet, &AS_play);
			if (CAT_timer_tick(play_timer_id))
			{
				CAT_timer_delete(play_timer_id);
				play_timer_id = CAT_timer_init(CAT_rand_float(1.0f, 3.0f));
				laser_state = BORED;
			}
			if (CAT_vec2_dist2(pet.pos, laser_pos) >= 16)
				laser_state = SEEKING;
			break;
		case BORED:
			if (!CAT_anim_is_in(&AM_pet, &AS_idle))
				CAT_anim_transition(&AM_pet, &AS_idle);
			if (CAT_vec2_dist2(pet.pos, laser_pos) >= 32)
				laser_state = SEEKING;
			break;
		}
		break;
	case CAT_MACHINE_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_laser()
{
	CAT_render_room();

	CAT_item *item = CAT_item_get(toy_laser_pointer_item);
	CAT_draw_flag flags = CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y;
	CAT_draw_queue_add(item->data.tool_data.cursor, -1, 0, laser_pos.x, laser_pos.y, flags);
}

