#ifndef MT_TEST_H
#define MT_TEST_H

#include <stdint.h>
#include <stdbool.h>


#define BROADCAST_ADDR 0xFFFFFFFF


int mt_send_text ( const char * text, uint32_t dest, uint8_t channel_index );


#endif // MT_TEST_H
