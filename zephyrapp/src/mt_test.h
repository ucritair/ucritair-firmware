#ifndef MT_TEST_H
#define MT_TEST_H

#include <stdint.h>
#include <stdbool.h>


#define BROADCAST_ADDR 0xFFFFFFFF


int mt_send_text ( const char * text, uint32_t dest, uint8_t channel_index );

int mt_config_secondary_chan ( uint8_t idx, const char *name, const uint8_t *psk, uint8_t psk_len );

int mt_config_add_channels ( void );


void mt_test_handle_packet_callback ( char *frame, uint16_t frame_sz );


#endif // MT_TEST_H
