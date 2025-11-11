#pragma once

#include "cat_machine.h"
#include <stdint.h>

typedef struct
{
	char sender[16];
	char text[128];
	uint64_t timestamp;
} CAT_chat_msg;

void CAT_send_chat_msg(char* sender, char* text);

void CAT_MS_chat(CAT_FSM_signal signal);
void CAT_draw_chat();

void CAT_chat_rcv_meowback(char* frame, uint16_t frame_size);