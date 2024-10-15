#ifndef CAT_DISPLAY_H
#define CAT_DISPLAY_H

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include "cat_texture.h"
#include "cat_shader.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

typedef struct CAT_display
{
	GLFWwindow* window;
	
	GLuint vao_id;
	GLuint vbo_id;
	CAT_texture frame;
	GLuint tex_id;
	
	CAT_shader shader;
	int tex_loc;	
} CAT_display;

void glfw_error_callback(int error, const char* description)
{
	printf("[%d] %s\n", error, description);
}

void CAT_display_init(CAT_display* display)
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
    display->window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CAT", NULL, NULL);
	if(display->window == NULL)
	{
		printf("Failed to create window\n");
	}

	glfwMakeContextCurrent(display->window);
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
    glGenVertexArrays(1, &display->vao_id);
    glBindVertexArray(display->vao_id);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &display->vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, display->vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);

    CAT_texture_init(&display->frame, SCREEN_WIDTH, SCREEN_HEIGHT);
    glGenTextures(1, &display->tex_id);
    glBindTexture(GL_TEXTURE_2D, display->tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 240, 320, 0, GL_RGBA, GL_FLOAT, display->frame.data);
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
	CAT_shader_init(&display->shader, vert_src, frag_src);

	display->tex_loc = glGetUniformLocation(display->shader.prog_id, "tex");
}

void CAT_display_refresh(CAT_display* display)
{
    glBindTexture(GL_TEXTURE_2D, display->tex_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_FLOAT, display->frame.data);
}

#endif
