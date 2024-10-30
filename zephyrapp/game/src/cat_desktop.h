#ifndef CAT_DESKTOP_H
#define CAT_DESKTOP_H

#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <pthread.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// DEV MODE

typedef struct CAT_simulator
{
    GLFWwindow* lcd;
    GLuint vao_id;
    GLuint vbo_id;
    GLuint tex_id;
    GLuint prog_id;
    GLuint tex_loc;    

	ALuint al_src_id;

    float time;
    float delta_time;
} CAT_simulator;
extern CAT_simulator simulator;

#endif