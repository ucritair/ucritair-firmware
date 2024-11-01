#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cat_core.h"
#include "cat_sprite.h"
#include "cat_math.h"
#include "cat_item.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "cat_machine.h"

#pragma region CONSTANTS

#define CAT_MAX_PROP_COUNT 210

#pragma endregion

#pragma region DATA

typedef enum CAT_screen
{
	CAT_SCREEN_ROOM,
	CAT_SCREEN_MENU,
	CAT_SCREEN_STATS,
	CAT_SCREEN_BAG
} CAT_screen;
CAT_screen screen;

typedef enum CAT_mode
{
	CAT_MODE_DEFAULT,
	CAT_MODE_FEED,
	CAT_MODE_STUDY,
	CAT_MODE_PLAY,
	CAT_MODE_DECO
} CAT_mode;
CAT_mode mode;

typedef enum CAT_pet_status
{
	CAT_PET_STATUS_IDLE,
	CAT_PET_STATUS_WALK,
	CAT_PET_STATUS_PET,
	CAT_PET_STATUS_FEED,
	CAT_PET_STATUS_STUDY,
	CAT_PET_STATUS_PLAY,
	CAT_PET_STATUS_DEATH
} CAT_pet_status;

typedef struct CAT_pet
{
	CAT_pet_status status;

	float vigour;
	float focus;
	float spirit;
	float delta_vigour;
	float delta_focus;
	float delta_spirit;

	CAT_vec2 targ;
	CAT_vec2 pos;
	CAT_vec2 vel;
	
	int walk_timer_id;
	int mood_timer_id;
	int stat_timer_id;
} CAT_pet;
CAT_pet pet;

typedef struct CAT_room
{
	CAT_ivec2 min;
	CAT_ivec2 max;
	int props[CAT_MAX_PROP_COUNT];
	CAT_ivec2 places[CAT_MAX_PROP_COUNT];
	int prop_count;
	CAT_ivec2 cursor;

	void (*buttons[5])();
	int selector;
} CAT_room;
CAT_room room;

typedef struct CAT_menu_screen
{
	const char* items[3];
	int idx;
} CAT_menu_screen;
CAT_menu_screen menu_screen;

typedef struct CAT_bag_screen
{
	int window;
	int base;
	int idx;
} CAT_bag_screen;
CAT_bag_screen bag_screen;

#pragma endregion

#pragma region PET

void CAT_pet_stat()
{
	pet.vigour += pet.delta_vigour;
	pet.focus += pet.delta_focus;
	pet.spirit += pet.delta_spirit;
}

bool CAT_pet_seek()
{
	CAT_vec2 line = CAT_vec2_sub(pet.targ, pet.pos);
	float dist = CAT_vec2_mag(line);
	if(dist < 8)
	{
		pet.vel = (CAT_vec2) {0, 0};
		return true;
	}
	else
	{
		pet.vel = CAT_vec2_mul(line, 48.0f/dist);
		pet.pos = CAT_vec2_add(pet.pos, CAT_vec2_mul(pet.vel, CAT_get_delta_time()));
		return false;
	}
}

void CAT_pet_eat(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	pet.vigour += item->data.food_data.d_v;
	pet.focus += item->data.food_data.d_f;
	pet.spirit += item->data.food_data.d_s;
	pet.delta_vigour += item->data.food_data.dd_v;
	pet.delta_focus += item->data.food_data.dd_f;
	pet.delta_spirit += item->data.food_data.dd_s;
}

void CAT_pet_init()
{
	pet.status = CAT_PET_STATUS_IDLE;

	pet.vigour = 12;
	pet.focus = 12;
	pet.spirit = 12;
	pet.delta_vigour = -1;
	pet.delta_focus = -1;
	pet.delta_spirit = -1;

	pet.targ = (CAT_vec2) {-1, -1};
	pet.vel = (CAT_vec2) {0, 0};
	pet.pos = (CAT_vec2) {120, 200};
	
	pet.walk_timer_id = CAT_timer_init(3.0f);
	pet.mood_timer_id = CAT_timer_init(2.0f);
	pet.stat_timer_id = CAT_timer_init(3.0f);
}

#pragma endregion

#pragma region ROOM

void CAT_room_move_cursor()
{
	if(CAT_input_pulse(CAT_BUTTON_UP))
		room.cursor.y -= 1;
	if(CAT_input_pulse(CAT_BUTTON_RIGHT))
		room.cursor.x += 1;
	if(CAT_input_pulse(CAT_BUTTON_DOWN))
		room.cursor.y += 1;
	if(CAT_input_pulse(CAT_BUTTON_LEFT))
		room.cursor.x -= 1;
	room.cursor.x = clamp(room.cursor.x, room.min.x, room.max.x);
	room.cursor.y = clamp(room.cursor.y, room.min.y, room.max.y);
}

bool CAT_room_emplace(int prop_id, CAT_ivec2 place)
{
	if(room.prop_count >= CAT_MAX_PROP_COUNT)
		return false;

	CAT_item* prop = CAT_item_get(prop_id);
	CAT_ivec2 shape = prop->data.prop_data.shape;
	CAT_ivec2 prop_min = (CAT_ivec2) {place.x, place.y - shape.y};
	CAT_ivec2 prop_max = (CAT_ivec2) {place.x + shape.x, place.y};

	if(!CAT_test_contain(room.min, room.max, prop_min, prop_max))
		return false;
	for(int i = 0; i < room.prop_count; i++)
	{
		CAT_ivec2 other_place = room.places[i];
		CAT_item* other = CAT_item_get(room.props[i]);
		CAT_ivec2 other_shape = other->data.prop_data.shape;
		CAT_ivec2 other_min = (CAT_ivec2) {other_place.x, other_place.y - other_shape.y};
		CAT_ivec2 other_max = (CAT_ivec2) {other_place.x + other_shape.x, other_place.y};
		if(CAT_test_overlap(prop_min, prop_max, other_min, other_max))
			return false;
	}

	int idx = room.prop_count;
	room.prop_count += 1;
	room.props[idx] = prop_id;
	room.places[idx] = place;
	return true;
}

CAT_machine_state machine;

void CAT_MS_default(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			mode = CAT_MODE_DEFAULT;
			pet.status = CAT_PET_STATUS_IDLE;
			CAT_timer_reset(pet.walk_timer_id);
			CAT_timer_reset(pet.mood_timer_id);
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				room.selector += 1;
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				room.selector -= 1;
			room.selector = clamp(room.selector, 0, 4);
			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				room.buttons[room.selector]();
			}

			if(CAT_input_touch(pet.pos.x, pet.pos.y - 16, 8))
			{
				pet.status = CAT_PET_STATUS_PET;	
			}

			if(CAT_timer_tick(pet.stat_timer_id))
			{
				CAT_pet_stat();
			}

			switch(pet.status)
			{
				case CAT_PET_STATUS_IDLE:
					if(CAT_timer_tick(pet.walk_timer_id))
					{
						pet.targ = (CAT_vec2)
						{
							rand_float(room.min.x * 16, room.max.x * 16),
							rand_float(room.min.y * 16, room.max.y * 16)
						};
						pet.status = CAT_PET_STATUS_WALK;
					}
					break;
				case CAT_PET_STATUS_WALK:
					if(CAT_pet_seek())
					{
						pet.status = CAT_PET_STATUS_IDLE;
					}
					break;
				case CAT_PET_STATUS_PET:
					if(CAT_timer_tick(pet.mood_timer_id))
					{
						pet.status = CAT_PET_STATUS_IDLE;
					}
					break;
			}

			
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

void CAT_MS_feed(CAT_machine_signal signal)
{
	static bool committed;

	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			mode = CAT_MODE_FEED;
			pet.status = CAT_PET_STATUS_IDLE;
			committed = false;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_room_move_cursor();

			if(!committed)
			{
				if(CAT_input_pressed(CAT_BUTTON_A))
				{
					pet.targ = (CAT_vec2) {room.cursor.x * 16, room.cursor.y * 16};
					committed = true;
				}
			}
			else
			{
				if(CAT_pet_seek())
				{
					pet.status = CAT_PET_STATUS_FEED;
				}
				else
				{
					pet.status = CAT_PET_STATUS_WALK;
				}
			}

			if(pet.status == CAT_PET_STATUS_FEED)
			{
				if(CAT_timer_tick(pet.mood_timer_id))
				{
					mode = CAT_MODE_DEFAULT;
					CAT_machine_transition(&machine, CAT_MS_default);
				}
			}

			if(CAT_input_pressed(CAT_BUTTON_B))
			{
				mode = CAT_MODE_DEFAULT;
				CAT_machine_transition(&machine, CAT_MS_default);
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

void CAT_MS_deco(CAT_machine_signal signal)
{
	static int prop_id = 0;

	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			mode = CAT_MODE_DECO;
			pet.status = CAT_PET_STATUS_IDLE;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{
			CAT_room_move_cursor();

			if(CAT_input_held(CAT_BUTTON_A, 0.25))
			{
				CAT_item* prop = CAT_item_get(prop_id);
				CAT_ivec2 shape = prop->data.prop_data.shape;
				CAT_ivec2 prop_min = (CAT_ivec2) {room.cursor.x, room.cursor.y - shape.y};
				CAT_ivec2 prop_max = (CAT_ivec2) {room.cursor.x + shape.x, room.cursor.y};
				for(int y = prop_min.y; y < prop_max.y; y++)
				{
					for(int x = prop_min.x; x < prop_max.x; x++)
					{
						CAT_draw_queue_add(cursor_sprite_id[9], 3, x * 16, y * 16, CAT_DRAW_MODE_DEFAULT);
					}
				}
			}
			else if(CAT_input_released(CAT_BUTTON_A))
			{
				if(CAT_room_emplace(prop_id, room.cursor))
					prop_id = CAT_bag_seek(prop_id+1, CAT_ITEM_TYPE_PROP);
			}
			else
			{
				CAT_draw_queue_add(cursor_sprite_id[18], 3, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
			}

			if(CAT_input_pressed(CAT_BUTTON_B))
			{
				mode = CAT_MODE_DEFAULT;
				CAT_machine_transition(&machine, CAT_MS_default);
			}
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}

void CAT_room_feed_button()
{
	CAT_machine_transition(&machine, CAT_MS_feed);
}
void CAT_room_deco_button()
{
	CAT_machine_transition(&machine, CAT_MS_deco);
}
void CAT_room_menu_button()
{
	screen = CAT_SCREEN_MENU;
}

void CAT_room_init()
{
	room.min = (CAT_ivec2) {0, 7};
	room.max = (CAT_ivec2) {15, 17};
	room.prop_count = 0;
	room.cursor = room.min;

	room.buttons[0] = CAT_room_feed_button;
	room.buttons[1] = CAT_room_feed_button;
	room.buttons[2] = CAT_room_feed_button;
	room.buttons[3] = CAT_room_deco_button;
	room.buttons[4] = CAT_room_menu_button;
	room.selector = 0;
}

#pragma endregion

#pragma region MENU SCREEN LOGIC

void CAT_menu_screen_init()
{
	menu_screen.items[0] = "STATS";
	menu_screen.items[1] = "BAG";
	menu_screen.items[2] = "BACK";
	menu_screen.idx = 0;
}

void CAT_menu_screen_logic()
{
	if(CAT_input_pulse(CAT_BUTTON_UP))
		menu_screen.idx -= 1;
	if(CAT_input_pulse(CAT_BUTTON_DOWN))
		menu_screen.idx += 1;
	menu_screen.idx = clamp(menu_screen.idx, 0, 2);

	if(CAT_input_pressed(CAT_BUTTON_A))
	{
		if(menu_screen.idx == 0)
			screen = CAT_SCREEN_STATS;
		if(menu_screen.idx == 1)
			screen = CAT_SCREEN_BAG;
		if(menu_screen.idx == 2)
			screen = CAT_SCREEN_ROOM;
	}

	if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
		screen = CAT_SCREEN_ROOM;
}

#pragma endregion

#pragma region STATS SCREEN LOGIC

void CAT_stats_screen_logic()
{
	if(CAT_input_pressed(CAT_BUTTON_B))
		screen = CAT_SCREEN_MENU;
	if(CAT_input_pressed(CAT_BUTTON_START))
		screen = CAT_SCREEN_ROOM;
}

#pragma endregion

#pragma region BAG SCREEN LOGIC

void CAT_bag_screen_init()
{
	bag_screen.window = 9;
	bag_screen.base = 0;
	bag_screen.idx = 0;
}

void CAT_bag_screen_logic()
{
	int dir = 0;
	int limit = bag_screen.idx;
	if(CAT_input_pulse(CAT_BUTTON_UP))
	{
		dir = -1;
		limit = -1;
	}
	if(CAT_input_pulse(CAT_BUTTON_DOWN))

	{
		dir = 1;
		limit = CAT_ITEM_TABLE_MAX_LENGTH;
	}
	for(int i = bag_screen.idx+dir; i != limit; i += dir)
	{
		if(CAT_bag_count(i) > 0)
		{
			bag_screen.idx = i;
			break;
		}
	}

	if(CAT_input_pressed(CAT_BUTTON_B))
		screen = CAT_SCREEN_MENU;
	if(CAT_input_pressed(CAT_BUTTON_START))
		screen = CAT_SCREEN_ROOM;
}

#pragma endregion

void CAT_logic()
{
	switch(screen)
	{
		case CAT_SCREEN_ROOM:
			CAT_machine_tick(&machine);
			break;
		case CAT_SCREEN_MENU:
			CAT_menu_screen_logic();
			break;
		case CAT_SCREEN_STATS:
			CAT_stats_screen_logic();	
			break;
		case CAT_SCREEN_BAG:
			CAT_bag_screen_logic();
			break;
	}
}

void CAT_render(int step)
{
	switch(screen)
	{
		case CAT_SCREEN_ROOM:
		{
			CAT_draw_tiles(0, 4, wall_sprite_id[0]);
			CAT_draw_tiles(4, 1, wall_sprite_id[1]);
			CAT_draw_tiles(5, 1, wall_sprite_id[2]);
			CAT_draw_tiles(6, 1, floor_sprite_id[2]);
			CAT_draw_tiles(7, 10, floor_sprite_id[0]);
			CAT_draw_tiles(17, 3, floor_sprite_id[1]);

			CAT_draw_queue_add(window_sprite_id, 2, 16, 8, CAT_DRAW_MODE_DEFAULT);
			CAT_anim_queue_add(vending_anim_id, 2, 164, 112, CAT_DRAW_MODE_BOTTOM);
			
			for(int i = 0; i < room.prop_count; i++)
			{
				int prop_id = room.props[i];
				CAT_item* prop = CAT_item_get(prop_id);
				CAT_ivec2 place = room.places[i];

				int anim_id = prop->data.prop_data.anim_id;
				if(anim_id != -1)
				{
					CAT_anim_queue_add(anim_id, 2, place.x * 16, place.y * 16, CAT_DRAW_MODE_BOTTOM);
				}
				else
				{
					CAT_draw_queue_add(prop->sprite, 2, place.x * 16, place.y * 16, CAT_DRAW_MODE_BOTTOM);
				}
			}

			int pet_mode = CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X;
			pet_mode |= (pet.pos.x < pet.targ.x ? CAT_DRAW_MODE_REFLECT_X : 0);

			switch(pet.status)
			{
				case CAT_PET_STATUS_IDLE:
					CAT_anim_queue_add(idle_anim_id, 2, pet.pos.x, pet.pos.y, pet_mode);
					break;
				case CAT_PET_STATUS_WALK:
					CAT_anim_queue_add(walk_anim_id, 2, pet.pos.x, pet.pos.y, pet_mode);
					break;
				case CAT_PET_STATUS_FEED:
				{
					CAT_anim_queue_add(idle_anim_id, 2, pet.pos.x, pet.pos.y, pet_mode);
					int x_off = (pet_mode & CAT_DRAW_MODE_REFLECT_X) > 0 ? 16 : -16;
					CAT_anim_queue_add(fed_anim_id, 3, pet.pos.x + x_off, pet.pos.y - 48, pet_mode);
					break;
				}
				case CAT_PET_STATUS_PET:
				{
					CAT_anim_queue_add(idle_anim_id, 2, pet.pos.x, pet.pos.y, pet_mode);
					int x_off = (pet_mode & CAT_DRAW_MODE_REFLECT_X) > 0 ? 16 : -16;
					CAT_anim_queue_add(fed_anim_id, 3, pet.pos.x + x_off, pet.pos.y - 48, pet_mode);
					break;
				}
				case CAT_PET_STATUS_DEATH:
					CAT_anim_queue_add(death_anim_id, 2, pet.pos.x, pet.pos.y, pet_mode);
					break;
			}		

			CAT_draw_queue_add(feed_button_sprite_id, 3, 8, 280, CAT_DRAW_MODE_DEFAULT); 
			CAT_draw_queue_add(study_button_sprite_id, 3, 56, 280, CAT_DRAW_MODE_DEFAULT); 
			CAT_draw_queue_add(play_button_sprite_id, 3, 104, 280, CAT_DRAW_MODE_DEFAULT);
			CAT_draw_queue_add(deco_button_sprite_id, 3, 152, 280, CAT_DRAW_MODE_DEFAULT);
			CAT_draw_queue_add(menu_button_sprite_id, 3, 200, 280, CAT_DRAW_MODE_DEFAULT);

			switch(mode)
			{
				case CAT_MODE_DEFAULT:
					CAT_draw_queue_add(ring_hl_sprite_id, 4, 8+48*room.selector, 280, CAT_DRAW_MODE_DEFAULT);
					break;
				case CAT_MODE_FEED:
					if(pet.status == CAT_PET_STATUS_WALK)
						CAT_draw_queue_add(seed_sprite_id, 2, pet.targ.x, pet.targ.y, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y);
					else if(pet.status == CAT_PET_STATUS_IDLE)
						CAT_draw_queue_add(cursor_sprite_id[18], 3, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_BOTTOM);
					break;
				case CAT_MODE_DECO:
					break;
			}
			
			CAT_anim_queue_submit(step);
			CAT_draw_queue_submit();
			
			break;
		}
		case CAT_SCREEN_MENU:
		{
			CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
			CAT_gui_text("MENU");
			CAT_gui_image(a_sprite_id);
			CAT_gui_image(enter_sprite_id);
			CAT_gui_image(b_sprite_id);
			CAT_gui_image(exit_sprite_id);

			CAT_gui_panel((CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});  
			for(int i = 0; i < 3; i++)
			{
				CAT_gui_text("#");
				CAT_gui_text(menu_screen.items[i]);

				if(i == menu_screen.idx)
					CAT_gui_image(select_sprite_id);

				CAT_gui_line_break();
			}
			
			break;
		}
		case CAT_SCREEN_STATS:
		{
			CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
			CAT_gui_text("STATS");
			CAT_gui_image(b_sprite_id);
			CAT_gui_image(exit_sprite_id);

			CAT_gui_panel((CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});  
			CAT_gui_image(pet_sprite_id[0]); 
			CAT_gui_line_break();

			CAT_gui_image(vigour_sprite_id);
			CAT_gui_text("VIG");
			for(int i = 1; i <= 12; i++)
			{
				if(i <= pet.vigour)
					CAT_gui_image(cell_sprite_id[0]);
				else
					CAT_gui_image(cell_sprite_id[3]);
			}
			CAT_gui_line_break();

			CAT_gui_image(focus_sprite_id);
			CAT_gui_text("FOC");
			for(int i = 1; i <= 12; i++)
			{
				if(i <= pet.focus)
					CAT_gui_image(cell_sprite_id[1]);
				else
					CAT_gui_image(cell_sprite_id[3]);
			}
			CAT_gui_line_break();

			CAT_gui_image(spirit_sprite_id);
			CAT_gui_text("SPI");
			for(int i = 1; i <= 12; i++)
			{
				if(i <= pet.spirit)
					CAT_gui_image(cell_sprite_id[2]);
				else
					CAT_gui_image(cell_sprite_id[3]);
			}

			break;
		}
		case CAT_SCREEN_BAG:
		{
			CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
			CAT_gui_text("BAG");
			CAT_gui_image(b_sprite_id);
			CAT_gui_image(exit_sprite_id);

			CAT_gui_panel((CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});
			for(int i = 0; i < bag_screen.window; i++)
			{
				int item_id = bag_screen.base + i;
				if(item_id >= item_table.length)
					continue;
				CAT_item* item = CAT_item_get(item_id);
				if(item->count <= 0)
					continue;

				CAT_gui_panel((CAT_ivec2) {0, 32+i*32}, (CAT_ivec2) {15, 2});
				CAT_gui_image(item_sprite_id); 
				char text[64];
				sprintf(text, "%s *%d", item->name, item->count);
				CAT_gui_text(text);

				if(item_id == bag_screen.idx)
				{
					CAT_gui_image(select_sprite_id);
				}
			}

			break;
		}
	}
}

void CAT_init()
{
	rand_init();
	CAT_platform_init();
	CAT_input_init();

	CAT_atlas_init("sprites/atlas.png");
	CAT_spriter_init();
	CAT_draw_queue_init();
	CAT_anim_table_init();
	CAT_anim_queue_init();
	CAT_sprite_mass_define();
	
	CAT_gui_init(panel_sprite_id, glyph_sprite_id);
	
	CAT_item_table_init();
	CAT_item_mass_define();
	
	CAT_timetable_init();

	CAT_pet_init();
	CAT_room_init();
	CAT_menu_screen_init();
	CAT_bag_screen_init();
	
	screen = CAT_SCREEN_ROOM;
	mode = CAT_MODE_DEFAULT;
	machine = NULL;
	CAT_machine_transition(&machine, CAT_MS_default);
}

void CAT_tick_logic()
{
	CAT_platform_tick();
	CAT_input_tick();

	CAT_logic();
}

void CAT_tick_render(int step)
{
	CAT_render(step);

	CAT_LCD_post(spriter.frame);
}

#ifdef CAT_DESKTOP
int main()
{
	CAT_init();

	while (CAT_get_battery_pct() > 0)
	{
		CAT_tick_logic();
		CAT_tick_render(0);
		CAT_tick_render(1);
	}

	CAT_spriter_cleanup();
	CAT_atlas_cleanup();
	CAT_platform_cleanup();
	return 0;
}
#endif