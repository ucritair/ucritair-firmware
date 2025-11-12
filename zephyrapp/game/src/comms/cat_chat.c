#include "cat_chat.h"

#include "cat_input.h"
#include "stdio.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_spriter.h"
#include "sprite_assets.h"
#include "cat_colours.h"

#include "msht.h"
#include "mt_test.h"
#include "meshtastic/mesh.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

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
					CAT_gui_open_keyboard(user_buffer, sizeof(user_buffer));

				if(CAT_input_pressed(CAT_BUTTON_B))
					CAT_gui_open_popup("Quit chat?", CAT_POPUP_STYLE_YES_NO);
			}		
		}
		break;
		
		case CAT_FSM_SIGNAL_EXIT: break;
	}
}

#define MSG_PAD_X 2
#define MSG_PAD_Y 4
#define MSG_W (CAT_LCD_SCREEN_W)
#define MSG_H (CAT_GLYPH_HEIGHT+MSG_PAD_Y*2)

int draw_message(int y, int i)
{
	CAT_chat_msg* msg = fetch_msg(i);
	bool change_sender = i > 0 && strcmp(fetch_msg(i-1)->sender, msg->sender) != 0;

	CAT_fillberry(0, y, MSG_W, MSG_H, CAT_MSG_BG);

	CAT_lineberry(0, y, 4, y, CAT_MSG_RIM);
	CAT_lineberry(0, y, 0, y+4, CAT_MSG_RIM);
	CAT_lineberry(MSG_W-1-4, y+MSG_H-1, MSG_W-1, y+MSG_H-1, CAT_MSG_RIM);
	CAT_lineberry(MSG_W-1, y+MSG_H-1, MSG_W-1, y+MSG_H-1-4, CAT_MSG_RIM);

	CAT_draw_textf(MSG_PAD_X, y+MSG_PAD_Y, "%s: %s", msg->sender, msg->text);

	if(change_sender)
		CAT_lineberry(64, y, MSG_W-1-64, y, CAT_224_GREY);

	return y + MSG_H;
}

void CAT_draw_chat()
{
	CAT_frameberry(CAT_WHITE);

	CAT_draw_tinysprite
	(
		(CAT_LCD_SCREEN_W-tnyspr_chat_splash.width)/2,
		(CAT_LCD_SCREEN_H-tnyspr_chat_splash.height)/2,
		&tnyspr_chat_splash, CAT_BLACK, CAT_WHITE
	);

	int box_y1 = in_length * MSG_H;
	int keyboard_y0 = CAT_gui_keyboard_is_open() ? CAT_LCD_SCREEN_H/2 : CAT_LCD_SCREEN_H;
	int overlap = CAT_max(box_y1 - keyboard_y0, 0);

	int cursor_y = -overlap;

	for(int i = 0; i < in_length; i++)
	{
		cursor_y = draw_message(cursor_y, i);
	}
}

#define HEADER_SIZE 18
#define FOOTER_SIZE 3
#define PAYLOAD_SIZE(x) (x - HEADER_SIZE - FOOTER_SIZE)

uint8_t rcv_buf[512];

void CAT_chat_rcv_meowback(char* frame, uint16_t frame_size)
{
	memcpy(rcv_buf, frame, frame_size);

	CAT_printf("[BEGIN RAW FRAME]\n");
	int row_size = 32;
	int i = 0;
	for(i = 0; i < frame_size; i++)
	{
		CAT_printf("%x ", frame[i]);
		if(i > 0 && i % row_size == 0 || i == frame_size-1)
			CAT_printf("\n");
	}
	CAT_printf("[END RAW FRAME]\n");

	pb_ostream_t stream = pb_ostream_from_buffer(rcv_buf, sizeof(rcv_buf));
	meshtastic_FromRadio from_radio = meshtastic_FromRadio_init_default;
	bool status = pb_decode(&stream, meshtastic_FromRadio_fields, &from_radio);
	
	meshtastic_Data_payload_t payload = from_radio.packet.decoded.payload;
	CAT_printf("(%d) [%d] %s\n", status, from_radio.id, payload.bytes);
}