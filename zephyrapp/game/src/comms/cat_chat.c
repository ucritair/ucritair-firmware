#include "cat_chat.h"

#include "cat_render.h"

void CAT_MS_chat(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER: break;

		case CAT_FSM_SIGNAL_TICK:
		{
			
		}
		break;
		
		case CAT_FSM_SIGNAL_EXIT: break;
	}
}

void CAT_draw_chat()
{
	CAT_frameberry(CAT_BLACK);
}