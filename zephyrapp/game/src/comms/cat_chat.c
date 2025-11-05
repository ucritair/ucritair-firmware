#include "cat_chat.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_input.h"

static CAT_chat_msg in[128];
static uint8_t in_head = 0;

static void push_msg(CAT_chat_msg* msg)
{
	if(in_head >= 128)
		return;
	memcpy(&in[in_head], msg, sizeof(CAT_chat_msg));
	in_head += 1;
}

static CAT_chat_msg out;

void CAT_MS_chat(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_draw_chat);
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_gui_popup_is_open())
				return;
			if(CAT_gui_consume_popup())
				CAT_pushdown_pop();

			if(CAT_gui_keyboard_is_open())
			{
				
			}
			else
			{
				if(out.text != NULL && strlen(out.text) > 0)
				{
					out.timestamp = CAT_get_RTC_now();
					push_msg(&out);
					memset(&out, 0, sizeof(CAT_chat_msg));
				}

				if(CAT_input_pressed(CAT_BUTTON_A))
					CAT_gui_open_keyboard(&out.text);

				if(CAT_input_pressed(CAT_BUTTON_B))
					CAT_gui_open_popup("Quit chat?", CAT_POPUP_STYLE_YES_NO);
			}			
		}
		break;
		
		case CAT_FSM_SIGNAL_EXIT: break;
	}
}

void CAT_draw_chat()
{
	CAT_frameberry(CAT_WHITE);

	int cursor_x = 0;
	int cursor_y = 0;

	for(int i = 0; i < in_head; i++)
	{
		cursor_y = CAT_draw_textf(cursor_x, cursor_y, "%s\n", in[i].text);
		cursor_x = 0;
	}
}