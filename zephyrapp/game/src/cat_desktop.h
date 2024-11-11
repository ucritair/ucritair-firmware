#pragma once

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

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

    float time;
    float delta_time;
} CAT_simulator;
extern CAT_simulator simulator;

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

