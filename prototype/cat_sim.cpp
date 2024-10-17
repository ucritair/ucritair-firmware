#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cat_display.h"
#include "cat_texture.h"
#include "cat_debug.h"
#include "cat_math.h"
#include "cat_item.h"

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

typedef struct CAT_context
{
	float time;
	float delta_time;
	CAT_vec2 pet_pos;
	CAT_vec2 pet_target;
	float pet_timer;
	CAT_room room;
	CAT_ivec2 cursor;
} CAT_context;

void CAT_context_init(CAT_context* context)
{
	context->time = glfwGetTime();
	context->delta_time = 0;
	context->pet_pos = {120, 200};
	context->pet_target = {120, 200};
	context->pet_timer = 0;
	context->room.prop_count = 0;
	context->cursor = {0, 96};
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
	return (input_mask[button] & CAT_BUTTON_STATE_PRESSED) > 0;
}

int CAT_button_released(int button)
{
	return (input_mask[button] & CAT_BUTTON_STATE_RELEASED) > 0;
}

int CAT_button_edge(int button)
{
	return (input_mask[button] & CAT_BUTTON_STATE_EDGE) > 0;
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

	if(CAT_button_pressed(CAT_BUTTON_UP) && CAT_button_edge(CAT_BUTTON_UP))
		context->cursor.y -= 16;
	if(CAT_button_pressed(CAT_BUTTON_RIGHT) && CAT_button_edge(CAT_BUTTON_RIGHT))
		context->cursor.x += 16;
	if(CAT_button_pressed(CAT_BUTTON_DOWN) && CAT_button_edge(CAT_BUTTON_DOWN))
		context->cursor.y += 16;
	if(CAT_button_pressed(CAT_BUTTON_LEFT) && CAT_button_edge(CAT_BUTTON_LEFT))
		context->cursor.x -= 16;
	if(CAT_button_released(CAT_BUTTON_A) && CAT_button_edge(CAT_BUTTON_A))
	{
		CAT_place_prop(&context->room, dummy, context->cursor);
	}
}

int main(int argc, char** argv)
{
	CAT_display display;
	CAT_display_init(&display);

	CAT_atlas atlas;
	CAT_texture atlas_tex;
	CAT_load_PNG(&atlas_tex, "sprites/test.png");
	atlas.texture = &atlas_tex;

	CAT_sprite bg_sprite = {272, 0, 240, 320};
	CAT_atlas_register(&atlas, 0, bg_sprite);
	CAT_sprite guy_sprite[3] = {{48, 16, 36, 32}, {84, 16, 36, 32}, {120, 16, 36, 32}};
	CAT_atlas_register(&atlas, 11, guy_sprite[0]);
	CAT_atlas_register(&atlas, 12, guy_sprite[1]);
	CAT_sprite chair_sprite = {0, 16, 24, 46};
	CAT_atlas_register(&atlas, 13, chair_sprite);
	CAT_sprite coffee_sprite = {24, 32, 26, 37};
	CAT_atlas_register(&atlas, 14, coffee_sprite);
	CAT_atlas_register(&atlas, 15, guy_sprite[2]);
	CAT_sprite cursor_sprite = {48, 0, 16, 16};
	CAT_atlas_register(&atlas, 16, cursor_sprite);
	CAT_sprite ascii_sprite[91];
	int alph_idx = 0;
	for(int y = 464; y < 512 && alph_idx < 91; y += 16)
	{
		for(int x = 0; x < 512 && alph_idx < 91; x += 16)
		{
			ascii_sprite[alph_idx] = {x, y, 16, 16};
			CAT_atlas_register(&atlas, 17+alph_idx, ascii_sprite[alph_idx]);
			alph_idx += 1;
		}
	}

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

	CAT_anim_queue renderer;
	CAT_anim_queue_init(&renderer);

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
		
		int widget_mode = CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y;
		int world_mode = CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X;
		int guy_mode = world_mode | (context.pet_pos.x < context.pet_target.x ? CAT_DRAW_MODE_REFLECT_X : 0);

		CAT_anim_command bg_cmd;
		CAT_anim_command_init(&bg_cmd, &bg_anim, 0, 0, 0, CAT_DRAW_MODE_DEFAULT);
		CAT_anim_queue_add(&renderer, bg_cmd);
		
		CAT_anim_command guy_cmd;
		CAT_anim_command_init(&guy_cmd, context.pet_timer > 0 ? &idle_anim : &walk_anim, 2, context.pet_pos.x, context.pet_pos.y, guy_mode);
		CAT_anim_queue_add(&renderer, guy_cmd);

		for(int i = 0; i < context.room.prop_count; i++)
		{
			CAT_item* prop = context.room.props[i];
			CAT_ivec2 place = context.room.places[i];
			CAT_anim* anim = prop->anim;
			CAT_anim_command cmd;
			CAT_anim_command_init(&cmd, anim, 2, place.x, place.y, world_mode);
			CAT_anim_queue_add(&renderer, cmd);
		}

		CAT_anim_command cursor_cmd;
		CAT_anim_command_init(&cursor_cmd, &cursor_anim, 1, context.cursor.x, context.cursor.y, CAT_DRAW_MODE_DEFAULT); 
		CAT_anim_queue_add(&renderer, cursor_cmd);
	
		anim_timer += context.delta_time;
		if(anim_timer >= 0.2f)
		{
			CAT_anim_queue_tick(&renderer);
			anim_timer = 0.0f;
		}
		CAT_anim_queue_submit(&renderer, &display.frame, &atlas);
		
		char* msg = "Hello, world!\nAnd again...\nLine number 3!";
		int txt_x = 0;
		int txt_y = 16;
		for(char* c = msg; *c != '\0'; c++)
		{
			if(*c == '\n')
			{
				txt_y += 20;
				txt_x = 0;
				continue;
			}

			CAT_draw_sprite(&display.frame, txt_x, txt_y, &atlas, (*c)-' '+17, CAT_DRAW_MODE_DEFAULT);
			txt_x += 16;
		}
		
		CAT_display_refresh(&display);
		glUseProgram(display.shader.prog_id);
		glBindVertexArray(display.vao_id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, display.tex_id);
		glProgramUniform1i(display.shader.prog_id, display.tex_loc, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		CAT_debug_GUI_begin();
		//ImGui::Text("%.0f FPS", 1.0f/context.delta_time);
		//ImGui::Text("pos: (%.0f, %.0f)", context.pet_pos.x, context.pet_pos.y);
		//ImGui::Text("target: (%.0f, %.0f)", context.pet_target.x, context.pet_target.y);
		//ImGui::Text("timer: %.2f", context.pet_timer);
		CAT_debug_GUI_end();
		
		glfwSwapBuffers(display.window);
	}

	CAT_debug_GUI_cleanup();

	return 0;
}
