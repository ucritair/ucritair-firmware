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

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_MAX_PROP_COUNT 210


//////////////////////////////////////////////////////////////////////////
// GAMEMODES

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


//////////////////////////////////////////////////////////////////////////
// PET

typedef enum CAT_pet_status
{
	CAT_PET_STATUS_IDLE,
	CAT_PET_STATUS_WALK,
	CAT_PET_STATUS_MOOD,
	CAT_PET_STATUS_DEATH
} CAT_pet_status;

typedef enum CAT_pet_mood
{
	CAT_PET_MOOD_DEFAULT,
	CAT_PET_MOOD_PET,
	CAT_PET_MOOD_FED,
	CAT_PET_MOOD_STUDIED,
	CAT_PET_MOOD_PLAYED
} CAT_pet_mood;

typedef struct CAT_pet
{
	CAT_pet_status status;
	CAT_pet_mood mood;

	float vigour;
	float focus;
	float spirit;
	float delta_vigour;
	float delta_focus;
	float delta_spirit;

	int walk_timer_id;
	CAT_vec2 targ;
	CAT_vec2 pos;
	CAT_vec2 vel;
} CAT_pet;
CAT_pet pet;

void CAT_pet_init()
{
	pet.status = CAT_PET_STATUS_IDLE;
	pet.mood = CAT_PET_MOOD_DEFAULT;

	pet.vigour = 12;
	pet.focus = 12;
	pet.spirit = 12;
	pet.delta_vigour = -1;
	pet.delta_focus = -1;
	pet.delta_spirit = -1;

	pet.walk_timer_id = CAT_timer_init(3.0f);
	pet.targ = (CAT_vec2) {-1, -1};
	pet.vel = (CAT_vec2) {0, 0};
	pet.pos = (CAT_vec2) {120, 200};
}

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
		pet.mood = CAT_PET_STATUS_IDLE;
		return true;
	}
	else
	{
		pet.vel = CAT_vec2_mul(line, 48.0f/dist);
		pet.pos = CAT_vec2_add(pet.pos, CAT_vec2_mul(pet.vel, simulator.delta_time));
		pet.mood = CAT_PET_STATUS_WALK;
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
// ROOM

typedef struct CAT_room
{
	CAT_ivec2 min;
	CAT_ivec2 max;
	CAT_ivec2 cursor;

	void (*buttons[5])();
	int selector;
} CAT_room;
CAT_room room;

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

void CAT_room_feed_button()
{
	mode = CAT_MODE_FEED;
}
void CAT_room_study_button()
{
	mode = CAT_MODE_STUDY;
}
void CAT_room_play_button()
{
	mode = CAT_MODE_PLAY;
}
void CAT_room_deco_button()
{
	mode = CAT_MODE_DECO;
}
void CAT_room_menu_button()
{
	screen = CAT_SCREEN_MENU;
}

void CAT_room_init()
{
	room.min = (CAT_ivec2) {0, 6};
	room.max = (CAT_ivec2) {14, 16};
	room.cursor = room.min;

	room.buttons[0] = CAT_room_feed_button;
	room.buttons[1] = CAT_room_study_button;
	room.buttons[2] = CAT_room_play_button;
	room.buttons[3] = CAT_room_deco_button;
	room.buttons[4] = CAT_room_menu_button;
	room.selector = 0;
}


//////////////////////////////////////////////////////////////////////////
// FEED

typedef struct CAT_feed_state
{
	CAT_ivec2 target;
	bool committed;
} CAT_feed_state;
CAT_feed_state feed_state;

void CAT_feed_item(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	pet.vigour += item->data.food_data.d_v;
	pet.focus += item->data.food_data.d_f;
	pet.spirit += item->data.food_data.d_s;
	pet.delta_vigour += item->data.food_data.dd_v;
	pet.delta_focus += item->data.food_data.dd_f;
	pet.delta_spirit += item->data.food_data.dd_s;
}

void CAT_feed_state_init()
{
	feed_state.committed = false;
}

//////////////////////////////////////////////////////////////////////////
// STUDY

typedef struct CAT_study_state
{
	CAT_ivec2 target;
	bool committed;
} CAT_study_state;
CAT_study_state study_state;

void CAT_study_state_init()
{
	study_state.committed = false;
}


//////////////////////////////////////////////////////////////////////////
// PLAY

typedef struct CAT_play_state
{
	CAT_ivec2 target;
	bool committed;
} CAT_play_state;
CAT_play_state play_state;

void CAT_play_state_init()
{
	play_state.committed = false;
}


//////////////////////////////////////////////////////////////////////////
// DECO

typedef struct CAT_deco_state
{
	int props[CAT_MAX_PROP_COUNT];
	CAT_ivec2 places[CAT_MAX_PROP_COUNT];
	int prop_count;
} CAT_deco_state;
CAT_deco_state deco_state;

bool CAT_place_prop(int prop_id, CAT_ivec2 place)
{
	if(deco_state.prop_count >= CAT_MAX_PROP_COUNT)
		return false;

	CAT_item* prop = CAT_item_get(prop_id);
	if(!CAT_test_contain(room.min, room.max, place, CAT_ivec2_add(place, prop->data.prop_data.shape)))
		return false;

	int idx = deco_state.prop_count;
	deco_state.prop_count += 1;
	deco_state.props[idx] = prop_id;
	deco_state.places[idx] = place;
	return true;
}

void CAT_deco_state_init()
{
	deco_state.prop_count = 0;
}


//////////////////////////////////////////////////////////////////////////
// ROOM LOGIC

void CAT_room_logic()
{
	switch(mode)
	{
		case CAT_MODE_DEFAULT:
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
				pet.mood = CAT_PET_MOOD_PET;	
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
			}
			break;
		}
		case CAT_MODE_FEED:
		{
			CAT_room_move_cursor();

			if(CAT_input_pressed(CAT_BUTTON_A) && !feed_state.committed)
			{
				pet.targ = (CAT_vec2) {room.cursor.x * 16, room.cursor.y * 16};
				feed_state.committed = true;
				pet.status = CAT_PET_STATUS_WALK;
			}
			
			if(feed_state.committed && CAT_pet_seek())
			{
				feed_state.committed = false;
				pet.status = CAT_PET_STATUS_IDLE;
				mode = CAT_MODE_DEFAULT;
			}
			break;
		}
		case CAT_MODE_STUDY:
		{
			CAT_room_move_cursor();

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
			}
			break;
		}
		case CAT_MODE_PLAY:
		{
			CAT_room_move_cursor();

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
			}
			break;
		}
		case CAT_MODE_DECO:
		{
			CAT_room_move_cursor();

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
			}
				
			if(CAT_input_pressed(CAT_BUTTON_B))
			{
				mode = CAT_MODE_DEFAULT;
			}
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// MENU LOGIC

typedef struct CAT_menu_state
{
	const char* items[3];
	int idx;
} CAT_menu_state;
CAT_menu_state menu_state;

void CAT_menu_init()
{
	menu_state.items[0] = "STATS";
	menu_state.items[1] = "BAG";
	menu_state.items[2] = "BACK";
	menu_state.idx = 0;
}

void CAT_menu_logic()
{
	if(CAT_input_pulse(CAT_BUTTON_UP))
		menu_state.idx -= 1;
	if(CAT_input_pulse(CAT_BUTTON_DOWN))
		menu_state.idx += 1;
	menu_state.idx = clamp(menu_state.idx, 0, 2);

	if(CAT_input_pressed(CAT_BUTTON_A))
	{
		const char* selector = menu_state.items[menu_state.idx];
		if(strcmp(selector, "STATS") == 0)
			screen = CAT_SCREEN_STATS;
		if(strcmp(selector, "BAG") == 0)
			screen = CAT_SCREEN_BAG;
		if(strcmp(selector, "BACK") == 0)
			screen = CAT_SCREEN_ROOM;
	}

	if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
		screen = CAT_SCREEN_ROOM;
}


////////////////////////////////////////////////////////////////////
// STATS LOGIC

void CAT_stats_logic()
{
	if(CAT_input_pressed(CAT_BUTTON_B))
		screen = CAT_SCREEN_MENU;
	if(CAT_input_pressed(CAT_BUTTON_START))
		screen = CAT_SCREEN_ROOM;
}


//////////////////////////////////////////////////////////////////////////
// BAG LOGIC

typedef struct CAT_bag_state
{
	int window;
	int base;
	int idx;
} CAT_bag_state;
CAT_bag_state bag_state;

void CAT_bag_init()
{
	bag_state.window = 9;
	bag_state.base = 0;
	bag_state.idx = 0;
}

void CAT_bag_logic()
{
	int dir = 0;
	int limit = bag_state.idx;
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
	for(int i = bag_state.idx+dir; i != limit; i += dir)
	{
		if(CAT_bag_count(i) > 0)
		{
			bag_state.idx = i;
			break;
		}
	}

	if(CAT_input_pressed(CAT_BUTTON_B))
		screen = CAT_SCREEN_MENU;
	if(CAT_input_pressed(CAT_BUTTON_START))
		screen = CAT_SCREEN_ROOM;
}


void CAT_logic()
{
	switch(screen)
	{
		case CAT_SCREEN_ROOM:
			CAT_room_logic();
			break;
		case CAT_SCREEN_MENU:
			CAT_menu_logic();
			break;
		case CAT_SCREEN_STATS:
			CAT_stats_logic();	
			break;
		case CAT_SCREEN_BAG:
			CAT_bag_logic();
			break;
	}
}

void CAT_render()
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
			
			for(int i = 0; i < deco_state.prop_count; i++)
			{
				int prop_id = deco_state.props[i];
				CAT_item* prop = CAT_item_get(prop_id);
				CAT_ivec2 place = deco_state.places[i];

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
				case CAT_PET_STATUS_MOOD:
				{
					CAT_anim_queue_add(idle_anim_id, 2, pet.pos.x, pet.pos.y, pet_mode);
					switch(pet.mood)
					{
						case CAT_PET_MOOD_FED:
						{
							int x_off = (pet_mode & CAT_DRAW_MODE_REFLECT_X) > 0 ? 16 : -16;
							CAT_anim_queue_add(fed_anim_id, 3, pet.pos.x + x_off, pet.pos.y - 48, pet_mode);
							break;
						}
						case CAT_PET_MOOD_STUDIED:
						{
							int x_off = (pet_mode & CAT_DRAW_MODE_REFLECT_X) > 0 ? 16 : -16;
							CAT_anim_queue_add(studied_anim_id, 3, pet.pos.x + x_off, pet.pos.y - 48, pet_mode);
							break;
						}
						case CAT_PET_MOOD_PLAYED:
						{
							int x_off = (pet_mode & CAT_DRAW_MODE_REFLECT_X) > 0 ? 16 : -16;
							CAT_anim_queue_add(played_anim_id, 3, pet.pos.x + x_off, pet.pos.y - 48, pet_mode);
							break;
						}
						case CAT_PET_MOOD_PET:
						{
							int x_off = (pet_mode & CAT_DRAW_MODE_REFLECT_X) > 0 ? 16 : -16;
							CAT_anim_queue_add(fed_anim_id, 3, pet.pos.x + x_off, pet.pos.y - 48, pet_mode);
							break;
						}
					}
					break;
				}
				case CAT_PET_STATUS_DEATH:
					CAT_anim_queue_add(death_anim_id, 2, pet.pos.x, pet.pos.y, pet_mode);
					break;
			}		

			CAT_draw_queue_add(feed_button_sprite_id, 3, 8, 280, CAT_DRAW_MODE_DEFAULT); 
			CAT_draw_queue_add(study_button_sprite_id, 3, 56, 280, CAT_DRAW_MODE_DEFAULT); 
			CAT_draw_queue_add(play_button_sprite_id, 3, 104, 280, CAT_DRAW_MODE_DEFAULT);
			CAT_draw_queue_add(study_button_sprite_id, 3, 152, 280, CAT_DRAW_MODE_DEFAULT);
			CAT_draw_queue_add(study_button_sprite_id, 3, 200, 280, CAT_DRAW_MODE_DEFAULT);

			switch(mode)
			{
				case CAT_MODE_DEFAULT:
					CAT_draw_queue_add(ring_hl_sprite_id, 4, 8+48*room.selector, 280, CAT_DRAW_MODE_DEFAULT);
					break;
				case CAT_MODE_FEED:
				case CAT_MODE_STUDY:
				case CAT_MODE_PLAY:
				case CAT_MODE_DECO:
					CAT_draw_queue_add(cursor_sprite_id[0], 3, room.cursor.x * 16, room.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
			}
			
			CAT_anim_queue_submit();
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
				CAT_gui_text(menu_state.items[i]);

				if(i == menu_state.idx)
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
			for(int i = 0; i < bag_state.window; i++)
			{
				int item_id = bag_state.base + i;
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

				if(item_id == bag_state.idx)
				{
					CAT_gui_image(select_sprite_id);
				}
			}

			break;
		}
	}
}

int main(int argc, char** argv)
{
	rand_init();
	CAT_simulator_init();
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

	CAT_feed_state_init();
	CAT_study_state_init();
	CAT_play_state_init();
	CAT_deco_state_init();

	CAT_menu_init();
	CAT_bag_init();
	
	screen = CAT_SCREEN_ROOM;
	mode = CAT_MODE_DEFAULT;

	while(CAT_get_battery_pct() > 0)
	{
		CAT_simulator_tick();
		CAT_input_tick();

		CAT_logic();
		CAT_render();

		CAT_LCD_post(spriter.frame);
	}
	
	CAT_spriter_cleanup();
	CAT_atlas_cleanup();
	CAT_simulator_cleanup();
	return 0;
}
