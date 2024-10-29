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

#define CAT_MAX_PROP_COUNT 210

typedef enum CAT_game_mode
{
	CAT_GAME_MODE_ROOM,
	CAT_GAME_MODE_MENU,
	CAT_GAME_MODE_STATS,
	CAT_GAME_MODE_BAG
} CAT_game_mode;
CAT_game_mode game_mode;

typedef enum CAT_pet_status
{
	CAT_PET_STATUS_IDLE,
	CAT_PET_STATUS_WALK,
	CAT_PET_STATUS_FEED,
	CAT_PET_STATUS_STUDY,
	CAT_PET_STATUS_PLAY,
	CAT_PET_STATUS_PET,
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

	CAT_vec2 pos;
	CAT_vec2 targ;
	
	float stat_timer;
	float move_timer;
	float mood_timer;
} CAT_pet;
CAT_pet pet;

void CAT_pet_init()
{
	pet.status = CAT_PET_STATUS_IDLE;

	pet.vigour = 12;
	pet.focus = 12;
	pet.spirit = 12;
	pet.delta_vigour = -1;
	pet.delta_focus = -1;
	pet.delta_spirit = -1;

	pet.pos = (CAT_vec2) {120, 200};
	pet.targ = (CAT_vec2) {120, 200};

	pet.stat_timer = 0;
	pet.move_timer = 0;
	pet.mood_timer = 0;
}

void CAT_pet_stat()
{
	pet.vigour += pet.delta_vigour;
	pet.focus += pet.delta_focus;
	pet.spirit += pet.delta_spirit;
	pet.stat_timer = 0;
	if(pet.vigour <= 0 || pet.focus <= 0 || pet.spirit <= 0)
		pet.status = CAT_PET_STATUS_DEATH;
}

void CAT_pet_feed(int item_id)
{
	CAT_item* item = CAT_item_get(item_id);
	pet.vigour += item->data.food_data.d_v;
	pet.focus += item->data.food_data.d_f;
	pet.spirit += item->data.food_data.d_s;
	pet.delta_vigour += item->data.food_data.dd_v;
	pet.delta_focus += item->data.food_data.dd_f;
	pet.delta_spirit += item->data.food_data.dd_s;
	pet.status = CAT_PET_STATUS_FEED;
}

typedef struct CAT_room
{
	CAT_ivec2 min;
	CAT_ivec2 max;
	int props[CAT_MAX_PROP_COUNT];
	CAT_ivec2 places[CAT_MAX_PROP_COUNT];
	int prop_count;
} CAT_room;
CAT_room room;

void CAT_room_init()
{
	room.min = (CAT_ivec2) {0, 7};
	room.max = (CAT_ivec2) {14, 16};
	room.prop_count = 0;
}

bool CAT_place_prop(int prop_id, CAT_ivec2 place)
{
	if(room.prop_count >= CAT_MAX_PROP_COUNT)
		return false;

	CAT_item* prop = CAT_item_get(prop_id);
	if(!CAT_test_contain(room.min, room.max, place, CAT_ivec2_add(place, prop->data.prop_data.shape)))
		return false;

	room.props[room.prop_count] = prop_id;
	room.places[room.prop_count] = place;
	room.prop_count += 1;
	return true;
}

CAT_vec2 bauble_pos = (CAT_vec2) {-1.0f, -1.0f}; 
CAT_vec2 bauble_targ = (CAT_vec2) {-1.0f, -1.0f};

typedef struct CAT_room_gui
{
	int mode;
	CAT_ivec2 cursor;
	int selection;
} CAT_room_gui;
CAT_room_gui room_gui =
{
	0, (CAT_ivec2) {0, 7}, 0
};

void CAT_room_gui_logic()
{
	if(room_gui.mode == 0)
	{
		if(CAT_input_pulse(CAT_BUTTON_UP))
			room_gui.cursor.y -= 1;
		if(CAT_input_pulse(CAT_BUTTON_RIGHT))
			room_gui.cursor.x += 1;
		if(CAT_input_pulse(CAT_BUTTON_DOWN))
			room_gui.cursor.y += 1;
		if(CAT_input_pulse(CAT_BUTTON_LEFT))
			room_gui.cursor.x -= 1;
		room_gui.cursor.x = clamp(room_gui.cursor.x, room.min.x, room.max.x);
		if(room_gui.cursor.y > room.max.y)
		{
			room_gui.selection = clamp(room_gui.cursor.x / 5, 0, 2);
			room_gui.mode = 1;
		}
		else if(room_gui.cursor.y < room.min.y)
		{
			room_gui.selection = clamp(3 + room_gui.cursor.x / 5, 3, 5);
			room_gui.mode = 1;
		}
	}
	else if(room_gui.mode == 1)
	{
		if(room_gui.selection < 3)
		{
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				room_gui.selection += 1;
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				room_gui.selection -= 1;
			room_gui.selection = clamp(room_gui.selection, 0, 2);
			if(CAT_input_pressed(CAT_BUTTON_UP))
			{
				room_gui.cursor.y -= 1;
				room_gui.cursor.x = 2 + 5 * room_gui.selection;
				room_gui.mode = 0;
			}

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				switch(room_gui.selection)
				{
					case 0:
						CAT_pet_feed(seed_item_id);
						bauble_targ = (CAT_vec2) {rand_float(room.min.x, room.max.x), rand_float(room.min.y, room.max.y)};
						break;
					case 1:
						pet.status = CAT_PET_STATUS_STUDY;
						break;
					case 2:
						pet.status = CAT_PET_STATUS_PLAY;
						break;
				}
			}
		}
		else if(room_gui.selection >= 3)
		{
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				room_gui.selection += 1;
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				room_gui.selection -= 1;
			room_gui.selection = clamp(room_gui.selection, 3, 5);
			if(CAT_input_pressed(CAT_BUTTON_DOWN))
			{
				room_gui.cursor.y += 1;
				room_gui.cursor.x = 2 + 5 * (room_gui.selection-3);
				room_gui.mode = 0;
			}
		}
	}

	if(CAT_input_touch(pet.pos.x, pet.pos.y - 16, 8))
	{
		pet.status = CAT_PET_STATUS_PET;	
	}

	if(CAT_input_pressed(CAT_BUTTON_START))
	{
		game_mode = CAT_GAME_MODE_MENU;
	}
}

typedef struct CAT_menu_gui
{
	const char* items[3];
	int idx;
} CAT_menu_gui;
CAT_menu_gui menu_gui =
{
	{"STATS", "BAG", "BACK"},
	0
};

void CAT_menu_gui_logic()
{
	if(CAT_input_pulse(CAT_BUTTON_UP))
		menu_gui.idx -= 1;
	if(CAT_input_pulse(CAT_BUTTON_DOWN))
		menu_gui.idx += 1;
	menu_gui.idx = clamp(menu_gui.idx, 0, 2);

	if(CAT_input_pressed(CAT_BUTTON_A))
	{
		const char* selection = menu_gui.items[menu_gui.idx];
		if(strcmp(selection, "STATS") == 0)
			game_mode = CAT_GAME_MODE_STATS;
		if(strcmp(selection, "BAG") == 0)
			game_mode = CAT_GAME_MODE_BAG;
		if(strcmp(selection, "BACK") == 0)
			game_mode = CAT_GAME_MODE_ROOM;
	}

	if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
		game_mode = CAT_GAME_MODE_ROOM;
}

void CAT_stats_gui_logic()
{
	if(CAT_input_pressed(CAT_BUTTON_B))
		game_mode = CAT_GAME_MODE_MENU;
	if(CAT_input_pressed(CAT_BUTTON_START))
		game_mode = CAT_GAME_MODE_ROOM;
}

typedef struct CAT_bag_gui
{
	int window;
	int base;
	int idx;
} CAT_bag_gui;
CAT_bag_gui bag_gui =
{
	9, 0, 0
};

void CAT_bag_gui_logic()
{
	int dir = 0;
	int limit = bag_gui.idx;
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
	for(int i = bag_gui.idx+dir; i != limit; i += dir)
	{
		if(CAT_bag_count(i) > 0)
		{
			bag_gui.idx = i;
			break;
		}
	}

	if(CAT_input_pressed(CAT_BUTTON_B))
		game_mode = CAT_GAME_MODE_MENU;
	if(CAT_input_pressed(CAT_BUTTON_START))
		game_mode = CAT_GAME_MODE_ROOM;
}


void CAT_logic()
{
	switch(game_mode)
	{
		case CAT_GAME_MODE_ROOM:
		{
			pet.stat_timer += simulator.delta_time;
			if(pet.stat_timer >= 3)
			{
				CAT_pet_stat();
				pet.stat_timer = 0;
			}

			switch(pet.status)
			{
				case CAT_PET_STATUS_IDLE:
				{
					pet.move_timer += simulator.delta_time;
					if(pet.move_timer >= 1)
					{
						pet.targ = (CAT_vec2) {rand_float(16, 224), rand_float(112, 304)};
						pet.status = CAT_PET_STATUS_WALK;
						pet.move_timer = 0;
					}
					break;
				}
				case CAT_PET_STATUS_WALK:
				{
					float speed = 48;
					if(pet.vigour <= 6)
						speed -= 10 - pet.vigour;
					if(pet.spirit <= 6)
						speed -= 10 - pet.spirit;

					CAT_vec2 line = CAT_vec2_sub(pet.targ, pet.pos);
					float dist = CAT_vec2_mag(line);
					if(dist > 8.0f)
					{
						CAT_vec2 dir = CAT_vec2_mul(line, 1.0f/dist);
						pet.pos = CAT_vec2_add(pet.pos, CAT_vec2_mul(dir, speed*simulator.delta_time));
					}
					else
					{
						pet.status = CAT_PET_STATUS_IDLE;
					}
					break;
				}
				case CAT_PET_STATUS_FEED:
				case CAT_PET_STATUS_STUDY:
				case CAT_PET_STATUS_PLAY:
				case CAT_PET_STATUS_PET:
					pet.mood_timer += simulator.delta_time;
					if(pet.mood_timer >= 2.0f)
					{
						pet.status = CAT_PET_STATUS_IDLE;
						pet.mood_timer = 0.0f;
					}
					break;
			}

			CAT_room_gui_logic();
			break;
		}
		case CAT_GAME_MODE_MENU:
			CAT_menu_gui_logic();
			break;
		case CAT_GAME_MODE_STATS:
			CAT_stats_gui_logic();	
			break;
		case CAT_GAME_MODE_BAG:
			CAT_bag_gui_logic();
			break;
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
	
	CAT_item_table_init();
	CAT_item_mass_define();
	
	CAT_gui_init(panel_sprite_id, glyph_sprite_id);

	game_mode = CAT_GAME_MODE_ROOM;
	CAT_room_init();
	CAT_pet_init();

	while(CAT_get_battery_pct() > 0)
	{
		CAT_simulator_tick();
		CAT_input_tick();
		CAT_logic();

		switch(game_mode)
		{
			case CAT_GAME_MODE_ROOM:
			{
				CAT_draw_tiles(0, 4, wall_sprite_id[0]);
				CAT_draw_tiles(4, 1, wall_sprite_id[1]);
				CAT_draw_tiles(5, 1, wall_sprite_id[2]);
				CAT_draw_tiles(6, 1, floor_sprite_id[2]);
				CAT_draw_tiles(7, 10, floor_sprite_id[0]);
				CAT_draw_tiles(17, 3, floor_sprite_id[1]);

				CAT_anim_queue_add(vending_anim_id, 2, 6, 112, CAT_DRAW_MODE_BOTTOM);
				CAT_anim_queue_add(pot_anim_id, 2, 120, 112, CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X);
				CAT_draw_queue_add(device_sprite_id, 2, 192, 112, CAT_DRAW_MODE_BOTTOM);
				
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
					case CAT_PET_STATUS_DEATH:
						CAT_anim_queue_add(death_anim_id, 2, pet.pos.x, pet.pos.y, pet_mode);
						break;
					case CAT_PET_STATUS_FEED:
					{
						int x_off = (pet_mode & CAT_DRAW_MODE_REFLECT_X) > 0 ? 16 : -16;
						CAT_anim_queue_add(idle_anim_id, 2, pet.pos.x, pet.pos.y, pet_mode);
						CAT_anim_queue_add(fed_anim_id, 3, pet.pos.x + x_off, pet.pos.y - 48, pet_mode);
						break;
					}
					case CAT_PET_STATUS_STUDY:
					{
						int x_off = (pet_mode & CAT_DRAW_MODE_REFLECT_X) > 0 ? 16 : -16;
						CAT_anim_queue_add(idle_anim_id, 2, pet.pos.x, pet.pos.y, pet_mode);
						CAT_anim_queue_add(studied_anim_id, 3, pet.pos.x + x_off, pet.pos.y - 48, pet_mode);
						break;
					}
					case CAT_PET_STATUS_PLAY:
					{
						int x_off = (pet_mode & CAT_DRAW_MODE_REFLECT_X) > 0 ? 16 : -16;
						CAT_anim_queue_add(idle_anim_id, 2, pet.pos.x, pet.pos.y, pet_mode);
						CAT_anim_queue_add(played_anim_id, 3, pet.pos.x + x_off, pet.pos.y - 48, pet_mode);
						break;
					}
					case CAT_PET_STATUS_PET:
					{
						int x_off = (pet_mode & CAT_DRAW_MODE_REFLECT_X) > 0 ? 16 : -16;
						CAT_anim_queue_add(idle_anim_id, 2, pet.pos.x, pet.pos.y, pet_mode);
						CAT_anim_queue_add(fed_anim_id, 3, pet.pos.x + x_off, pet.pos.y - 48, pet_mode);
						break;
					}
				}
				
				if(room_gui.mode == 0)
					CAT_draw_queue_add(cursor_sprite_id[0], 3, room_gui.cursor.x * 16, room_gui.cursor.y * 16, CAT_DRAW_MODE_DEFAULT);
				CAT_draw_queue_add(feed_button_sprite_id, 3, 40, 294, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y); 
				CAT_draw_queue_add(study_button_sprite_id, 3, 120, 294, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y); 
				CAT_draw_queue_add(play_button_sprite_id, 3, 200, 294, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y); 
				if(room_gui.mode == 1)
				{
					if(room_gui.selection == 0)
						CAT_draw_queue_add(ring_hl_sprite_id, 4, 40, 294, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y);
					if(room_gui.selection == 1)
						CAT_draw_queue_add(ring_hl_sprite_id, 4, 120, 294, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y);
					if(room_gui.selection == 2)
						CAT_draw_queue_add(ring_hl_sprite_id, 4, 200, 294, CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y);
				}

				CAT_anim_queue_submit();
				CAT_draw_queue_submit();
				
				break;
			}
			case CAT_GAME_MODE_MENU:
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
					CAT_gui_text(menu_gui.items[i]);

					if(i == menu_gui.idx)
						CAT_gui_image(select_sprite_id);

					CAT_gui_line_break();
				}
				
				break;
			}
			case CAT_GAME_MODE_STATS:
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
			case CAT_GAME_MODE_BAG:
			{
				CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
				CAT_gui_text("BAG");
				CAT_gui_image(b_sprite_id);
				CAT_gui_image(exit_sprite_id);

				CAT_gui_panel((CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});
				for(int i = 0; i < bag_gui.window; i++)
				{
					int item_id = bag_gui.base + i;
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

					if(item_id == bag_gui.idx)
					{
						CAT_gui_image(select_sprite_id);
					}
				}

				break;
			}
		}

		CAT_LCD_post(spriter.frame);
	}
	
	CAT_spriter_cleanup();
	CAT_atlas_cleanup();
	CAT_simulator_cleanup();
	return 0;
}
