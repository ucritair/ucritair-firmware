#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
//#include <zephyr/net_buf.h>
#include <zephyr/sys/ring_buffer.h>

#include <string.h>

#include "msht.h"


#define MSHT_RX_COUNT 2
#define MSHT_RX_SIZE 512


// buffer pool for received frames
//NET_BUF_POOL_DEFINE(rx_frame, MSHT_RX_COUNT, MSHT_RX_SIZE, 0, NULL);



LOG_MODULE_REGISTER(msht, LOG_LEVEL_INF);

// UART3 device
static const struct device *uart3_dev = DEVICE_DT_GET(DT_NODELABEL(uart3));



// RX ring buffer for ISR
static uint8_t rxrb_buf[(MSHT_RX_SIZE+4) * 10] = {0};
static struct ring_buf rxrb;

//static volatile uint16_t G_UART3_BYTES = 0;

// UART interrupt handler
static void uart3_isr_handler(const struct device *dev, void *user_data)
{
//	printk("--- uart3_isr(): ding!\r\n");

    if (!uart_irq_update(dev)) {
        return;
    }

    if (uart_irq_rx_ready(dev)) {
        uint8_t c;
        int bytes_read = 0;

        // Read all available bytes
        while (uart_fifo_read(dev, &c, 1) == 1) {
//		printk("--- uart3_isr(): <0x%02X>\r\n", c);

		if (ring_buf_put(&rxrb, &c, 1) != 1) {
			// ring buffer could not write out!
			LOG_ERR("UART3: ring buffer full!\r\n");
		}
            bytes_read++;
        }

        if (bytes_read > 0) {
	    //G_UART3_BYTES++;
            LOG_DBG("UART3 ISR: Read %d bytes", bytes_read);
        }
    }
}


// returns the number of bytes available
int msht_status ( void )
{
	return !ring_buf_is_empty(&rxrb);
}



// callback for testing
void msht_test_callback ( char *frame, uint16_t frame_sz )
{
	int i = 0;

	printk("%s(): %u byte frame: ", __FUNCTION__, frame_sz);

	printk("< ");

	for ( i = 0; i < frame_sz; i++ )
	{
		if ( frame[i] >= ' ' && frame[i]<= '~' )
		{
			printk("'%c'", frame[i]);
		}
		else
		{
			printk("%02X", frame[i]);
		}

		printk(",");
	}

	printk(" >\r\n");
}

/*
 * Streaming API
 *
 * from: https://meshtastic.org/docs/development/device/client-api/
 *
 * The 4 byte header is constructed to both provide framing and to not look like 'normal' 7 bit ASCII.
 *
 *  Byte 0: START1 (0x94)
 *  Byte 1: START2 (0xc3)
 *  Byte 2: MSB of protobuf length
 *  Byte 3: LSB of protobuf length
 *
 * The receiver will validate length and if >512 it will assume the packet is corrupted and return to looking for START1.
 * While looking for START1 any other characters are printed as "debug output". For a small example implementation of this reader see the python implementation.
 * 
 */


// first set in msht_init(), can be changed after that if needed
volatile uint32_t msht_process_timeout_ms;


#define MSHT_MAGIC0 0x94
#define MSHT_MAGIC1 0xC3

// receive buffer callback function pointer type
typedef void (*RX_CB_FN_PTR)(char*,uint16_t);

// process the ring buffer for meshtastic protobuf frames
// when we complete a frame, exec a callback with a pointer to the buffer, and it's length
int msht_process( RX_CB_FN_PTR meowback )
{
	uint8_t c;


	// FIXME: turn this mess into an actual state machine...
	

	// flag to reset state
	uint8_t f_rstate = 0;

	// -- static state --
	static uint8_t found_magic0 = 0;
	static uint8_t found_magic1 = 0;

	static uint8_t got_len0 = 0;

	static uint16_t pbuf_len = 0;

	static uint8_t got_header = 0;

	static uint16_t pbuf_gotten = 0;

	static uint8_t buf[MSHT_RX_SIZE+4] = {0};
	static uint16_t buf_idx = 0;

	static uint64_t timeout_abs = 0;
	// ------------------
	

	while (ring_buf_get(&rxrb, &c, 1) > 0) {

		//G_UART3_BYTES--;


		if ( !got_header )
		{
			// header state machine
			// look for magic0
			if ( !found_magic1 && !found_magic0 )
			{
				if ( c == MSHT_MAGIC0 )
				{
					// began processing a fame, set the timeout
					timeout_abs = k_uptime_get() + msht_process_timeout_ms;

					// found MAGIC0!
					found_magic0 = 1;
				}
				else
				{
					// not magic... skip
					continue;
				}
			}
			// look for magic1
			else if ( !found_magic1 )
			{
				if ( c == MSHT_MAGIC1 )
				{
					// found MAGIC1!
					found_magic1 = 1;
				}
				else
				{
					// not magic... skip
					f_rstate = 1;
				}

			}
			// get the first length byte
			else if ( !got_len0 )
			{
				pbuf_len = c << 8;

				got_len0 = 1;
			}
			// get the second length byte
			else if ( got_len0 )
			{
				pbuf_len = pbuf_len | c;

				if ( pbuf_len > MSHT_RX_SIZE )
				{
					// set flag to reset our static state variables
					f_rstate = 1;

					// pbuf_len is an invalid size!
					LOG_ERR("msht_process(): pbuf_len invalid! %u\r\n", pbuf_len);
				}

				got_header = 1;
			}
			else
			{
				// SHOULD NEVER REACH HERE
				LOG_ERR("OMGWTFBBQ?!\r\n");
			}

		}
		else
		{
			// save the pbuf payload frame

			pbuf_gotten++;

			//printk("<%02X>\n", c);
			//FIXME: write this to a netbuf !

			if ( pbuf_gotten > MSHT_RX_SIZE )
			{
				LOG_DBG("OVER SIZE!?\r\n");
				f_rstate = 1;
			}
			else
			{
				buf[buf_idx] = c;
				buf_idx++;
			}


			// complete frame!
			if ( pbuf_gotten == pbuf_len )
			{
				printk("complete frame! %u bytes\r\n", pbuf_len);

				if ( meowback != NULL )
				{
					// exec callback function
					meowback(buf, pbuf_len);
				}

				// got all the bytes of the frame, reset to find the next frame
				f_rstate = 1;
			}
		}

		uint64_t timenow = k_uptime_get();

		if ( timeout_abs && timenow >= timeout_abs )
		{
			LOG_ERR("msht_process timeout %u > %u ! %u %u %u %u %u %u %u", timenow - timeout_abs, msht_process_timeout_ms, found_magic0, found_magic1, got_len0, pbuf_len, got_header, pbuf_gotten, buf_idx);

			// timeout reached
			f_rstate = 1;
		}


		// reset static state
		if ( f_rstate )
		{
			// reset our internal state
			f_rstate = 0;

			found_magic0 = 0;
			found_magic1 = 0;
			got_len0 = 0;
			pbuf_len = 0;
			got_header = 0;
			pbuf_gotten = 0;

			memset(buf, 0, sizeof(buf));
			buf_idx = 0;
			timeout_abs = 0;
		}
	}

	return 0;
}



// test write
//void testw(const uint8_t *buff, uint32_t len)
void msht_w(const uint8_t *buff, uint32_t len)
{
    if (!device_is_ready(uart3_dev)) {
        LOG_ERR("UART3 not ready");
        return;
    }

    //LOG_INF("Sending %u bytes to UART3", len);
    LOG_INF("Sending %u bytes to UART3", len);
    //for (uint32_t i = 0; i < len; i++) {
    for (uint32_t i = 0; i < len; i++) {
        uart_poll_out(uart3_dev, buff[i]);
    }
}


// --- Public API ---

void msht_init(void)
{
    if (!device_is_ready(uart3_dev)) {
    	printk("UART3 device not ready\n");
        return;
    }

    // Configure UART interrupt
    uart_irq_callback_user_data_set(uart3_dev, uart3_isr_handler, NULL);
    uart_irq_rx_enable(uart3_dev);

	
    ring_buf_init(&rxrb, sizeof(rxrb_buf), rxrb_buf); 

    printk("%s(): init complete complete\n", __FUNCTION__);

    msht_process_timeout_ms = 500;
}


