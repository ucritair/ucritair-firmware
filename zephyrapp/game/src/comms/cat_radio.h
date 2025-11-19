#pragma once

#include <stdint.h>

#define CAT_RADIO_BROADCAST_ADDR 0xFFFFFFFF

void CAT_radio_init();
void CAT_radio_clear_buffer();
void CAT_radio_start_modem();
int CAT_radio_config_channel(uint8_t idx, const char* name, const uint8_t* psk, uint8_t psk_len);
int CAT_radio_add_chanels();
void CAT_radio_TX(const char* text, uint32_t address, uint8_t channel);
typedef void (*CAT_radio_RX_callback)(char*, uint16_t);
void CAT_radio_poll_RX(CAT_radio_RX_callback meowback);