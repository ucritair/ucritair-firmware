#include "cat_play.h"

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

static int toy_id = -1;

static void select_toy(int item_id)
{
	toy_id = item_id;
}

void CAT_MS_play(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_render_play);
			CAT_gui_begin_item_grid_context(true);
			toy_id = -1;
			break;
		}
		case CAT_FSM_SIGNAL_TICK:
		{
			if(toy_id == -1)
			{
				CAT_gui_begin_item_grid();
				CAT_gui_item_grid_add_tab("Toys", NULL, select_toy);
				/*for(int i = 0; i < item_table.length; i++)
				{
					CAT_item* item = CAT_get_item(i);
					if(item->type == CAT_ITEM_TYPE_TOOL && item->tool_type == CAT_TOOL_TYPE_TOY)
						CAT_gui_item_grid_cell(i);
				}*/
				CAT_gui_item_grid_cell(toy_laser_pointer_item);
			}
			else if(toy_id == toy_laser_pointer_item)
				CAT_pushdown_push(CAT_MS_laser);
			break;
		}
		case CAT_FSM_SIGNAL_EXIT:
		{
			break;
		}
	}
}

void CAT_render_play()
{
	return;
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
				CAT_pushdown_rebase(CAT_MS_room);
				
			if (CAT_input_touching())
			{
				CAT_touch touch;
				CAT_get_touch(&touch);

				laser_pos.x = CAT_clamp(touch.x, CAT_ROOM_X, CAT_ROOM_MAX_X);
				laser_pos.y = CAT_clamp(touch.y, CAT_ROOM_Y, CAT_ROOM_MAX_Y);

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
			laser_pos.x = CAT_clamp(laser_pos.x, CAT_ROOM_X, CAT_ROOM_MAX_X);
			laser_pos.y = CAT_clamp(laser_pos.y, CAT_ROOM_Y, CAT_ROOM_MAX_Y);
			laser_dir = (CAT_vec2){0, 0};

			switch (laser_state)
			{
			case SEEKING:
				if (!CAT_anim_is_in(&AM_pet, &AS_walk))
					CAT_anim_transition(&AM_pet, &AS_walk);
				else
				{
					CAT_vec2 seek_ray = CAT_vec2_sub(laser_pos, pet.pos);
					seek_dist = sqrtf(CAT_vec2_mag2(seek_ray));
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

