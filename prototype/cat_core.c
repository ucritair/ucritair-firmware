#include "cat_core.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// DEV MODE

CAT_simulator simulator;

void glfw_error_callback(int error, const char* msg)
{
	printf("GLFW error %d: %s\n", error, msg);
}

void CAT_simulator_init()
{
	glfwSetErrorCallback(glfw_error_callback);
	
	if(!glfwInit())
	{
		printf("Failed to initialize GLFW\n");
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    simulator.lcd = glfwCreateWindow(LCD_SCREEN_W, LCD_SCREEN_H, "CAT", NULL, NULL);
	if(simulator.lcd == NULL)
	{
		printf("Failed to create lcd\n");
	}

	glfwMakeContextCurrent(simulator.lcd);
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("GL version: %s\n", glGetString(GL_VERSION));
	printf("SL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	
	printf("Initializing GLEW\n");
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		printf("Failed to initialize GLEW\n");
	}

	float geometry[12] =
	{
		-1.0f, -1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		-1.0f, 1.0f,
		-1.0f, -1.0f
	};
    glGenVertexArrays(1, &simulator.vao_id);
    glBindVertexArray(simulator.vao_id);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &simulator.vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, simulator.vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);

    glGenTextures(1, &simulator.tex_id);
    glBindTexture(GL_TEXTURE_2D, simulator.tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, LCD_SCREEN_W, LCD_SCREEN_H, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
    glGenerateMipmap(GL_TEXTURE_2D);

	FILE* vert_file = fopen("shaders/cat.vert", "r");
	fseek(vert_file, SEEK_SET, SEEK_END);
	size_t vert_len = ftell(vert_file);
	rewind(vert_file);
	char vert_src[vert_len+1];
	vert_src[vert_len] = '\0';
	fread(vert_src, 1, vert_len, vert_file);
	fclose(vert_file);
	FILE* frag_file = fopen("shaders/cat.frag", "r");
	fseek(frag_file, SEEK_SET, SEEK_END);
	size_t frag_len = ftell(frag_file);
	rewind(frag_file);
	char frag_src[frag_len+1];
	frag_src[frag_len] = '\0';
	fread(frag_src, 1, frag_len, frag_file);
	fclose(frag_file);
	CAT_shader_init(&simulator.shader, vert_src, frag_src);

	simulator.tex_loc = glGetUniformLocation(simulator.shader.prog_id, "tex");
}

void CAT_simulator_tick()
{
	float time = glfwGetTime();
	simulator.delta_time = time - simulator.time;
	simulator.time = time;

	glfwPollEvents();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// LCD SCREEN

void CAT_LCD_post(uint16_t* buffer)
{
    glBindTexture(GL_TEXTURE_2D, simulator.tex_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, LCD_SCREEN_W, LCD_SCREEN_H, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, buffer);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(simulator.shader.prog_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, simulator.tex_id);
	glProgramUniform1i(simulator.shader.prog_id, simulator.tex_loc, 0);

	glBindVertexArray(simulator.vao_id);
	glDrawArrays(GL_TRIANGLES, 0, 6);	
	glfwSwapBuffers(simulator.lcd);
}

bool CAT_LCD_is_posted()
{
	return true;
}

void CAT_LCD_set_backlight(int percent)
{
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// EINK SCREEN

void CAT_ink_post(uint8_t* buffer)
{
	return;
}

bool CAT_ink_is_posted()
{
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// INPUT

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

uint16_t CAT_get_buttons()
{
	uint16_t mask = 0;
	for(int i = 0; i < CAT_BUTTON_LAST; i++)
	{
		int key = input_map[i];
		if(glfwGetKey(simulator.lcd, key))
		{
			mask |= 1 << i;
		}
	}
	return mask;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// TIME

uint64_t CAT_get_time_ms()
{
	return glfwGetTime();
}

void CAT_get_datetime(CAT_datetime* datetime)
{
	time_t t = time(NULL);
	struct tm* lt = localtime(&t);
	datetime->year = lt->tm_year;
	datetime->month = lt->tm_mon;
	datetime->day = lt->tm_mday;
	datetime->minute = lt->tm_min;
	datetime->second = lt->tm_sec;
}

void CAT_set_datetime(CAT_datetime* datetime)
{
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY

void* CAT_malloc(int bytes)
{
	return malloc(bytes);
}

void CAT_free(void* ptr)
{
	free(ptr);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

int CAT_get_battery_pct()
{
	if(glfwWindowShouldClose(simulator.lcd))
	{
		return 0;
	}
	return 100;
}