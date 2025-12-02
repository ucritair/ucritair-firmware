#include "cat_radio.h"

#include <stddef.h>
#include <string.h>
#include "cat_core.h"
#include <stdio.h>

#ifdef CAT_RADIO_ENABLED
#include "msht.h"
#include "mt_test.h"
#include "meshtastic/mesh.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#endif

#define MODULE_NAME "CAT_radio"
#define MODULE_PREFIX "["MODULE_NAME"] "

void CAT_radio_init()
{
	CAT_printf(MODULE_PREFIX "Initializing\n");
#ifdef CAT_RADIO_ENABLED
	CAT_printf(MODULE_PREFIX "(This is a radio-enabled build)\n");
	msht_init();
	CAT_msleep(12000);
#endif
}

void CAT_radio_clear_buffer()
{
	CAT_printf(MODULE_PREFIX "Clearing buffer\n");
#ifdef CAT_RADIO_ENABLED
	if (msht_status())
		msht_process(NULL);
#endif
}

void CAT_radio_start_modem()
{
	CAT_printf(MODULE_PREFIX "Starting modem\n");
#ifdef CAT_RADIO_ENABLED
	uint8_t start_cmd[] = {0x94,0xc3,0x00,0x06,0x18,0xa6,0xbe,0xb2,0xa3,0x0b};
	msht_w(start_cmd, sizeof(start_cmd));
	CAT_msleep(500);
#endif
}

int CAT_radio_config_channel(uint8_t idx, const char* name, const uint8_t* psk, uint8_t psk_len)
{
	CAT_printf(MODULE_PREFIX "Configuring secondary channel\n");
	int result = 0;
#ifdef CAT_RADIO_ENABLED
	result = mt_config_secondary_chan(idx, name, psk, psk_len);
#endif
	return result;
}

int CAT_radio_add_chanels()
{
	CAT_printf(MODULE_PREFIX "Adding channels\n");
	int result = 0;
#ifdef CAT_RADIO_ENABLED
	result = mt_config_add_channels();
#endif
	return result;
}

void CAT_radio_TX(const char* text, uint32_t address, uint8_t channel)
{
	size_t n = strlen(text);
	CAT_printf(MODULE_PREFIX "Sending %d bytes to 0x%x on channel %d\n", n, address, channel);
#ifdef CAT_RADIO_ENABLED
	mt_send_text(text, address, channel);
#endif
}

void CAT_radio_poll_RX(CAT_radio_RX_callback meowback)
{
#ifdef CAT_RADIO_ENABLED
	if(msht_status())
		msht_process(meowback);
#endif
}

void CAT_radio_telemetry_TX()
{
	static char buffer[512];
	CAT_printf(MODULE_PREFIX "TXing telemetry data on channel %d\n", CAT_RADIO_TELEMETRY_CHANNEL);
#ifdef CAT_RADIO_ENABLED
	snprintf
	(
		buffer, sizeof(buffer),
		"Reporting: %0.1f PPM CO2, %0.1f \4g/m\5 PM 2.5, %0.1f \4g/m\5 PM 10.0",
		readings.sunrise.ppm_filtered_uncompensated,
		readings.sen5x.pm2_5,
		readings.sen5x.pm10_0
	);
	mt_send_text(buffer, CAT_RADIO_BROADCAST_ADDR, CAT_RADIO_TELEMETRY_CHANNEL);
	CAT_msleep(100);
#endif
}