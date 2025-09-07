#include "cat_actions.h"

#include "cat_room.h"
#include "cat_input.h"
#include "cat_pet.h"
#include "cat_render.h"
#include <stdio.h>
#include <stddef.h>
#include "sprite_assets.h"
#include <math.h>
#include "cat_math.h"
#include "cat_item.h"
#include "item_assets.h"
#include "cat_gui.h"

// MECHANIC PROFILE

static CAT_tool_type tool_type;

static CAT_FSM_state action_MS;
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
static float timer = 0;

void action_enter()
{
	timer = 0;
	CAT_room_nearest_free_cell(cursor.x, cursor.y, &cursor.x, &cursor.y);
	CAT_pet_settle();
	CAT_gui_begin_item_grid_context(true);
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
	cursor.x = clamp(cursor.x, 0, CAT_ROOM_GRID_W - 1);
	cursor.y = clamp(cursor.y, 0, CAT_ROOM_GRID_H - 1);
}

void apply_tool()
{
	CAT_item *tool = CAT_get_item(tool_id);
	if (tool == NULL)
		return;

	pet.vigour = clamp(pet.vigour + tool->tool_dv, 0, 12);
	pet.focus = clamp(pet.focus + tool->tool_df, 0, 12);
	pet.spirit = clamp(pet.spirit + tool->tool_ds, 0, 12);
}

void choose_tool(int item_id)
{
	tool_id = item_id;
}

void action_tick()
{
	if (CAT_input_pressed(CAT_BUTTON_B))
		CAT_pushdown_transition(CAT_MS_room);

	if (tool_id == -1)
	{
		CAT_gui_begin_item_grid();
		CAT_gui_item_grid_add_tab("TOYS", NULL, choose_tool);
		if(CAT_inventory_count(toy_laser_pointer_item) > 0)
			CAT_gui_item_grid_cell(toy_laser_pointer_item);
		return;
	}

	if (tool_id == toy_laser_pointer_item)
	{
		CAT_pushdown_transition(CAT_MS_laser);
		return;
	}

	if (!action_confirmed)
	{
		control_cursor();

		if (CAT_input_pressed(CAT_BUTTON_A))
		{
			if(CAT_room_is_block_free(cursor.x, cursor.y, 1, 1))
			{
				int x, y;
				CAT_room_cell2point(cursor.x, cursor.y, &x, &y);
				tool_anchor = (CAT_vec2){x + 8, y + 16};
				pet_anchor = (CAT_vec2)
				{
					tool_anchor.x > pet.pos.x ? tool_anchor.x - 24 : tool_anchor.x + 24,
					tool_anchor.y
				};

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
		if (timer >= 2 || CAT_input_pressed(CAT_BUTTON_A))
		{
			apply_tool();

			CAT_item *item = CAT_get_item(tool_id);
			if (item->tool_type == CAT_TOOL_TYPE_FOOD)
				CAT_inventory_remove(tool_id, 1);
			action_complete = true;

			CAT_anim_transition(&AM_pet, result_AS);

			timer = 0;
		}
		timer += CAT_get_delta_time_s();
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
		CAT_pushdown_transition(CAT_MS_room);
	}
}

void action_exit()
{
	tool_id = -1;
	action_confirmed = false;
	action_complete = false;

	CAT_set_LEDs(0, 0, 0);
}

void CAT_MS_play(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
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
		case CAT_FSM_SIGNAL_TICK:
		{
			action_tick();
			break;
		}
		case CAT_FSM_SIGNAL_EXIT:
		{
			action_exit();
			break;
		}
	}
}

void CAT_render_action()
{
	CAT_room_draw_statics();
	if (CAT_get_render_cycle() == 0)
		CAT_draw_queue_clear();
	CAT_room_draw_props();
	CAT_room_draw_pickups();
	CAT_room_draw_pet();

	if (tool_id != -1)
	{
		CAT_item *item = CAT_get_item(tool_id);
		if (!action_confirmed)
		{
			int mode = CAT_DRAW_FLAG_BOTTOM;
			int x, y;
			CAT_room_cell2point(cursor.x, cursor.y, &x, &y);
			CAT_draw_queue_add(item->tool_cursor, 0, PROPS_LAYER, x, y + 16, mode);
			CAT_draw_queue_add(&tile_hl_sprite, 0, GUI_LAYER, x, y + 16, mode);
		}
		else if (!action_complete)
		{
			int mode = CAT_DRAW_FLAG_BOTTOM | CAT_DRAW_FLAG_CENTER_X;
			if (tool_anchor.x > pet_anchor.x)
				mode |= CAT_DRAW_FLAG_REFLECT_X;
			int tool_layer = item->tool_type == CAT_TOOL_TYPE_FOOD ? STATICS_LAYER : PROPS_LAYER;
			CAT_draw_queue_add(item->sprite, -1, tool_layer, tool_anchor.x, tool_anchor.y, mode);
		}
	}

	CAT_draw_queue_submit();
	CAT_room_draw_gui();
}

static float laser_speed = 72.0f;
static CAT_vec2 laser_pos = {120, 180};
static CAT_vec2 laser_dir = {0, 0};
static float seek_dist = 0;
static CAT_vec2 pounce_start = {120, 180};
static CAT_vec2 pounce_end = {120, 180};
static float play_timer = 0;
static float play_duration = 0;
static int pounce_count = 0;

enum
{
	SEEKING,
	POUNCING,
	PLAYING,
	BORED,
	HAPPY,
} laser_state = BORED;

void CAT_MS_laser(CAT_FSM_signal signal)
{
	switch (signal)
	{
	case CAT_FSM_SIGNAL_ENTER:
		CAT_set_render_callback(CAT_render_laser);
		CAT_pet_settle();
		play_timer = 0;
		play_duration = CAT_rand_float(1.5f, 3.0f);
		break;
	case CAT_FSM_SIGNAL_TICK:
		if (CAT_input_pressed(CAT_BUTTON_B))
			CAT_pushdown_transition(CAT_MS_room);
		if (CAT_input_touching())
		{
			CAT_touch touch;
			CAT_get_touch(&touch);

			laser_pos.x = clamp(touch.x, CAT_ROOM_X, CAT_ROOM_MAX_X);
			laser_pos.y = clamp(touch.y, CAT_ROOM_Y, CAT_ROOM_MAX_Y);

			play_timer = 0;
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
		laser_pos.x = clamp(laser_pos.x, CAT_ROOM_X, CAT_ROOM_MAX_X);
		laser_pos.y = clamp(laser_pos.y, CAT_ROOM_Y, CAT_ROOM_MAX_Y);
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
				pounce_count += 1;
				if(pounce_count == 3)
				{
					pounce_count = 0;
					laser_state = HAPPY;
				}
				else
					laser_state = PLAYING;
			}
			break;
		case PLAYING:
			if (!CAT_anim_is_in(&AM_pet, &AS_play))
				CAT_anim_transition(&AM_pet, &AS_play);

			if (play_timer >= play_duration)
			{
				play_timer = 0;
				play_duration = CAT_rand_float(1.0f, 3.0f);
				laser_state = BORED;
			}
			play_timer += CAT_get_delta_time_s();

			if (CAT_vec2_dist2(pet.pos, laser_pos) >= 16)
				laser_state = SEEKING;
			break;
		case BORED:
			if (!CAT_anim_is_in(&AM_pet, &AS_idle))
				CAT_anim_transition(&AM_pet, &AS_idle);
			if (CAT_vec2_dist2(pet.pos, laser_pos) >= 32)
				laser_state = SEEKING;
			break;
		case HAPPY:
			if(!CAT_anim_is_in(&AM_pet, &AS_spi_up))
				CAT_anim_transition(&AM_pet, &AS_spi_up);
			if(CAT_anim_is_ending(&AM_pet))
			{
				CAT_pet_change_spirit(1);
				laser_state = SEEKING;
			}
		}
		break;
	case CAT_FSM_SIGNAL_EXIT:
		break;
	}
}

void CAT_render_laser()
{
	CAT_room_draw_statics();
	if (CAT_get_render_cycle() == 0)
		CAT_draw_queue_clear();
	CAT_room_draw_props();
	CAT_room_draw_pickups();
	CAT_room_draw_pet();

	CAT_item *item = CAT_get_item(toy_laser_pointer_item);
	CAT_draw_flag flags = CAT_DRAW_FLAG_CENTER_X | CAT_DRAW_FLAG_CENTER_Y;
	CAT_draw_queue_add(item->tool_cursor, -1, PROPS_LAYER, laser_pos.x, laser_pos.y, flags);

	CAT_draw_queue_submit();
	CAT_room_draw_gui();
}

