#include "cat_chat.h"

#include "cat_input.h"
#include "stdio.h"
#include "cat_render.h"
#include "cat_gui.h"
#include "cat_spriter.h"
#include "sprite_assets.h"
#include "cat_colours.h"
#include "cat_radio.h"
#include "cat_curves.h"

#if CAT_RADIO_ENABLED
#include "meshtastic/mesh.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#endif

#define MODULE_NAME "CAT_chat"
#define MODULE_PREFIX "["MODULE_NAME"] "

//////////////////////////////////////////////////////////////////////////
// NODE LIST

static CAT_chat_node nodes[64];
#define NODES_CAPACITY (sizeof(nodes)/sizeof(CAT_chat_node))
static uint8_t node_count = 0;
#define NODE_ADDRESS_ME 0
#define NODE_ADDRESS_ANY CAT_RADIO_BROADCAST_ADDR

static int register_node(uint32_t address, const char* name)
{
	if(node_count >= NODES_CAPACITY)
		return -1;
	int i = 0;
	while(i < node_count && nodes[i].address != address)
		i++;
	nodes[i].address = address;
	strncpy(nodes[i].name, name, sizeof(nodes[i].name));
	if(i >= node_count)
		node_count = i+1;
	return i;
}

static int find_node(uint32_t address)
{
	for(int i = 0; i < node_count; i++)
	{
		if(nodes[i].address == address)
			return i;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
// INBOX

static CAT_chat_msg in[22];
#define IN_CAPACITY (sizeof(in)/sizeof(in[0]))
static uint8_t in_whead = 0;
static uint8_t in_rhead = 0;
static uint8_t in_length = 0;
static int in_selector = 0;

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

void CAT_chat_log_msg(uint32_t from, uint32_t to, uint8_t channel, char* text)
{
	static CAT_chat_msg out;
	out.from = from;
	out.to = to;
	out.channel = channel;
	out.timestamp = CAT_get_RTC_now();
	strncpy(out.text, text, sizeof(out.text));
	push_msg(&out);
}


//////////////////////////////////////////////////////////////////////////
// OUTBOX

static char user_buffer[CAT_CHAT_MAX_MSG_LEN];


//////////////////////////////////////////////////////////////////////////
// MAIN

static CAT_timed_latch splash_latch = CAT_TIMED_LATCH_INIT(1.0f);

void CAT_MS_chat(CAT_FSM_signal signal)
{
	switch (signal)
	{
		case CAT_FSM_SIGNAL_ENTER:
		{
			CAT_set_render_callback(CAT_draw_chat);
			CAT_timed_latch_reset(&splash_latch);
			CAT_timed_latch_raise(&splash_latch);
		}
		break;

		case CAT_FSM_SIGNAL_TICK:
		{
			if(CAT_gui_popup_is_open())
				return;
			if(CAT_gui_consume_popup())
				CAT_pushdown_pop();
			
			if(CAT_timed_latch_get(&splash_latch))
			{
				if(CAT_input_pressed(CAT_BUTTON_A))
				{
					CAT_timed_latch_reset(&splash_latch);
					CAT_input_clear();
				}
				CAT_timed_latch_tick(&splash_latch);
			}

			if(CAT_gui_keyboard_is_open())
			{
				
			}
			else
			{
				if(strlen(user_buffer) > 0)
				{
					CAT_chat_TX(user_buffer, CAT_RADIO_BROADCAST_ADDR, 0);
					user_buffer[0] = '\0';
				}

				if(CAT_input_pressed(CAT_BUTTON_UP))
					in_selector = CAT_max(in_selector-1, 0);
				if(CAT_input_pressed(CAT_BUTTON_DOWN))
				{
					if(in_selector >= in_length-1)
						CAT_gui_open_keyboard(user_buffer, sizeof(user_buffer));
					else
						in_selector = CAT_min(in_selector+1, in_length-1);
				}

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
	int sender_idx = find_node(msg->from);
	CAT_chat_node* sender = sender_idx == -1 ? NULL : &nodes[sender_idx];
	bool change_sender = i > 0 && fetch_msg(i-1)->from != msg->from;
	const char* sender_name =
	sender != NULL ? sender->name :
	msg->from == NODE_ADDRESS_ME ? "Me" :
	"?";

	char line[30];
	snprintf
	(
		line, sizeof(line),
		"%s%s: %s",
		msg->to == NODE_ADDRESS_ANY ? "" : "[DM] ",
		sender_name,
		msg->text
	);
	size_t len = strlen(line);
	size_t max_len = CAT_LINE_CAPACITY(MSG_PAD_X, MSG_PAD_X, CAT_GLYPH_WIDTH);
	if(max_len - len <= 0)
	{
		for(int i = len-1; i >= 0 && i > len-4; i--)
			line[i] = '.';
	}

	CAT_fillberry(0, y, MSG_W, MSG_H, CAT_MSG_BG);

	CAT_lineberry(0, y, 4, y, CAT_MSG_RIM);
	CAT_lineberry(0, y, 0, y+4, CAT_MSG_RIM);
	CAT_lineberry(MSG_W-1-4, y+MSG_H-1, MSG_W-1, y+MSG_H-1, CAT_MSG_RIM);
	CAT_lineberry(MSG_W-1, y+MSG_H-1, MSG_W-1, y+MSG_H-1-4, CAT_MSG_RIM);

	if(i == in_selector)
	{
		CAT_strokeberry(0, y, MSG_W, MSG_H, CAT_colour_lerp(CAT_BLACK, CAT_WHITE, CAT_wave(0.5f)));
	}

	CAT_draw_text(MSG_PAD_X, y+MSG_PAD_Y, line);

	if(change_sender)
		CAT_lineberry(64, y, MSG_W-1-64, y, CAT_224_GREY);

	return y + MSG_H;
}

void CAT_draw_chat()
{
	if(CAT_timed_latch_get(&splash_latch))
	{
		CAT_frameberry(CAT_WHITE);
		CAT_draw_tinysprite
		(
			(CAT_LCD_SCREEN_W-tnyspr_chat_splash.width)/2,
			(CAT_LCD_SCREEN_H-tnyspr_chat_splash.height)/2,
			&tnyspr_chat_splash, CAT_BLACK, CAT_WHITE
		);
		return;
	}

	int box_y1 = in_length * MSG_H;
	int keyboard_y0 = CAT_gui_keyboard_is_open() ? CAT_LCD_SCREEN_H/2 : CAT_LCD_SCREEN_H;
	int overlap = CAT_max(box_y1 - keyboard_y0, 0);

	if(box_y1 < keyboard_y0)
		CAT_rowberry(box_y1, CAT_LCD_SCREEN_H, CAT_WHITE);

	int cursor_y = -overlap;
	for(int i = 0; i < in_length; i++)
	{
		cursor_y = draw_message(cursor_y, i);
	}
}

void CAT_chat_TX(const char* text, uint32_t address, uint8_t channel)
{
	CAT_chat_log_msg(NODE_ADDRESS_ME, address, channel, text);
#if CAT_RADIO_ENABLED
	CAT_radio_TX(text, address, channel);
	CAT_msleep(500);
#endif
}

void CAT_chat_RX_meowback(char* frame, uint16_t frame_size)
{
#if CAT_RADIO_ENABLED
	pb_istream_t stream = pb_istream_from_buffer(frame, frame_size);
	meshtastic_FromRadio from_radio = meshtastic_FromRadio_init_zero;
	bool status = pb_decode(&stream, meshtastic_FromRadio_fields, &from_radio);

	if (!status)
	{
		CAT_printf(MODULE_PREFIX "Failed to decode message");
		return;
	}
	CAT_printf(MODULE_PREFIX "Received payload variant %d\n", from_radio.which_payload_variant);

	switch (from_radio.which_payload_variant)
	{
		case meshtastic_FromRadio_packet_tag:
		{
			if (from_radio.packet.which_payload_variant == meshtastic_MeshPacket_decoded_tag)
			{
				if (from_radio.packet.decoded.portnum == meshtastic_PortNum_TEXT_MESSAGE_APP)
				{
					meshtastic_MeshPacket packet = from_radio.packet;

					CAT_chat_log_msg
					(
						packet.from,
						packet.to,
						packet.channel,
						packet.decoded.payload.bytes
					);

					CAT_printf
					(
						MODULE_PREFIX "Received text message:\n"
						"From: 0x%02X\n"
						"To: 0x%02X\n"
						"Channel: 0x%02X\n"
						"Message: %s\n",
						packet.from,
						packet.to,
						packet.channel,
						packet.decoded.payload.bytes
					);
				}
				else if (from_radio.packet.decoded.portnum == meshtastic_PortNum_NODEINFO_APP)
				{
					meshtastic_NodeInfo info = from_radio.node_info;

					if(info.has_user)
					{
						register_node(info.num, info.user.short_name);
					}

					CAT_printf
					(
						MODULE_PREFIX "Received node info:\n"
						"Number: 0x%02X\n"
						"Has User: %d\n"
						"Long Name: %.40s\n"
						"Short Name: %.5s\n",
						info.num,
						info.has_user,
						info.has_user ? info.user.long_name : "N/A",
						info.has_user ? info.user.short_name : "N/A"
					);
				}
			}
		};

		default:
		{
		}
		break;
	}
#else
	CAT_printf(MODULE_PREFIX "Received %d byte frame\n", frame_size);
#endif
}