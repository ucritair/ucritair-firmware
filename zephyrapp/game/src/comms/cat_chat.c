#include "cat_chat.h"

#include "cat_render.h"
#include "cat_gui.h"
#include "cat_input.h"
#include "stdio.h"

//////////////////////////////////////////////////////////////////////////
// INBOX

static CAT_chat_msg in[22];
#define IN_CAPACITY (sizeof(in)/sizeof(in[0]))
static uint8_t in_whead = 0;
static uint8_t in_rhead = 0;
static uint8_t in_length = 0;

static void push_msg(CAT_chat_msg* msg)
{
	memcpy(&in[in_whead], msg, sizeof(CAT_chat_msg));
	in_whead = CAT_wrap(in_whead+1, IN_CAPACITY);
	if(in_length < IN_CAPACITY)
		in_length += 1;
	else
		in_rhead = CAT_wrap(in_rhead+1, IN_CAPACITY);
}

static CAT_chat_msg* fetch_msg(int idx)
{
	if(idx < 0 || idx >= in_length)
		return NULL;
	return &in[CAT_wrap(in_rhead+idx, IN_CAPACITY)];
}

static CAT_chat_msg out;

void CAT_send_chat_msg(char* sender, char* text)
{
	out.timestamp = CAT_get_RTC_now();
	strncpy(out.sender, sender, sizeof(out.sender));
	strncpy(out.text, text, sizeof(out.text));
	push_msg(&out);
}


//////////////////////////////////////////////////////////////////////////
// OUTBOX

static char user_buffer[128];


//////////////////////////////////////////////////////////////////////////
// SYSTEM

static char sys_buffer[128];

static CAT_timed_latch sys_latch = CAT_TIMED_LATCH_INIT(5);

static void sysout_init()
{
	CAT_timed_latch_reset(&sys_latch);
	CAT_timed_latch_raise(&sys_latch);
}

static void sysout_tick()
{
	CAT_timed_latch_tick(&sys_latch);

	if(!CAT_timed_latch_get(&sys_latch) && CAT_timed_latch_flipped(&sys_latch))
	{
		CAT_datetime datetime;
		CAT_get_datetime(&datetime);
		snprintf
		(
			sys_buffer, sizeof(sys_buffer),
			"%.2d/%.2d/%.4d %.2d:%.2d:%.2d",
			datetime.day, datetime.month, datetime.year,
			datetime.hour, datetime.minute, datetime.second
		);
		CAT_send_chat_msg("SYSTEM", sys_buffer);
		CAT_timed_latch_raise(&sys_latch);
	}
}

void CAT_MS_chat(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_draw_chat);
			sysout_init();
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			sysout_tick();

			if(CAT_gui_popup_is_open())
				return;
			if(CAT_gui_consume_popup())
				CAT_pushdown_pop();

			if(CAT_gui_keyboard_is_open())
			{
				
			}
			else
			{
				if(strlen(user_buffer) > 0)
				{
					CAT_send_chat_msg("Me", user_buffer);
					user_buffer[0] = '\0';
				}

				if(CAT_input_pressed(CAT_BUTTON_A))
					CAT_gui_open_keyboard(user_buffer);

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

	int box_y1 = in_length * CAT_TEXT_LINE_HEIGHT;
	int keyboard_y0 = CAT_gui_keyboard_is_open() ? CAT_LCD_SCREEN_H/2 : CAT_LCD_SCREEN_H;
	int overlap = CAT_max(box_y1 - keyboard_y0, 0);

	int cursor_x = 0;
	int cursor_y = -overlap;

	for(int i = 0; i < in_length; i++)
	{
		CAT_chat_msg* msg = fetch_msg(i);
		if(cursor_y + CAT_TEXT_LINE_HEIGHT > 0)
			CAT_draw_textf(cursor_x, cursor_y, "%s: %s", msg->sender, msg->text);
		cursor_y += CAT_TEXT_LINE_HEIGHT;
		cursor_x = 0;
	}
}