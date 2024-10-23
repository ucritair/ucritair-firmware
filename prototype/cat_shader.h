#ifndef CAT_SHADER_H
#define CAT_SHADER_H

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

typedef struct CAT_shader
{
	GLuint vert_id;
	GLuint frag_id;
	GLuint prog_id;
} CAT_shader;

void CAT_shader_init(CAT_shader* shader, char* vert_src, char* frag_src);

#endif
