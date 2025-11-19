#pragma once

#include "cat_machine.h"
#include <stdint.h>

#define CAT_CHAT_MAX_MSG_LEN 64
#define CAT_CHAT_MAX_NAME_LEN 16

typedef struct
{
	uint32_t address;
	char long_name[40];
	char short_name[5];
} CAT_chat_node;

typedef struct
{
	uint32_t from;
	uint32_t to;
	uint8_t channel;
	char text[CAT_CHAT_MAX_MSG_LEN];
	uint64_t timestamp;
} CAT_chat_msg;

void CAT_chat_log_msg(uint32_t from, uint32_t to, uint8_t channel, char* text);

void CAT_MS_chat(CAT_FSM_signal signal);
void CAT_draw_chat();

void CAT_chat_TX(const char* text, uint32_t address, uint8_t channel);
void CAT_chat_RX_meowback(char* frame, uint16_t frame_size);