#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cat_display.h"
#include "cat_texture.h"
#include "cat_debug.h"
#include "cat_math.h"

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

typedef struct CAT_context
{
	float time;
	float delta_time;

	CAT_vec2 pet_pos;
	CAT_vec2 pet_target;

	float pet_timer;
} CAT_context;

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

int input_mask[CAT_BUTTON_LAST];

void CAT_input(CAT_display* display, CAT_context* context)
{
	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		input_mask[i] =
			glfwGetKey(display->window, input_map[i]) ==
			GLFW_PRESS;
	}
}

void CAT_logic(CAT_display* display, CAT_context* context)
{
	CAT_vec2 beeline = CAT_vec2_sub(context->pet_target, context->pet_pos);
	float dist = CAT_vec2_mag(beeline);
	if(dist > 8.0f)
	{
		CAT_vec2 dir = CAT_vec2_mul(beeline, 1.0f/dist);
		context->pet_pos = CAT_vec2_add(context->pet_pos, CAT_vec2_mul(dir, 16*context->delta_time));
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
	CAT_sprite num_sprite[10];
	for(int i = 0; i < 10; i++)
	{
		num_sprite[i] = {80+i*16, 0, 16, 16};
		CAT_atlas_register(&atlas, i+1, num_sprite[i]);  
	}
	CAT_sprite guy_sprite[3] = {{48, 16, 36, 32}, {84, 16, 36, 32}, {120, 16, 36, 32}};
	CAT_atlas_register(&atlas, 11, guy_sprite[0]);
	CAT_atlas_register(&atlas, 12, guy_sprite[1]);
	CAT_sprite chair_sprite = {0, 16, 24, 46};
	CAT_atlas_register(&atlas, 13, chair_sprite);
	CAT_sprite coffee_sprite = {24, 32, 26, 37};
	CAT_atlas_register(&atlas, 14, coffee_sprite);
	CAT_atlas_register(&atlas, 15, guy_sprite[2]);

	CAT_anim bg_anim;
	CAT_anim_init(&bg_anim);
	CAT_anim_register(&bg_anim, 0);
	CAT_anim num_anim;
	CAT_anim_init(&num_anim);
	for(int i = 0; i < 10; i++)
	{
		CAT_anim_register(&num_anim, i+1);
	}
	CAT_anim idle_anim;
	CAT_anim_init(&idle_anim);
	CAT_anim_register(&idle_anim, 11);
	CAT_anim_register(&idle_anim, 12);
	CAT_anim chair_anim;
	CAT_anim_init(&chair_anim);
	CAT_anim_register(&chair_anim, 13);
	CAT_anim coffee_anim;
	CAT_anim_init(&coffee_anim);
	CAT_anim_register(&coffee_anim, 14);
	CAT_anim walk_anim;
	CAT_anim_init(&walk_anim);
	CAT_anim_register(&walk_anim, 11);
	CAT_anim_register(&walk_anim, 15);

	CAT_anim_queue renderer;
	CAT_anim_queue_init(&renderer);

	rand_init();
	CAT_context context;
	context.time = glfwGetTime();
	context.pet_pos = {120.0f, 160.0f};
	context.pet_target = {120.0f, 160.0f};
	context.pet_timer = 0.0f;
	float anim_timer = 0.0f;

	CAT_debug_GUI_init(&display);
	
	while(!glfwWindowShouldClose(display.window))
	{
		float time = glfwGetTime();
		context.delta_time = time - context.time;
		context.time = time;

		glfwPollEvents();

		CAT_input(&display, &context);
		CAT_logic(&display, &context);

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		
		int widget_mode = CAT_DRAW_MODE_CENTER_X | CAT_DRAW_MODE_CENTER_Y;
		int world_mode = CAT_DRAW_MODE_BOTTOM | CAT_DRAW_MODE_CENTER_X;

		CAT_anim_command bg_cmd;
		CAT_anim_command_init(&bg_cmd, &bg_anim, 0, 0, 0, CAT_DRAW_MODE_DEFAULT);
		CAT_anim_queue_add(&renderer, &bg_cmd);
		
		CAT_anim_command num_cmd;
		CAT_anim_command_init(&num_cmd, &num_anim, 2, 208, 292, widget_mode);
		CAT_anim_queue_add(&renderer, &num_cmd);
		
		CAT_anim_command guy_cmd;
		CAT_anim_command_init(&guy_cmd, &walk_anim, 1, context.pet_pos.x, context.pet_pos.y, world_mode);
		CAT_anim_queue_add(&renderer, &guy_cmd);
		
		CAT_anim_command chair_cmd;
		CAT_anim_command_init(&chair_cmd, &chair_anim, 1, 80, 148, world_mode);
		CAT_anim_queue_add(&renderer, &chair_cmd);
		
		CAT_anim_command coffee_cmd;
		CAT_anim_command_init(&coffee_cmd, &coffee_anim, 1, 80, 132, world_mode);
		CAT_anim_queue_add(&renderer, &coffee_cmd);

		anim_timer += context.delta_time;
		if(anim_timer >= 0.2f)
		{
			CAT_anim_queue_tick(&renderer);
			anim_timer = 0.0f;
		}

		CAT_anim_queue_submit(&renderer, &display.frame, &atlas);
		CAT_display_refresh(&display);

		glUseProgram(display.shader.prog_id);
		glBindVertexArray(display.vao_id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, display.tex_id);
		glProgramUniform1i(display.shader.prog_id, display.tex_loc, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		CAT_debug_GUI_begin();
		ImGui::Text("%.0f FPS", 1.0f/context.delta_time);
		ImGui::Text("pos: (%.0f, %.0f)", context.pet_pos.x, context.pet_pos.y);
		ImGui::Text("target: (%.0f, %.0f)", context.pet_target.x, context.pet_target.y);
		ImGui::Text("timer: %.2f", context.pet_timer);
		CAT_debug_GUI_end();
		
		glfwSwapBuffers(display.window);
	}

	CAT_debug_GUI_cleanup();

	return 0;
}
