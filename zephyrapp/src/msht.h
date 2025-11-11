#ifndef MSHT_H
#define MSHT_H

#include <stdint.h>
#include <stdbool.h>



// --- API Functions ---

/**
 * Initialize meshtastic radio UART1
 */
void msht_init(void);

void msht_w(const uint8_t *buff, uint32_t len);



int msht_status ( void );


void msht_test_callback ( char *frame, uint16_t frame_sz );

// receive buffer callback function pointer type
typedef void (*RX_CB_FN_PTR)(char*,uint16_t);
                                        
// process the ring buffer for meshtastic protobuf frames
// when we complete a frame, exec a callback with a pointer to the buffer, and it's length
int msht_process( RX_CB_FN_PTR meowback );

//int msht_process(void);



#endif // MSHT_H
