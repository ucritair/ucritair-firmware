#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MTTEST, LOG_LEVEL_INF);
#include <zephyr/logging/log_ctrl.h>

#include <string.h>

// radio UART API
#include "msht.h"


#include "meshtastic/mesh.pb.h"
#include "meshtastic/admin.pb.h"

#include "protobufs/pb_encode.h"
#include "protobufs/pb_decode.h"


#include "mt_test.h"

#include <zephyr/random/random.h>

// pulled from https://github.com/meshtastic/Meshtastic-arduino/tree/master/src/
// GNU General Public License v3.0


#define PB_BUFSIZE 512
pb_byte_t pb_buf[PB_BUFSIZE+4];
size_t pb_size = 0; // Number of bytes currently in the buffer


int _mt_send_admin(meshtastic_AdminMessage admin) {
	//pb_buf[0] = MT_MAGIC_0;
	//pb_buf[1] = MT_MAGIC_1;
	
	pb_buf[0] = 0x94;
	pb_buf[1] = 0xC3;

	pb_ostream_t stream = pb_ostream_from_buffer(pb_buf + 4, PB_BUFSIZE);
	int status = pb_encode(&stream, meshtastic_AdminMessage_fields, &admin);
	if (!status) {
		printk("Couldn't encode AdminMessage\r\n");
		return false;
	}

	// Store the payload length in the header
	pb_buf[2] = stream.bytes_written / 256;
	pb_buf[3] = stream.bytes_written % 256;

	printk(";;; encoded pb: ");
	for ( int i = 0; i < stream.bytes_written+4; i++)
	{
		printk("%02X ", pb_buf[i]);
	}
	printk("\r\n");

	msht_w((const char *)pb_buf, 4 + stream.bytes_written);
	printk("sent admin message!\r\n");

	// Clear the buffer so it can be used to hold reply packets
	pb_size = 0;

	return 1;
}


int _mt_send_toRadio(meshtastic_ToRadio toRadio) {
	//pb_buf[0] = MT_MAGIC_0;
	//pb_buf[1] = MT_MAGIC_1;
	
	pb_buf[0] = 0x94;
	pb_buf[1] = 0xC3;

	pb_ostream_t stream = pb_ostream_from_buffer(pb_buf + 4, PB_BUFSIZE);
	int status = pb_encode(&stream, meshtastic_ToRadio_fields, &toRadio);
	if (!status) {
		printk("Couldn't encode toRadio\r\n");
		return false;
	}

	// Store the payload length in the header
	pb_buf[2] = stream.bytes_written / 256;
	pb_buf[3] = stream.bytes_written % 256;

	//int rv = mt_send_radio((const char *)pb_buf, 4 + stream.bytes_written);
	/*
	printk("tx buf: ");
	for ( int i = 0; i < stream.bytes_written; i++ )
	{
		printk("%02x ", pb_buf[i]);
	}
	printk("\r\n");
	*/

	msht_w((const char *)pb_buf, 4 + stream.bytes_written);
	printk("TXd!\r\n");

	// Clear the buffer so it can be used to hold reply packets
	pb_size = 0;

	return 1;
}


#if 0
// Request a node report from our MT
int mt_request_node_report ( void (*callback)(mt_node_t *, mt_nr_progress_t) )
{
  meshtastic_ToRadio toRadio = meshtastic_ToRadio_init_default;
  toRadio.which_payload_variant = meshtastic_ToRadio_want_config_id_tag;
  want_config_id = random(0x7FffFFff);  // random() can't handle anything bigger
  toRadio.want_config_id = want_config_id;

  bool rv = _mt_send_toRadio(toRadio);

  if (rv) node_report_callback = callback;
  return rv;
}
#endif


// FIXME: add feedback for error handling
//
// configure a channel
//

int mt_config_secondary_chan ( uint8_t idx, const char *name, const uint8_t *psk, uint8_t psk_len )
{
	if ( idx < 1 || idx > 7 )
	{
		LOG_ERR("%s(): index out of range.", __FUNCTION__);
		return 0;
	}


	meshtastic_Channel channel_buf = meshtastic_Channel_init_default;

	if ( strlen(name) > sizeof(channel_buf.settings.name) - 1 )
	{
		LOG_ERR("%s(): name '%s' too long > %d", __FUNCTION__, name, sizeof(channel_buf.settings.name) - 1);
		return 0;
	}


	channel_buf.has_settings = true;
	channel_buf.role = meshtastic_Channel_Role_SECONDARY;
	channel_buf.index = idx;


	memcpy(channel_buf.settings.psk.bytes, psk, psk_len);
	channel_buf.settings.psk.size = psk_len;

//	printk("~~~~~~~~~~~~~~~~~ name: '%s'\r\n", name);
	strncpy(channel_buf.settings.name, name, strlen(name));

	channel_buf.settings.name[strlen(name)] = '\0';


/*	printk("~~~~~~~~~~~~~~~~~ %s(): index: [%u] name '%s'    %u\r\n", __FUNCTION__, channel_buf.index, channel_buf.settings.name, sizeof(channel_buf.settings.name));

	printk("~~~~~~~~~~~~~~~~~ psk: ");
	for ( int i = 0; i < psk_len; i++ )
	{
		printk("%02X ", channel_buf.settings.psk.bytes[i]);
	}
	printk("\r\n");
*/

#if 0

	meshtastic_ChannelSettings *chan_settings = &channel_buf.settings;

	memcpy(chan_settings->psk.bytes, psk, psk_len);
	chan_settings->psk.size = psk_len;

	strncpy(chan_settings->name, name, sizeof(chan_settings->name) - 1);
	chan_settings->name[sizeof(chan_settings->name) - 1] = '\0';
#endif

	// -- AdminMessage --

	meshtastic_AdminMessage admin_msg = meshtastic_AdminMessage_init_default;
	admin_msg.set_channel = channel_buf;
	admin_msg.which_payload_variant = meshtastic_AdminMessage_set_channel_tag;


	uint8_t admin_payload[256];
	pb_ostream_t admin_stream = pb_ostream_from_buffer(admin_payload, sizeof(admin_payload));
	if (!pb_encode(&admin_stream, meshtastic_AdminMessage_fields, &admin_msg)) {
		LOG_ERR("pb_encode error! PB_GET_ERROR: %s", PB_GET_ERROR(&admin_stream));
		return 0; 
	}


	// -- MeshPacket --

	meshtastic_MeshPacket meshPacket = meshtastic_MeshPacket_init_default;
	meshPacket.which_payload_variant = meshtastic_MeshPacket_decoded_tag;
	//meshPacket.id = random(0x7FFFFFFF);
	meshPacket.decoded.portnum = meshtastic_PortNum_ADMIN_APP;
	meshPacket.to = 0x24c96312;
	//meshPacket.channel = channel_index;
	meshPacket.want_ack = false; // maybe want true?

	memcpy(meshPacket.decoded.payload.bytes, admin_payload, admin_stream.bytes_written);
	meshPacket.decoded.payload.size = admin_stream.bytes_written;

	//memcpy(meshPacket.decoded.payload.bytes, admin_msg, meshPacket.decoded.payload.size);

	// -- ToRadio --


	meshtastic_ToRadio toRadio = meshtastic_ToRadio_init_default;
	toRadio.which_payload_variant = meshtastic_ToRadio_packet_tag;
	toRadio.packet = meshPacket;

	
//	printk("Sending admin message...\r\n");


	return _mt_send_toRadio(toRadio);

	//return _mt_send_admin(admin_msg);
}


// FIXME: add feedback for error handling
//
// configure channels 1 and 2 for the event

int mt_config_add_channels ( void )
{
	// testing keys
	const char key1[16] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };
	const char key2[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

	mt_config_secondary_chan(1, "UCA Lounge", key1, sizeof(key1));

	mt_config_secondary_chan(2, "Meowlemetry", key2, sizeof(key2));
}


// send text message 
//
// text: message
// dest: 32bit address (all ones for broadcast on the public/group channels)
// channel_index: send to a "channel" (configured crypto slot)
//
// for this particular event's demo:
// 0 = public/default Channel
// 1 = General Encrypted Group Channel
// 2 = Sensor Telemetry Channel

int mt_send_text ( const char * text, uint32_t dest, uint8_t channel_index )
{
	meshtastic_MeshPacket meshPacket = meshtastic_MeshPacket_init_default;
	meshPacket.which_payload_variant = meshtastic_MeshPacket_decoded_tag;
	meshPacket.id = sys_rand32_get();
	meshPacket.decoded.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;
	meshPacket.to = dest;
	meshPacket.channel = channel_index;
	meshPacket.want_ack = true;
	meshPacket.decoded.payload.size = strlen(text);
	memcpy(meshPacket.decoded.payload.bytes, text, meshPacket.decoded.payload.size);

	meshtastic_ToRadio toRadio = meshtastic_ToRadio_init_default;
	toRadio.which_payload_variant = meshtastic_ToRadio_packet_tag;
	toRadio.packet = meshPacket;

	
	printk("Sending text message to 0x%x w/ id 0x%X on channel %u\r\n", dest, meshPacket.id, channel_index);



	return _mt_send_toRadio(toRadio);
}

const char* const which_payload_variant_name[] =
{
	NULL,
	"meshtastic_FromRadio_id_tag",
	"meshtastic_FromRadio_packet_tag",
	"meshtastic_FromRadio_my_info_tag",
	"meshtastic_FromRadio_node_info_tag",
	"meshtastic_FromRadio_config_tag",
	"meshtastic_FromRadio_log_record_tag",
	"meshtastic_FromRadio_config_complete_id_tag",
	"meshtastic_FromRadio_rebooted_tag",
	"meshtastic_FromRadio_moduleConfig_tag",
	"meshtastic_FromRadio_channel_tag",
	"meshtastic_FromRadio_queueStatus_tag",
	"meshtastic_FromRadio_xmodemPacket_tag",
	"meshtastic_FromRadio_metadata_tag",
	"meshtastic_FromRadio_mqttClientProxyMessage_tag",
	"meshtastic_FromRadio_fileInfo_tag",
	"meshtastic_FromRadio_clientNotification_tag",
	"meshtastic_FromRadio_deviceuiConfig_tag"
};



/*
 * demo to show different call backs and their useage
 *
 *
 */

void mt_test_handle_packet_callback ( char *frame, uint16_t frame_sz )
{
	int status;

	pb_istream_t stream = pb_istream_from_buffer(frame, frame_sz);

	meshtastic_FromRadio fromRadio = meshtastic_FromRadio_init_zero;
	status = pb_decode(&stream, meshtastic_FromRadio_fields, &fromRadio);

	/*
	// Be prepared to request a node report to re-establish flow after an MT reboot
	meshtastic_ToRadio toRadio = meshtastic_ToRadio_init_default;
	toRadio.which_payload_variant = meshtastic_ToRadio_want_config_id_tag;
	toRadio.want_config_id = SPECIAL_NONCE;
	*/

	if (!status)
	{
		LOG_ERR("failed to decode!");
		return;
	}

	LOG_INF("got fromRadio.which_payload_variant: %u '%s'",
			fromRadio.which_payload_variant,
			(fromRadio.which_payload_variant > 1 && fromRadio.which_payload_variant <= 17) ? which_payload_variant_name[fromRadio.which_payload_variant] : "?"
		);

	switch ( fromRadio.which_payload_variant )
	{
		case meshtastic_FromRadio_packet_tag:
			if ( fromRadio.packet.which_payload_variant == meshtastic_MeshPacket_decoded_tag )
			{
				if ( fromRadio.packet.decoded.portnum == meshtastic_PortNum_TEXT_MESSAGE_APP )
				{
					//text_message_callback(meshPacket->from, meshPacket->to, meshPacket->channel, (const char*)meshPacket->decoded.payload.bytes);
					LOG_INF("TEXT_MESSAGE_APP: from: 0x%02X to: 0x%02X chan: %u rssi: %i snr: %f msg: '%s'",
							fromRadio.packet.from,
							fromRadio.packet.to,
							fromRadio.packet.channel,
							fromRadio.packet.rx_rssi,
							fromRadio.packet.rx_snr,
							fromRadio.packet.decoded.payload.bytes
						);
				}

			}
			break;

		case meshtastic_FromRadio_node_info_tag:
//			printk(">>> got a node_info frame\r\n");
			if ( fromRadio.node_info.has_user )
			{
				// store these to build a lookup table of address -> long_name
				// on radio protocol "start" the radio dumps it's non-volatile node info DB
				LOG_INF("NODEINFO_APP: from: 0x%02X to: 0x%02X chan: %u snr: %f long_name: '%s' short_name: '%s'", 
						fromRadio.packet.from,
						fromRadio.packet.to,
						fromRadio.packet.channel,
						fromRadio.node_info.snr,
						fromRadio.node_info.user.long_name,
						fromRadio.node_info.user.short_name
					);
			}
			//else
			//{
			//	printk("no has_user\r\n");
			//}
			break;

	}

	return;
}

