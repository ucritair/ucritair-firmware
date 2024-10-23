#include "cat_shader.h"

#include <stdio.h>

void CAT_shader_init(CAT_shader* shader, char* vert_src, char* frag_src)
{
	int status;
	char log[512];

	shader->vert_id = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader->vert_id, 1, &vert_src, NULL);
	glCompileShader(shader->vert_id);
	glGetShaderiv(shader->vert_id, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE)
	{
		glGetShaderInfoLog(shader->vert_id, 512, NULL, log);
		printf("While compiling vertex shader:\n%s\n", log);
	}
	
	shader->frag_id = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader->frag_id, 1, &frag_src, NULL);
	glCompileShader(shader->frag_id);
	glGetShaderiv(shader->frag_id, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE)
	{
		glGetShaderInfoLog(shader->frag_id, 512, NULL, log);
		printf("While compiling fragment shader:\n%s\n", log);
	}

	shader->prog_id = glCreateProgram();
	glAttachShader(shader->prog_id, shader->vert_id);
	glAttachShader(shader->prog_id, shader->frag_id);
	glLinkProgram(shader->prog_id);
	glGetProgramiv(shader->prog_id, GL_LINK_STATUS, &status);
	if(status != GL_TRUE)
	{
		glGetShaderInfoLog(shader->prog_id, 512, NULL, log);
		printf("While linking shader program:\n%s\n", log);
	}
}
