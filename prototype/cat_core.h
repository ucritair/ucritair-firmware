#ifndef CAT_CORE_H
#define CAT_CORE_H

#include <stdint.h>
#include <stdbool.h>
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
    
    int vert_id;
    int frag_id;
    int prog_id;
    int tex_loc;    

    float time;
    float delta_time;
} CAT_simulator;
extern CAT_simulator simulator;

void CAT_simulator_init();
void CAT_simulator_tick();


////////////////////////////////////////////////////////////////////////////////////////////////////
// LCD SCREEN

#define LCD_SCREEN_W 240
#define LCD_SCREEN_H 320

void CAT_LCD_post(uint16_t* buffer);
bool CAT_LCD_is_posted();
void CAT_LCD_set_backlight(int percent);


////////////////////////////////////////////////////////////////////////////////////////////////////
// EINK SCREEN

#define EINK_SCREEN_W 212
#define EINK_SCREEN_H 104

void CAT_eink_post(uint8_t* buffer);
bool CAT_eink_is_posted();


////////////////////////////////////////////////////////////////////////////////////////////////////
// INPUT

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

uint16_t CAT_get_buttons();

////////////////////////////////////////////////////////////////////////////////////////////////////
// TIME

uint64_t CAT_get_time_ms();

typedef struct CAT_datetime
{
    int year, month, day, hour, minute, second;
} CAT_datetime;

void CAT_get_datetime(CAT_datetime* datetime);
void CAT_set_datetime(CAT_datetime* datetime);


////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY

void* CAT_malloc(int bytes);
void CAT_free(void* ptr);

////////////////////////////////////////////////////////////////////////////////////////////////////
// POWER

int CAT_get_battery_pct();

#endif
