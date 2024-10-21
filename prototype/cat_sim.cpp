#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cat_display.h"
#include "cat_texture.h"
#include "cat_debug.h"
#include "cat_math.h"
#include "cat_item.h"
#include "cat_gui.h"

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
} CAT_pet;

typedef struct CAT_room
{
	CAT_item* props[210];
	CAT_ivec2 places[210];
	int prop_count;
} CAT_room;

void CAT_place_prop(CAT_room* room, CAT_item* prop, CAT_ivec2 place)
{
	if(room->prop_count >= 210)
	{
		return;
	}

	room->props[room->prop_count] = prop;
	room->places[room->prop_count] = place;
	room->prop_count += 1;
}

typedef enum CAT_game_mode
{
	CAT_GAME_MODE_ROOM,
	CAT_GAME_MODE_MENU,
	CAT_GAME_MODE_INVENTORY
} CAT_game_mode;

typedef struct CAT_context
{
	CAT_game_mode mode;

	float time;
	float delta_time;

	CAT_vec2 pet_pos;
	CAT_vec2 pet_target;
	float pet_timer;

	CAT_room room;
	CAT_ivec2 cursor;

	CAT_bag bag;

	const char* menu_items[2] =
	{
		"INVENTORY",
		"BACK"
	};
	CAT_menu main_menu;
	
	CAT_menu bag_menu;
} CAT_context;

void CAT_context_init(CAT_context* context)
{
	context->mode = CAT_GAME_MODE_ROOM;

	context->time = glfwGetTime();
	context->delta_time = 0;

	context->pet_pos = {120, 200};
	context->pet_target = {120, 200};
	context->pet_timer = 0;

	context->room.prop_count = 0;
	context->cursor = {0, 96};

	CAT_bag_init(&context->bag);

	CAT_menu_init(&context->main_menu, 2, 9);
	CAT_menu_init(&context->bag_menu, 13, 9);
}

typedef enum CAT_button
{
	CAT_BUTTON_START,
	CAT_BUTTON_SELECT,
	CAT_BUTTON_UP,
	CAT_BUTTON_RIGHT,
	CAT_BUTTON_DOWN,
	CAT_BUTTON_LEFT,
	CAT_BUTTON_A,
	CAT_BUTTON_B,
	CAT_BUTTON_LAST
} CAT_button;

typedef enum CAT_button_state
{
	CAT_BUTTON_STATE_PRESSED = 1,
	CAT_BUTTON_STATE_RELEASED = 2,
	CAT_BUTTON_STATE_EDGE = 4
} CAT_button_state;

int input_map[CAT_BUTTON_LAST] =
{
	GLFW_KEY_ESCAPE,
	GLFW_KEY_TAB,
	GLFW_KEY_W,
	GLFW_KEY_D,
	GLFW_KEY_S,
	GLFW_KEY_A,
	GLFW_KEY_P,
	GLFW_KEY_L
};

int input_mask[CAT_BUTTON_LAST] =
{
	CAT_BUTTON_STATE_RELEASED,
	CAT_BUTTON_STATE_RELEASED,
	CAT_BUTTON_STATE_RELEASED,
	CAT_BUTTON_STATE_RELEASED,
	CAT_BUTTON_STATE_RELEASED,
	CAT_BUTTON_STATE_RELEASED,
	CAT_BUTTON_STATE_RELEASED,
	CAT_BUTTON_STATE_RELEASED
};

int CAT_button_pressed(int button)
{
	return
		(input_mask[button] & CAT_BUTTON_STATE_PRESSED) > 0 &&
		(input_mask[button] & CAT_BUTTON_STATE_EDGE) > 0;
}

void CAT_input(CAT_display* display, CAT_context* context)
{
	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		int last = input_mask[i];
		int raw = glfwGetKey(display->window, input_map[i]);

		if(raw == GLFW_PRESS)
			input_mask[i] = CAT_BUTTON_STATE_PRESSED;
		if(raw == GLFW_RELEASE)
			input_mask[i] = CAT_BUTTON_STATE_RELEASED;

		if((last & input_mask[i]) <= 0)
			input_mask[i] |= CAT_BUTTON_STATE_EDGE;
	}
}

void CAT_logic(CAT_display* display, CAT_context* context, CAT_item* dummy)
{
	switch(context->mode)
	{
		case CAT_GAME_MODE_ROOM:
		{
			CAT_vec2 beeline = CAT_vec2_sub(context->pet_target, context->pet_pos);
			float dist = CAT_vec2_mag(beeline);
			if(dist > 8.0f)
			{
				CAT_vec2 dir = CAT_vec2_mul(beeline, 1.0f/dist);
				context->pet_pos = CAT_vec2_add(context->pet_pos, CAT_vec2_mul(dir, 32*context->delta_time));
			}
			else
			{
				context->pet_timer += context->delta_time;
				if(context->pet_timer >= 1)
				{
					context->pet_timer = 0;
					context->pet_target = {rand_float(16, 224), rand_float(112, 304)};
				}
			}

			if(CAT_button_pressed(CAT_BUTTON_UP))
				context->cursor.y -= 16;
			if(CAT_button_pressed(CAT_BUTTON_RIGHT))
				context->cursor.x += 16;
			if(CAT_button_pressed(CAT_BUTTON_DOWN))
				context->cursor.y += 16;
			if(CAT_button_pressed(CAT_BUTTON_LEFT))
				context->cursor.x -= 16;
			if(CAT_button_pressed(CAT_BUTTON_A))
			{
				CAT_place_prop(&context->room, dummy, context->cursor);
			}

			if(CAT_button_pressed(CAT_BUTTON_B))
			{
				context->mode = CAT_GAME_MODE_MENU;
			}

			break;
		}
		case CAT_GAME_MODE_MENU:
		{
			if(CAT_button_pressed(CAT_BUTTON_UP))
				CAT_menu_shift(&context->main_menu, -1);
			if(CAT_button_pressed(CAT_BUTTON_DOWN))
				CAT_menu_shift(&context->main_menu, 1);

			if(CAT_button_pressed(CAT_BUTTON_A))
			{
				const char* selection = context->menu_items[context->main_menu.idx];
				if(strcmp(selection, "INVENTORY") == 0)
				{
					context->mode = CAT_GAME_MODE_INVENTORY;
					break;
				}
				if(strcmp(selection, "BACK") == 0)
				{
					context->mode = CAT_GAME_MODE_ROOM;
					break;
				}
			}

			if(CAT_button_pressed(CAT_BUTTON_B))
			{
				context->mode = CAT_GAME_MODE_ROOM;
				break;
			}

			break;
		}
		case CAT_GAME_MODE_INVENTORY:
		{
			if(CAT_button_pressed(CAT_BUTTON_UP))
				CAT_menu_shift(&context->bag_menu, -1);
			if(CAT_button_pressed(CAT_BUTTON_DOWN))
				CAT_menu_shift(&context->bag_menu, 1);

			if(CAT_button_pressed(CAT_BUTTON_B))
			{
				context->mode = CAT_GAME_MODE_MENU;
				break;
			}

			break;
		}
	}
}

int main(int argc, char** argv)
{
	CAT_display display;
	CAT_display_init(&display);

	CAT_atlas atlas;
	CAT_texture atlas_tex;
	CAT_load_PNG(&atlas_tex, "sprites/test.png");
	CAT_atlas_init(&atlas, &atlas_tex);

	CAT_sprite bg_sprite = {272, 0, 240, 320};
	CAT_atlas_add(&atlas, 0, bg_sprite);
	CAT_sprite guy_sprite[3] = {{48, 16, 36, 32}, {84, 16, 36, 32}, {120, 16, 36, 32}};
	CAT_atlas_add(&atlas, 11, guy_sprite[0]);
	CAT_atlas_add(&atlas, 12, guy_sprite[1]);
	CAT_sprite chair_sprite = {0, 16, 24, 46};
	CAT_atlas_add(&atlas, 13, chair_sprite);
	CAT_sprite coffee_sprite = {24, 32, 26, 37};
	CAT_atlas_add(&atlas, 14, coffee_sprite);
	CAT_atlas_add(&atlas, 15, guy_sprite[2]);
	CAT_sprite cursor_sprite = {48, 0, 16, 16};
	CAT_atlas_add(&atlas, 16, cursor_sprite);
	CAT_sprite ascii_sprite[91];
	int alph_idx = 0;
	for(int y = 488; y < 512 && alph_idx < 91; y += 12)
	{
		for(int x = 0; x < 512 && alph_idx < 91; x += 8)
		{
			ascii_sprite[alph_idx] = {x, y, 8, 12};
			CAT_atlas_add(&atlas, 17+alph_idx, ascii_sprite[alph_idx]);
			alph_idx += 1;
		}
	}
	CAT_sprite ui_sprite[9];
	for(int i = 0; i < 9; i++)
	{
		ui_sprite[i] = {80+i*16, 0, 16, 16};
		CAT_atlas_add(&atlas, 108+i, ui_sprite[i]);
	}
	CAT_sprite seed_sprite = {160, 16, 24, 24};
	CAT_atlas_add(&atlas, 117, seed_sprite);
	CAT_sprite select_sprite = {192, 16, 16, 16};
	CAT_atlas_add(&atlas, 118, select_sprite);
	CAT_sprite exit_sprite = {224, 0, 12, 16};
	CAT_atlas_add(&atlas, 119, exit_sprite);
	CAT_sprite a_sprite = {240, 0, 16, 16};
	CAT_atlas_add(&atlas, 120, a_sprite);
	CAT_sprite b_sprite = {256, 0, 16, 16};
	CAT_atlas_add(&atlas, 121, b_sprite);
	CAT_sprite enter_sprite = {208, 16, 12, 16};
	CAT_atlas_add(&atlas, 122, enter_sprite);

	CAT_anim bg_anim;
	CAT_anim_init(&bg_anim);
	CAT_anim_add(&bg_anim, 0);
	CAT_anim idle_anim;
	CAT_anim_init(&idle_anim);
	CAT_anim_add(&idle_anim, 11);
	CAT_anim_add(&idle_anim, 12);
	CAT_anim chair_anim;
	CAT_anim_init(&chair_anim);
	CAT_anim_add(&chair_anim, 13);
	CAT_anim coffee_anim;
	CAT_anim_init(&coffee_anim);
	CAT_anim_add(&coffee_anim, 14);
	CAT_anim walk_anim;
	CAT_anim_init(&walk_anim);
	CAT_anim_add(&walk_anim, 11);
	CAT_anim_add(&walk_anim, 15);
	CAT_anim cursor_anim;
	CAT_anim_init(&cursor_anim);
	CAT_anim_add(&cursor_anim, 16);

	CAT_anim_queue anim_queue;
	CAT_anim_queue_init(&anim_queue);

	CAT_gui gui;
	for(int i = 0; i < 9; i++)
	{
		gui.tiles[i] = 108+i;
	}
	for(int i = 0; i < 91; i++)
	{
		gui.glyphs[i] = 17+i;
	}
	gui.margin = 8;

	rand_init();
	CAT_context context;
	CAT_context_init(&context);

	CAT_item chair;
	CAT_item_init(&chair, CAT_ITEM_TYPE_PROP, "Chair", &chair_anim);

	float anim_timer = 0.0f;

	CAT_debug_GUI_init(&display);
	
	while(!glfwWindowShouldClose(display.window))
	{
		float time = glfwGetTime();
		context.delta_time = time - context.time;
		context.time = time;

		glfwPollEvents();

		CAT_input(&display, &context);
		CAT_logic(&display, &context, &chair);

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		switch(context.mode)
		{
			case CAT_GAME_MODE_ROOM:
			{
				int prop_mode = CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X;
				int guy_mode = prop_mode | (context.pet_pos.x < context.pet_target.x ? CAT_DRAW_MODE_REFLECT_X : 0);

				CAT_anim_command bg_cmd;
				CAT_anim_command_init(&bg_cmd, &bg_anim, 0, 0, 0, CAT_DRAW_MODE_DEFAULT);
				CAT_anim_queue_add(&anim_queue, bg_cmd);
				
				for(int i = 0; i < context.room.prop_count; i++)
				{
					CAT_item* prop = context.room.props[i];
					CAT_ivec2 place = context.room.places[i];
					CAT_anim* anim = prop->anim;
					CAT_anim_command cmd;
					CAT_anim_command_init(&cmd, anim, 2, place.x, place.y, prop_mode);
					CAT_anim_queue_add(&anim_queue, cmd);
				}
				
				CAT_anim_command guy_cmd;
				CAT_anim_command_init(&guy_cmd, context.pet_timer > 0 ? &idle_anim : &walk_anim, 2, context.pet_pos.x, context.pet_pos.y, guy_mode);
				CAT_anim_queue_add(&anim_queue, guy_cmd);

				CAT_anim_command cursor_cmd;
				CAT_anim_command_init(&cursor_cmd, &cursor_anim, 1, context.cursor.x, context.cursor.y, CAT_DRAW_MODE_DEFAULT); 
				CAT_anim_queue_add(&anim_queue, cursor_cmd);
			
				anim_timer += context.delta_time;
				if(anim_timer >= 0.2f)
				{
					CAT_anim_queue_tick(&anim_queue);
					anim_timer = 0.0f;
				}
				CAT_anim_queue_draw(&anim_queue, &atlas, &display.frame);
				
				break;
			}
			case CAT_GAME_MODE_MENU:
			{
				CAT_gui_panel(&gui, &atlas, &display.frame, {0, 0}, {15, 2});  
				CAT_gui_text(&gui, &atlas, &display.frame, "MENU");
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, &atlas, &display.frame, 120);
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, &atlas, &display.frame, 122);
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, &atlas, &display.frame, 121);
				CAT_gui_same_line(&gui);
				CAT_gui_image(&gui, &atlas, &display.frame, 119);

				CAT_gui_panel(&gui, &atlas, &display.frame, {0, 32}, {15, 18});  

				for(int i = 0; i < context.main_menu.length; i++)
				{
					CAT_gui_text(&gui, &atlas, &display.frame, "#");
					CAT_gui_same_line(&gui);
					CAT_gui_text(&gui, &atlas, &display.frame, context.menu_items[i]);

					if(i == context.main_menu.idx)
					{
						CAT_gui_same_line(&gui);
						CAT_gui_image(&gui, &atlas, &display.frame, 118);
					}
				}
				
				break;
			}
			case CAT_GAME_MODE_INVENTORY:
			{
				CAT_gui_panel(&gui, &atlas, &display.frame, {0, 0}, {15, 2});  
				CAT_gui_text(&gui, &atlas, &display.frame, "INVENTORY");
				CAT_gui_panel(&gui, &atlas, &display.frame, {0, 32}, {15, 18});  

				for(int i = 0; i < context.bag_menu.window; i++)
				{
					int slot = context.bag_menu.base + i;

					CAT_gui_panel(&gui, &atlas, &display.frame, {0, 32+i*32}, {15, 2});
					CAT_gui_image(&gui, &atlas, &display.frame, 117); 
					CAT_gui_same_line(&gui);
					CAT_gui_text(&gui, &atlas, &display.frame, context.items[slot]);

					if(slot == context.bag_menu.idx)
					{
						CAT_gui_same_line(&gui);
						CAT_gui_image(&gui, &atlas, &display.frame, 118);
					}
				}

				break;
			}
		}
		
		CAT_display_refresh(&display);
		glUseProgram(display.shader.prog_id);
		glBindVertexArray(display.vao_id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, display.tex_id);
		glProgramUniform1i(display.shader.prog_id, display.tex_loc, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		glfwSwapBuffers(display.window);
	}

	CAT_debug_GUI_cleanup();

	return 0;
}
