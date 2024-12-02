#include "cat_mole.h"

#include "cat_input.h"
#include "cat_sprite.h"
#include "cat_room.h"
#include "cat_arcade.h"
#include "cat_bag.h"
#include "cat_gui.h"

#define ARENA_WIDTH 15
#define ARENA_HEIGHT 20

int mole_high_score = 0;

static int score = 0;
static bool is_high_score = false;

void CAT_MS_mole(CAT_machine_signal signal)
{
	switch(signal)
	{
		case CAT_MACHINE_SIGNAL_ENTER:
		{
			score = 0;
			is_high_score = false;
			break;
		}
		case CAT_MACHINE_SIGNAL_TICK:
		{	
			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_machine_back();
			if(CAT_input_pressed(CAT_BUTTON_START))
				CAT_machine_transition(CAT_MS_room);
			break;
		}
		case CAT_MACHINE_SIGNAL_EXIT:
		{
			break;
		}
	}
}


static int grasses[10] = {8, 2, 4, 10, 0, 11, 11, 7, 7, 12};

void CAT_render_mole()
{
	spriter.mode = CAT_DRAW_MODE_DEFAULT;

	CAT_frameberry(RGB8882565(122, 146, 57));
	for(int y = 0; y < 20; y += 2)
	{
		CAT_draw_sprite(grass_floor_sprite, 17, grasses[y/2]*16, y*16);
	}
}