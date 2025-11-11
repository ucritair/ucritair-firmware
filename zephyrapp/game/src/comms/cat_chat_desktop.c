#include "cat_chat.h"

#include "msht.h"
#include "mt_test.h"

void msht_init(void)
{
	return;
}

void msht_w(const uint8_t *buff, uint32_t len)
{
	return;
}

int msht_status ( void )
{
	return 0;
}

void msht_test_callback ( char *frame, uint16_t frame_sz )
{
	return;
}

// process the ring buffer for meshtastic protobuf frames
// when we complete a frame, exec a callback with a pointer to the buffer, and it's length
int msht_process( RX_CB_FN_PTR meowback )
{
	return 0;
}
