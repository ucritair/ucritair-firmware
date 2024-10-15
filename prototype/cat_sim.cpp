#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cat_display.h"
#include "cat_texture.h"
#include "cat_debug.h"

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

	int cursor_x;
	int cursor_y;
} CAT_context;

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
	CAT_sprite guy_sprite[2] = {{48, 16, 36, 32}, {84, 16, 36, 32}};
	CAT_atlas_register(&atlas, 11, guy_sprite[0]);
	CAT_atlas_register(&atlas, 12, guy_sprite[1]);
	CAT_sprite chair_sprite = {0, 16, 24, 46};
	CAT_atlas_register(&atlas, 13, chair_sprite);
	CAT_sprite coffee_sprite = {24, 32, 26, 37};
	CAT_atlas_register(&atlas, 14, coffee_sprite);

	CAT_animation bg_anim;
	CAT_animation_init(&bg_anim);
	CAT_animation_register(&bg_anim, 0);
	CAT_animation num_anim;
	CAT_animation_init(&num_anim);
	for(int i = 0; i < 10; i++)
	{
		CAT_animation_register(&num_anim, i+1);
	}
	CAT_animation guy_anim;
	CAT_animation_init(&guy_anim);
	CAT_animation_register(&guy_anim, 11);
	CAT_animation_register(&guy_anim, 12);
	CAT_animation chair_anim;
	CAT_animation_init(&chair_anim);
	CAT_animation_register(&chair_anim, 13);
	CAT_animation coffee_anim;
	CAT_animation_init(&coffee_anim);
	CAT_animation_register(&coffee_anim, 14);

	CAT_render_queue renderer;
	CAT_render_queue_init(&renderer);

	CAT_context context;
	context.time = glfwGetTime();
	float anim_timer = 0.0f;

	CAT_debug_GUI_init(&display);
	
	while(!glfwWindowShouldClose(display.window))
	{
		float time = glfwGetTime();
		context.delta_time = time - context.time;
		context.time = time;
		anim_timer += context.delta_time;

		glfwPollEvents();

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		
		if(anim_timer >= 0.2f)
		{
			CAT_animation_tick(&bg_anim);
			CAT_animation_tick(&num_anim);
			CAT_animation_tick(&guy_anim);
			CAT_animation_tick(&chair_anim);
			CAT_animation_tick(&coffee_anim);
			anim_timer = 0.0f;
		}
		
		CAT_render_command bg_render;
		CAT_render_command_init(&bg_render, &bg_anim, 0, 0, 0, 0);
		CAT_render_queue_add(&renderer, &bg_render);
		
		CAT_render_command num_render;
		CAT_render_command_init(&num_render, &num_anim, 2, 208, 292, 0);
		CAT_render_queue_add(&renderer, &num_render);
		
		CAT_render_command guy_render;
		CAT_render_command_init(&guy_render, &guy_anim, 1, 120, 160, 1);
		CAT_render_queue_add(&renderer, &guy_render);
		
		CAT_render_command chair_render;
		CAT_render_command_init(&chair_render, &chair_anim, 1, 80, 148, 1);
		CAT_render_queue_add(&renderer, &chair_render);
		
		CAT_render_command coffee_render;
		CAT_render_command_init(&coffee_render, &coffee_anim, 1, 80, 132, 1);
		CAT_render_queue_add(&renderer, &coffee_render);

		CAT_render_queue_submit(&renderer, &display.frame, &atlas);

		CAT_display_refresh(&display);
		glUseProgram(display.shader.prog_id);
		glBindVertexArray(display.vao_id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, display.tex_id);
		glProgramUniform1i(display.shader.prog_id, display.tex_loc, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		CAT_debug_GUI_begin();
		ImGui::Text("%.0f FPS", 1.0f/context.delta_time);
		CAT_debug_GUI_end();
		
		glfwSwapBuffers(display.window);
	}

	CAT_debug_GUI_cleanup();

	return 0;
}
