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
} CAT_room;
CAT_room room;

void CAT_room_init()
{
	room.prop_count = 0;
	room.cursor = (CAT_ivec2) {0, 96};
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

CAT_ivec2 room_cursor;

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
				pet.position = CAT_vec2_add(pet.position, CAT_vec2_mul(dir, 32*simulator.delta_time));
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
				CAT_place_prop(0, room.cursor);
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
				CAT_menu_shift(&main_menu, -1);
			if(CAT_input_pressed(CAT_BUTTON_DOWN))
				CAT_menu_shift(&main_menu, 1);

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

	CAT_atlas_init("sprites/test.png");
	CAT_spriter_init();
	CAT_anim_queue_init();
	
	int bg_sprite = CAT_atlas_add((CAT_sprite) {272, 0, 240, 320});
	int pet_sprite[3] =
	{
		CAT_atlas_add((CAT_sprite) {48, 16, 36, 32}),
		CAT_atlas_add((CAT_sprite) {84, 16, 36, 32}),
		CAT_atlas_add((CAT_sprite) {120, 16, 36, 32})
	};
	int chair_sprite = CAT_atlas_add((CAT_sprite) {0, 16, 24, 46});
	int coffee_sprite = CAT_atlas_add((CAT_sprite) {24, 32, 26, 37});
	int cursor_sprite = CAT_atlas_add((CAT_sprite) {48, 0, 16, 16});
	int ascii_sprite[91];
	int alph_idx = 0;
	for(int y = 488; y < 512 && alph_idx < 91; y += 12)
	{
		for(int x = 0; x < 512 && alph_idx < 91; x += 8)
		{
			ascii_sprite[alph_idx] = CAT_atlas_add((CAT_sprite) {x, y, 8, 12});
			alph_idx += 1;
		}
	}
	int gui_sprite[9];
	for(int i = 0; i < 9; i++)
	{
		gui_sprite[i] = CAT_atlas_add((CAT_sprite) {80+i*16, 0, 16, 16});
	}
	int seed_sprite = CAT_atlas_add((CAT_sprite) {160, 16, 24, 24});
	int select_sprite = CAT_atlas_add((CAT_sprite) {192, 16, 16, 16});
	int exit_sprite = CAT_atlas_add((CAT_sprite) {224, 0, 12, 16});
	int a_sprite = CAT_atlas_add((CAT_sprite) {240, 0, 16, 16});
	int b_sprite = CAT_atlas_add((CAT_sprite) {256, 0, 16, 16});
	int enter_sprite = CAT_atlas_add((CAT_sprite) {208, 16, 12, 16});
	int prop_icon = CAT_atlas_add((CAT_sprite) {32, 0, 16, 24});

	CAT_anim bg_anim;
	CAT_anim_init(&bg_anim);
	CAT_anim_add(&bg_anim, bg_sprite);
	CAT_anim idle_anim;
	CAT_anim_init(&idle_anim);
	CAT_anim_add(&idle_anim, pet_sprite[0]);
	CAT_anim_add(&idle_anim, pet_sprite[1]);
	CAT_anim chair_anim;
	CAT_anim_init(&chair_anim);
	CAT_anim_add(&chair_anim, chair_sprite);
	CAT_anim coffee_anim;
	CAT_anim_init(&coffee_anim);
	CAT_anim_add(&coffee_anim, coffee_sprite);
	CAT_anim walk_anim;
	CAT_anim_init(&walk_anim);
	CAT_anim_add(&walk_anim, pet_sprite[0]);
	CAT_anim_add(&walk_anim, pet_sprite[2]);
	CAT_anim cursor_anim;
	CAT_anim_init(&cursor_anim);
	CAT_anim_add(&cursor_anim, cursor_sprite);
	
	CAT_gui gui;
	CAT_gui_init(&gui, gui_sprite, ascii_sprite);
	
	game_mode = CAT_GAME_MODE_ROOM;
	CAT_room_init();
	CAT_pet_init();
	CAT_store_init();
	CAT_bag_init();
	CAT_menu_init(&main_menu, sizeof(menu_items)/sizeof(char*), 9);
	CAT_menu_init(&bag_menu, 256, 9);
	
	CAT_item chair_item;
	CAT_item_init(&chair_item, CAT_ITEM_TYPE_PROP, "Chair", prop_icon, &chair_anim);
	int chair_id = CAT_store_add(&chair_item);

	CAT_item coffee_item;
	CAT_item_init(&coffee_item, CAT_ITEM_TYPE_PROP, "Coffee Maker", prop_icon, &coffee_anim);
	int coffee_id = CAT_store_add(&coffee_item);

	CAT_bag_add(chair_id);
	CAT_bag_add(coffee_id);

	while(CAT_get_battery_pct() > 0)
	{
		CAT_simulator_tick();

		CAT_input_tick(simulator.delta_time);

		CAT_logic();

		switch(game_mode)
		{
			case CAT_GAME_MODE_ROOM:
			{
				int prop_mode = CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X;
				int pet_mode = prop_mode | (pet.position.x < pet.target.x ? CAT_DRAW_MODE_REFLECT_X : 0);

				CAT_anim_cmd bg_cmd;
				CAT_anim_cmd_init(&bg_cmd, &bg_anim, 0, 0, 0, CAT_DRAW_MODE_DEFAULT);
				CAT_anim_queue_add(bg_cmd);
				
				for(int i = 0; i < room.prop_count; i++)
				{
					int prop_id = room.props[i];
					CAT_ivec2 place = room.places[i];
					CAT_item* prop = store.table[prop_id];

					CAT_anim* anim = prop->anim;
					CAT_anim_cmd cmd;
					CAT_anim_cmd_init(&cmd, anim, 2, place.x, place.y, prop_mode);
					CAT_anim_queue_add(cmd);
				}
				
				CAT_anim_cmd pet_cmd;
				CAT_anim_cmd_init
				(
					&pet_cmd, pet.move_timer > 0 ? &idle_anim : &walk_anim,
					2, pet.position.x, pet.position.y,
					pet_mode
				);
				CAT_anim_queue_add(pet_cmd);

				CAT_anim_cmd cursor_cmd;
				CAT_anim_cmd_init(&cursor_cmd, &cursor_anim, 1, room.cursor.x, room.cursor.y, CAT_DRAW_MODE_DEFAULT); 
				CAT_anim_queue_add(cursor_cmd);
			
				CAT_anim_queue_tick(simulator.delta_time);
				CAT_anim_queue_draw();
				
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
					CAT_gui_image(&gui, item->icon); 
					CAT_gui_same_line(&gui);
					CAT_gui_text(&gui, item->name);

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
