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
#include "cat_gizmos.h"

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

static int register_node(uint32_t address, const char* short_name, const char* long_name)
{
	if(node_count >= NODES_CAPACITY)
		return -1;
	int i = 0;
	while(i < node_count && nodes[i].address != address)
		i++;

	nodes[i].address = address;
	strncpy(nodes[i].long_name, long_name, sizeof(nodes[i].long_name));
	strncpy(nodes[i].short_name, short_name, sizeof(nodes[i].short_name));

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

static CAT_chat_msg in[14];
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
	in_selector = in_length-1;
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
static uint32_t destination = CAT_RADIO_BROADCAST_ADDR;


//////////////////////////////////////////////////////////////////////////
// MAIN

static CAT_timed_latch splash_latch = CAT_TIMED_LATCH_INIT(1.0f);
static CAT_timed_latch notif_latch = CAT_TIMED_LATCH_INIT(0.5f);
static int notif_frames = 0;
static bool viewing_message = false;
static CAT_chat_msg view_buffer;

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
			if(CAT_timed_latch_get(&splash_latch))
			{
				if(CAT_input_pressed(CAT_BUTTON_A))
				{
					CAT_timed_latch_reset(&splash_latch);
					CAT_input_clear();
				}
				CAT_timed_latch_tick(&splash_latch);
				return;
			}

			if(CAT_timed_latch_get(&notif_latch))
			{
				notif_frames++;
				CAT_timed_latch_tick(&notif_latch);
			}

			if(CAT_gui_popup_is_open())
				return;
			if(CAT_gui_consume_popup())
				CAT_pushdown_pop();
			
			if(CAT_gui_keyboard_is_open())
				return;
			else
			{
				if(strlen(user_buffer) > 0)
				{
					CAT_chat_TX(user_buffer, destination, 0);
					user_buffer[0] = '\0';
				}
			}

			if(viewing_message)
			{	
				if(CAT_input_pressed(CAT_BUTTON_B))
					viewing_message = false;
				return;
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

			if(CAT_input_pressed(CAT_BUTTON_RIGHT))
			{
				int last_destination = destination;

				if(in_length == 0)
					destination = CAT_RADIO_BROADCAST_ADDR;
				else if(in[in_selector].from == NODE_ADDRESS_ME)
					destination = CAT_RADIO_BROADCAST_ADDR;
				else
					destination = in[in_selector].from;
				
				if(destination != last_destination)
				{
					CAT_timed_latch_raise(&notif_latch);
					notif_frames = 0;
				}
			}
			if(CAT_input_pressed(CAT_BUTTON_SELECT))
			{
				int last_destination = destination;
				destination = CAT_RADIO_BROADCAST_ADDR;
				if(destination != last_destination)
				{
					CAT_timed_latch_raise(&notif_latch);
					notif_frames = 0;
				}
			}

			if(CAT_input_pressed(CAT_BUTTON_A))
			{
				if(in_length > 0)
				{
					memcpy(&view_buffer, &in[in_selector], sizeof(view_buffer));
					viewing_message = true;
				}
			}

			if(CAT_input_pressed(CAT_BUTTON_B))
				CAT_gui_open_popup("Quit chat?", CAT_POPUP_STYLE_YES_NO);
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
	if(y < -MSG_H)
	{
		return y+MSG_H;
	}

	CAT_chat_msg* msg = fetch_msg(i);
	int sender_idx = find_node(msg->from);
	CAT_chat_node* sender = sender_idx == -1 ? NULL : &nodes[sender_idx];
	bool change_sender = i > 0 && fetch_msg(i-1)->from != msg->from;
	const char* sender_name =
	sender != NULL ? sender->short_name :
	msg->from == NODE_ADDRESS_ME ? "Me" :
	"?";

	bool selected = in_length > 0 && in_selector == i;

	char line[30];
	snprintf
	(
		line, sizeof(line),
		"%s%s: %s",
		msg->to == NODE_ADDRESS_ANY ? "" : "[DM] ",
		sender_name,
		msg->text
	);
	size_t whole_len = strlen(line);

	int left_pad = MSG_PAD_X;
	int right_pad = MSG_PAD_X + (selected ? (ui_chat_actions.width + CAT_GLYPH_WIDTH/2) : 0);
	size_t max_len = CAT_LINE_CAPACITY(left_pad, right_pad, CAT_GLYPH_WIDTH);
	if(whole_len >= max_len)
	{
		for(int j = whole_len-1; j > max_len-1; j--)
			line[j] = ' ';
		for(int j = max_len-1; j >= max_len-3; j--)
			line[j] = '.';
	}

	CAT_fillberry(0, y, MSG_W, MSG_H, CAT_MSG_BG);
	if(change_sender)
		CAT_lineberry(64, y, MSG_W-1-64, y, CAT_224_GREY);

	CAT_draw_text(MSG_PAD_X, y+MSG_PAD_Y, line);
	if(selected)
	{
		CAT_set_sprite_colour(CAT_64_GREY);
		CAT_draw_sprite(&ui_chat_actions, 0, MSG_W-MSG_PAD_X-ui_chat_actions.width, y+MSG_PAD_Y);
		CAT_strokeberry(0, y, MSG_W, MSG_H, CAT_colour_lerp(CAT_64_GREY, CAT_WHITE, CAT_wave(0.5f)));
	}

	return y + MSG_H;
}

#define MSG_Y0 MSG_H

void CAT_draw_chat()
{
	// Splash

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

	// View

	if(viewing_message)
	{
		CAT_frameberry(CAT_WHITE);
		CAT_strokeberry(0, 0, CAT_LCD_SCREEN_W, CAT_LCD_SCREEN_H, CAT_192_GREY);

		int cursor_y = 12;
		int cursor_x = 12;
		
		if(view_buffer.from == NODE_ADDRESS_ME)
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "From: Me\n");
		else
			cursor_y = CAT_draw_textf(cursor_x, cursor_y, "From: 0x%.8x\n", view_buffer.from);
		if(view_buffer.to == CAT_RADIO_BROADCAST_ADDR)
		cursor_y = CAT_draw_textf(cursor_x, cursor_y, "To: %s\n\n", view_buffer.to == CAT_RADIO_BROADCAST_ADDR ? "Everyone" : "Me");
		CAT_rowberry(cursor_y, cursor_y+1, CAT_192_GREY);

		CAT_set_text_mask(12, -1, CAT_LCD_SCREEN_W-12, -1);
		CAT_set_text_flags(CAT_TEXT_FLAG_WRAP);
		cursor_y = CAT_draw_textf(12, cursor_y, "\n%s\n\n", view_buffer.text);

		cursor_x = CAT_draw_button(cursor_x, cursor_y, CAT_BUTTON_B, CAT_64_GREY);
		CAT_set_text_colour(CAT_64_GREY);
		CAT_draw_text(cursor_x, cursor_y, " to close message");
		return;
	}

	// Messages and Keyboard

	int box_y1 = MSG_Y0 + in_length * MSG_H;
	int keyboard_y0 = CAT_gui_keyboard_is_open() ? CAT_LCD_SCREEN_H/2 : CAT_LCD_SCREEN_H;
	int overlap = CAT_max(box_y1 - keyboard_y0, 0);

	if(box_y1 < keyboard_y0)
		CAT_rowberry(0, keyboard_y0, CAT_WHITE);

	int cursor_y = MSG_Y0 - overlap;
	for(int i = 0; i < in_length; i++)
	{
		cursor_y = draw_message(cursor_y, i);
	}

	// Top Overlay

	cursor_y = MSG_PAD_Y;
	int cursor_x = MSG_PAD_X;
	if(!CAT_timed_latch_get(&notif_latch) || notif_frames % 2 == 0)
	{
		CAT_set_text_colour(CAT_GREY);
		if(destination == CAT_RADIO_BROADCAST_ADDR)
			CAT_draw_textf(cursor_x, cursor_y, "Broadcasting publicly!");
		else
			CAT_draw_textf(cursor_x, cursor_y, "Direct messaging 0x%.8x", destination);
	}
	CAT_rowberry(MSG_H-1, MSG_H, CAT_192_GREY);

	// Bottom Overlay

	if(!CAT_gui_keyboard_is_open())
	{
		cursor_y = MSG_Y0 + in_length * MSG_H + MSG_PAD_Y;
		int cursor_x = MSG_PAD_X;
		if(in_selector >= in_length-1)
		{
			cursor_x = CAT_draw_button(cursor_x, cursor_y, CAT_BUTTON_DOWN, CAT_160_GREY);
			CAT_set_text_colour(CAT_160_GREY);
			cursor_y = CAT_draw_text(cursor_x, cursor_y, " to compose a new message\n");
			cursor_x = MSG_PAD_X;
			
			if(destination != CAT_RADIO_BROADCAST_ADDR)
			{	
				cursor_x = CAT_draw_button(cursor_x, cursor_y, CAT_BUTTON_SELECT, CAT_160_GREY);
				CAT_set_text_colour(CAT_160_GREY);
				cursor_y = CAT_draw_text(cursor_x, cursor_y, " to broadcast publicly\n");
			}
		}
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

					register_node(packet.from, "????", "???? ????");

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
			}
		};

		case meshtastic_FromRadio_node_info_tag:
		{
			meshtastic_NodeInfo info = from_radio.node_info;

			if(from_radio.node_info.has_user)
			{
				register_node(info.num, info.user.short_name, info.user.long_name);
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
		break;

		default:
		{
		}
		break;
	}
#else
	CAT_printf(MODULE_PREFIX "Received %d byte frame\n", frame_size);
#endif
}