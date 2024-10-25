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
	CAT_anim_table_init();
	CAT_animator_init();
	CAT_sprite_mass_define();
	
	CAT_item_table_init();
	CAT_item_mass_define();
	
	CAT_gui gui;
	CAT_gui_init(&gui, panel_sprite_id, glyph_sprite_id);

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
				CAT_draw_tiles(0, 4, wall_sprite_id[0]);
				CAT_draw_tiles(4, 1, wall_sprite_id[1]);
				CAT_draw_tiles(5, 1, wall_sprite_id[2]);
				CAT_draw_tiles(6, 1, floor_sprite_id[2]);
				CAT_draw_tiles(7, 10, floor_sprite_id[0]);
				CAT_draw_tiles(17, 3, floor_sprite_id[1]);

				CAT_animator_add(vending_anim_id, 2, 0, 112, CAT_DRAW_MODE_BOTTOM);
				
				for(int i = 0; i < room.prop_count; i++)
				{
					int prop_id = room.props[i];
					CAT_ivec2 place = room.places[i];
					CAT_item* prop = CAT_item_get(prop_id);

					int anim_id = prop->data.prop_data.anim_id;
					if(anim_id != -1)
					{
						CAT_animator_add(anim_id, 2, place.x, place.y, CAT_DRAW_MODE_BOTTOM);
					}
					else
					{
						CAT_draw_queue_add(prop->sprite, 2, place.x, place.y, CAT_DRAW_MODE_BOTTOM);
					}
				}

				int pet_mode = CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X | (pet.position.x < pet.target.x ? CAT_DRAW_MODE_REFLECT_X : 0);
				int pet_anim_id = pet.move_timer > 0 ? idle_anim_id : walk_anim_id;
				CAT_animator_add(pet_anim_id, 2, pet.position.x, pet.position.y, pet_mode);
				
				CAT_animator_add(cursor_anim_id, 1, room.cursor.x, room.cursor.y, CAT_DRAW_MODE_DEFAULT);

				if(room.prop_id != -1)
					CAT_draw_queue_add(CAT_item_get(room.prop_id)->sprite, 3, 0, 0, CAT_DRAW_MODE_DEFAULT);
					
				CAT_animator_submit();
				CAT_draw_queue_submit();
				
				break;
			}
			case CAT_GAME_MODE_MENU:
			{
				CAT_gui_panel(&gui, (CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
				CAT_gui_text(&gui, "MENU");
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, a_sprite_id);
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, enter_sprite_id);
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, b_sprite_id);
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, exit_sprite_id);

				CAT_gui_panel(&gui, (CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});  
				for(int i = 0; i < main_menu.length; i++)
				{
					CAT_gui_text(&gui, "#");
					CAT_gui_same_line(&gui);
					CAT_gui_text(&gui, menu_items[i]);

					if(i == main_menu.idx)
					{
						CAT_gui_same_line(&gui);
						CAT_gui_image(&gui, select_sprite_id);
					}
				}
				
				break;
			}
			case CAT_GAME_MODE_STATS:
			{
				CAT_gui_panel(&gui, (CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
				CAT_gui_text(&gui, "STATS");
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, b_sprite_id);
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, exit_sprite_id);

				CAT_gui_panel(&gui, (CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});  
				CAT_gui_image(&gui, pet_sprite_id[0]); 
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
				CAT_gui_image(&gui, b_sprite_id);
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, exit_sprite_id);

				CAT_gui_panel(&gui, (CAT_ivec2) {0, 32}, (CAT_ivec2) {15, 18});
				for(int i = 0; i < bag_menu.window; i++)
				{
					int item_id = bag_menu.base + i;
					if(item_id >= item_table.length)
						continue;
					CAT_item* item = CAT_item_get(item_id);
					if(item->count <= 0)
						continue;

					CAT_gui_panel(&gui, (CAT_ivec2) {0, 32+i*32}, (CAT_ivec2) {15, 2});
					CAT_gui_image(&gui, item_sprite_id); 
					CAT_gui_same_line(&gui);
					char text[64];
					sprintf(text, "%s *%d", item->name, item->count);
					CAT_gui_text(&gui, text);

					if(item_id == bag_menu.idx)
					{
						CAT_gui_same_line(&gui);
						CAT_gui_image(&gui, select_sprite_id);
					}
				}

				break;
			}
		}

		CAT_LCD_post(spriter.frame);
	}

	return 0;
}
