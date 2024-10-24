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

typedef struct CAT_pet
{
	int age;
	int evolution;

	float vigour;
	float focus;
	float spirit;

	float delta_vigour;
	float delta_focus;
	float delta_spirit;

	CAT_vec2 position;
	CAT_vec2 target;
	float move_timer;
} CAT_pet;
CAT_pet pet;

void CAT_pet_init()
{
	pet.age = 0;
	pet.evolution = 0;
	pet.vigour = 12;
	pet.focus = 12;
	pet.spirit = 12;
	pet.delta_vigour = -1;
	pet.delta_focus = -1;
	pet.delta_spirit = -1;
	pet.position = (CAT_vec2) {120, 200};
	pet.target = (CAT_vec2) {120, 200};
	pet.move_timer = 0;
}

typedef struct CAT_room
{
	int props[210];
	CAT_ivec2 places[210];
	int prop_count;
	CAT_ivec2 cursor;
	int prop_id;
} CAT_room;
CAT_room room;

void CAT_room_init()
{
	room.prop_count = 0;
	room.cursor = (CAT_ivec2) {0, 96};
	room.prop_id = CAT_bag_seek(0, CAT_ITEM_TYPE_PROP);
}

void CAT_place_prop(int prop, CAT_ivec2 place)
{
	if(room.prop_count >= 210)
	{
		return;
	}

	room.props[room.prop_count] = prop;
	room.places[room.prop_count] = place;
	room.prop_count += 1;
}

typedef enum CAT_game_mode
{
	CAT_GAME_MODE_ROOM,
	CAT_GAME_MODE_MENU,
	CAT_GAME_MODE_STATS,
	CAT_GAME_MODE_BAG
} CAT_game_mode;
CAT_game_mode game_mode;

const char* menu_items[3] =
{
	"STATS",
	"BAG",
	"BACK"
};
CAT_menu main_menu;
CAT_menu bag_menu;

void CAT_logic()
{
	switch(game_mode)
	{
		case CAT_GAME_MODE_ROOM:
		{
			CAT_vec2 beeline = CAT_vec2_sub(pet.target, pet.position);
			float dist = CAT_vec2_mag(beeline);
			if(dist > 8.0f)
			{
				CAT_vec2 dir = CAT_vec2_mul(beeline, 1.0f/dist);
				pet.position = CAT_vec2_add(pet.position, CAT_vec2_mul(dir, 48*simulator.delta_time));
			}
			else
			{
				pet.move_timer += simulator.delta_time;
				if(pet.move_timer >= 1)
				{
					pet.move_timer = 0;
					pet.target = (CAT_vec2) {rand_float(16, 224), rand_float(112, 304)};
				}
			}

			if(CAT_input_pressed(CAT_BUTTON_UP))
				room.cursor.y -= 16;
			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
				room.cursor.x += 16;
			if(CAT_input_pressed(CAT_BUTTON_DOWN))
				room.cursor.y += 16;
			if(CAT_input_pressed(CAT_BUTTON_LEFT))
				room.cursor.x -= 16;

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				if(room.prop_id != -1)
				{
					CAT_place_prop(room.prop_id, room.cursor);
					CAT_bag_remove(room.prop_id);
					if(CAT_bag_count(room.prop_id) <= 0)
						room.prop_id = CAT_bag_seek(room.prop_id+1, CAT_ITEM_TYPE_PROP);
				}
			}

			if(CAT_input_pressed(CAT_BUTTON_START))
			{
				game_mode = CAT_GAME_MODE_MENU;
			}
			break;
		}
		case CAT_GAME_MODE_MENU:
		{
			if(CAT_input_pressed(CAT_BUTTON_UP))
			{
				CAT_menu_shift(&main_menu, -1);
			}	
			if(CAT_input_pressed(CAT_BUTTON_DOWN))
			{
				CAT_menu_shift(&main_menu, 1);
			}

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				const char* selection = menu_items[main_menu.idx];
				if(strcmp(selection, "STATS") == 0)
				{
					game_mode = CAT_GAME_MODE_STATS;
				}
				if(strcmp(selection, "BAG") == 0)
				{
					game_mode = CAT_GAME_MODE_BAG;
				}
				if(strcmp(selection, "BACK") == 0)
				{
					game_mode = CAT_GAME_MODE_ROOM;
				}
			}

			if(CAT_input_pressed(CAT_BUTTON_B) || CAT_input_pressed(CAT_BUTTON_START))
			{
				game_mode = CAT_GAME_MODE_ROOM;
			}
			break;
		}
		case CAT_GAME_MODE_STATS:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
			{
				game_mode = CAT_GAME_MODE_MENU;
			}
			if(CAT_input_pressed(CAT_BUTTON_START))
			{
				game_mode = CAT_GAME_MODE_ROOM;
			}
			break;
		}
		case CAT_GAME_MODE_BAG:
		{
			if(CAT_input_pressed(CAT_BUTTON_UP))
				CAT_menu_shift(&bag_menu, -1);
			if(CAT_input_pressed(CAT_BUTTON_DOWN))
				CAT_menu_shift(&bag_menu, 1);

			if(CAT_input_pressed(CAT_BUTTON_B))
			{
				game_mode = CAT_GAME_MODE_MENU;
			}
			if(CAT_input_pressed(CAT_BUTTON_START))
			{
				game_mode = CAT_GAME_MODE_ROOM;
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
	CAT_animator_init();
	
	int wall_sprite[3];
	int floor_sprite[3];
	for(int i = 0; i < 3; i++)
	{
		wall_sprite[i] = CAT_atlas_add(16*i, 0, 16, 16);
		floor_sprite[i] = CAT_atlas_add(16*i, 16, 16, 16);
	}
	int pet_sprite[13];
	for(int i = 0; i < 13; i++)
	{
		pet_sprite[i] = CAT_atlas_add(64*i, 544, 64, 48);
	}
	int chair_sprite[4];
	for(int i = 0; i < 4; i++)
	{
		chair_sprite[i] = CAT_atlas_add(32*i, 48, 32, 48);
	}
	int table_sprite = CAT_atlas_add(128, 48, 64, 48);
	int coffee_sprite[2];
	for(int i = 0; i < 2; i++)
	{
		coffee_sprite[i] = CAT_atlas_add(192+32*i, 48, 32, 48);
	}
	int device_sprite = CAT_atlas_add(256, 48, 48, 48);
	int vending_sprite[13];
	for(int i = 0; i < 13; i++)
	{
		vending_sprite[i] = CAT_atlas_add(64*i, 448, 64, 96);
	}

	int cursor_sprite[4];
	for(int i = 0; i < 4; i++)
	{
		cursor_sprite[i] = CAT_atlas_add(16*i, 336, 16, 16);
	}
	int glyph_sprite[91];
	for(int i = 0; i < 91; i++)
	{
		glyph_sprite[i] = CAT_atlas_add(8*i, 592, 8, 12);
	}
	int panel_sprite[9];
	for(int i = 0; i < 3; i++)
	{
		panel_sprite[0+i] = CAT_atlas_add(48+16*i, 0, 16, 16);
		panel_sprite[3+i] = CAT_atlas_add(48+16*i, 16, 16, 16);
		panel_sprite[6+i] = CAT_atlas_add(48+16*i, 32, 16, 16);
	}
	int a_sprite = CAT_atlas_add(96, 0, 16, 16);
	int b_sprite = CAT_atlas_add(112, 0, 16, 16);
	int enter_sprite = CAT_atlas_add(96, 16, 16, 16);
	int exit_sprite = CAT_atlas_add(112, 16, 16, 16);
	int select_sprite = CAT_atlas_add(96, 32, 16, 16);
	int arrow_sprite = CAT_atlas_add(112, 32, 16, 16);
	int atlas_sprite = CAT_atlas_add(0, 0, 240, 320);

	int item_icon = CAT_atlas_add(192, 304, 16, 16);

	CAT_anim idle_anim;
	CAT_anim_init(&idle_anim);
	for(int i = 0; i < 2; i++)
	{
		CAT_anim_add(&idle_anim, pet_sprite[i]); 
	}

	CAT_anim walk_anim;
	CAT_anim_init(&walk_anim);
	for(int i = 0; i < 2; i++)
	{
		CAT_anim_add(&walk_anim, pet_sprite[2+i]); 
	}

	CAT_anim mood_anim;
	CAT_anim_init(&mood_anim);
	for(int i = 0; i < 9; i++)
	{
		CAT_anim_add(&mood_anim, pet_sprite[4+i]); 
	}

	CAT_anim chair_anim;
	CAT_anim_init(&chair_anim);
	for(int i = 0; i < 4; i++)
	{
		CAT_anim_add(&chair_anim, chair_sprite[i]); 
	}
	
	CAT_anim coffee_anim;
	CAT_anim_init(&coffee_anim);
	for(int i = 0; i < 2; i++)
	{
		CAT_anim_add(&coffee_anim, coffee_sprite[i]); 
	}
	
	CAT_anim vending_anim;
	CAT_anim_init(&vending_anim);
	for(int i = 0; i < 13; i++)
	{
		CAT_anim_add(&vending_anim, vending_sprite[i]); 
	}

	CAT_anim cursor_anim;
	CAT_anim_init(&cursor_anim);
	for(int i = 0; i < 4; i++)
	{
		CAT_anim_add(&cursor_anim, cursor_sprite[i]); 
	}
	
	CAT_gui gui;
	CAT_gui_init(&gui, panel_sprite, glyph_sprite);
	
	
	CAT_store_init();
	CAT_bag_init();

	CAT_item chair_item;
	CAT_item_init(&chair_item, CAT_ITEM_TYPE_PROP, "Chair", chair_sprite[0], 1);
	CAT_prop_init(&chair_item, &chair_anim, 2, 2);
	int chair_id = CAT_store_add(&chair_item);
	CAT_bag_add(chair_id);

	CAT_item table_item;
	CAT_item_init(&table_item, CAT_ITEM_TYPE_PROP, "Table", table_sprite, 1);
	CAT_prop_init(&table_item, NULL, 4, 2);
	int table_id = CAT_store_add(&table_item);
	CAT_bag_add(table_id);
	
	CAT_item coffee_item;
	CAT_item_init(&coffee_item, CAT_ITEM_TYPE_PROP, "Coffee", coffee_sprite[0], 1);
	CAT_prop_init(&coffee_item, &coffee_anim, 4, 2);
	int coffee_id = CAT_store_add(&coffee_item);
	CAT_bag_add(coffee_id);
	
	CAT_item device_item;
	CAT_item_init(&device_item, CAT_ITEM_TYPE_PROP, "Device", device_sprite, 1);
	CAT_prop_init(&device_item, NULL, 4, 2);
	int device_id = CAT_store_add(&device_item);
	CAT_bag_add(device_id);

	game_mode = CAT_GAME_MODE_ROOM;
	CAT_room_init();
	CAT_pet_init();
	CAT_menu_init(&main_menu, sizeof(menu_items)/sizeof(char*), 9);
	CAT_menu_init(&bag_menu, 256, 9);

	while(CAT_get_battery_pct() > 0)
	{
		CAT_simulator_tick();

		CAT_input_tick(simulator.delta_time);

		CAT_logic();

		switch(game_mode)
		{
			case CAT_GAME_MODE_ROOM:
			{
				CAT_draw_tiles(0, 4, wall_sprite[0]);
				CAT_draw_tiles(4, 1, wall_sprite[1]);
				CAT_draw_tiles(5, 1, wall_sprite[2]);
				CAT_draw_tiles(6, 1, floor_sprite[2]);
				CAT_draw_tiles(7, 10, floor_sprite[0]);
				CAT_draw_tiles(17, 3, floor_sprite[1]);

				CAT_animator_add(&vending_anim);
				CAT_draw_queue_add(CAT_anim_frame(&vending_anim), 2, 0, 112, CAT_DRAW_MODE_BOTTOM);
				CAT_draw_queue_add(vending_sprite[0], 3, 0, 112, CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_WIREFRAME);
				
				for(int i = 0; i < room.prop_count; i++)
				{
					int prop_id = room.props[i];
					CAT_ivec2 place = room.places[i];
					CAT_item* prop = store.table[prop_id];

					CAT_anim* anim = prop->data.prop_data.anim;
					if(anim != NULL)
					{
						CAT_animator_add(anim);
						CAT_draw_queue_add(CAT_anim_frame(anim), 2, place.x, place.y, CAT_DRAW_MODE_BOTTOM);
					}
					else
					{
						CAT_draw_queue_add(prop->sprite, 2, place.x, place.y, CAT_DRAW_MODE_BOTTOM);
					}
				}

				int pet_mode = CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X | (pet.position.x < pet.target.x ? CAT_DRAW_MODE_REFLECT_X : 0);
				CAT_anim* pet_anim = pet.move_timer > 0 ? &idle_anim : &walk_anim;
				CAT_animator_add(pet_anim);
				CAT_draw_queue_add(CAT_anim_frame(pet_anim), 2, pet.position.x, pet.position.y, pet_mode);
				
				CAT_animator_add(&cursor_anim);
				CAT_draw_queue_add(CAT_anim_frame(&cursor_anim), 1, room.cursor.x, room.cursor.y, CAT_DRAW_MODE_DEFAULT);

				if(room.prop_id != -1)
					CAT_draw_queue_add(store.table[room.prop_id]->sprite, 3, 0, 0, CAT_DRAW_MODE_DEFAULT);
					
				CAT_animator_tick(simulator.delta_time);
				CAT_draw_queue_submit();
				
				break;
			}
			case CAT_GAME_MODE_MENU:
			{
				CAT_gui_panel(&gui, (CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
				CAT_gui_text(&gui, "MENU");
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, a_sprite);
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, enter_sprite);
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, b_sprite);
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, exit_sprite);

				CAT_gui_panel(&gui, (CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});  
				for(int i = 0; i < main_menu.length; i++)
				{
					CAT_gui_text(&gui, "#");
					CAT_gui_same_line(&gui);
					CAT_gui_text(&gui, menu_items[i]);

					if(i == main_menu.idx)
					{
						CAT_gui_same_line(&gui);
						CAT_gui_image(&gui, select_sprite);
					}
				}
				
				break;
			}
			case CAT_GAME_MODE_STATS:
			{
				CAT_gui_panel(&gui, (CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
				CAT_gui_text(&gui, "STATS");
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, b_sprite);
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, exit_sprite);

				CAT_gui_panel(&gui, (CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});  
				CAT_gui_image(&gui, pet_sprite[0]); 
				char text[64];
				sprintf(text, "Age: %d", pet.age);
				CAT_gui_text(&gui, text);
				sprintf(text, "Evolution: %d", pet.evolution);
				CAT_gui_text(&gui, text);
				sprintf(text, "Vigour: %.0f", pet.vigour);
				CAT_gui_text(&gui, text);
				sprintf(text, "Focus: %.0f", pet.focus);
				CAT_gui_text(&gui, text);
				sprintf(text, "Spirit: %.0f", pet.spirit);
				CAT_gui_text(&gui, text);
				sprintf(text, "dVigour: %.1f", pet.delta_vigour);
				CAT_gui_text(&gui, text);
				sprintf(text, "dFocus: %.1f", pet.delta_focus);
				CAT_gui_text(&gui, text);
				sprintf(text, "dSpirit: %.1f", pet.delta_spirit);
				CAT_gui_text(&gui, text);

				break;
			}
			case CAT_GAME_MODE_BAG:
			{
				CAT_gui_panel(&gui, (CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
				CAT_gui_text(&gui, "BAG");
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, b_sprite);
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, exit_sprite);

				CAT_gui_panel(&gui, (CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});
				for(int i = 0; i < bag_menu.window; i++)
				{
					int slot = bag_menu.base + i;
					if(slot >= 256 || bag.quantities[slot] <= 0)
						continue;
					CAT_item* item = store.table[i];

					CAT_gui_panel(&gui, (CAT_ivec2) {0, 32+i*32}, (CAT_ivec2) {15, 2});
					CAT_gui_image(&gui, item_icon); 
					CAT_gui_same_line(&gui);
					char text[64];
					sprintf(text, "%s *%d", item->name, bag.quantities[slot]);
					CAT_gui_text(&gui, text);

					if(slot == bag_menu.idx)
					{
						CAT_gui_same_line(&gui);
						CAT_gui_image(&gui, select_sprite);
					}
				}

				break;
			}
		}

		CAT_LCD_post(spriter.frame);
	}

	return 0;
}
