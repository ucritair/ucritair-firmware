#include "cat_arcade.h"
#include "cat_machine.h"
#include "cat_math.h"
#include "cat_gui.h"
#include "cat_room.h"
#include "cat_input.h"
#include "cat_sprite.h"

#define MAX_SNAKE_LENGTH 3

int width = 0;
int height = 0;
int x[MAX_SNAKE_LENGTH];
int y[MAX_SNAKE_LENGTH];
int length = 1;
int dir = 0;
int move_timer_id = -1;
int grow_timer_id = -1;
bool dead = false;

void CAT_arcade_init()
{
	move_timer_id = CAT_timer_init(0.5f);
	grow_timer_id = CAT_timer_init(5.0f);
	x[0] = 0;
	y[0] = 0;
}

void CAT_arcade_tick()
{
	for(int i = 0; i < length && !dead; i++)
	{
		dead =
		x[i] < 0 || x[i] >= width ||
		y[i] < 0 || y[i] >= height;
	}

	if(!dead)
	{
		if(CAT_input_pressed(CAT_BUTTON_UP))
			dir = 0;
		if(CAT_input_pressed(CAT_BUTTON_RIGHT))
			dir = 1;
		if(CAT_input_pressed(CAT_BUTTON_DOWN))
			dir = 2;
		if(CAT_input_pressed(CAT_BUTTON_LEFT))
			dir = 3;
		
		if(CAT_timer_tick(move_timer_id))
		{
			for(int i = length-1; i > 0; i--)
			{
				
			}
		}
	}
}

void CAT_MS_arcade(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
			break;
		case CAT_MACHINE_SIGNAL_TICK:
		{
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_transition(&machine, CAT_MS_menu);
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(&machine, CAT_MS_room);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
			break;
	}
}

void CAT_render_arcade()
{
	CAT_gui_panel((CAT_ivec2) {0, 0}, (CAT_ivec2) {15, 2});  
	CAT_gui_text("ARCADE ");
	CAT_gui_image(fbut_b_sprite, 1);
	CAT_gui_image(icon_exit_sprite, 0);
	CAT_gui_panel((CAT_ivec2) {0, 2}, (CAT_ivec2) {15, 18});
}