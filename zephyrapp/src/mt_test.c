#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MTTEST, LOG_LEVEL_INF);
#include <zephyr/logging/log_ctrl.h>

#include <string.h>

// radio UART API
#include "msht.h"


#include "meshtastic/mesh.pb.h"

#include "protobufs/pb_encode.h"
#include "protobufs/pb_decode.h"


#include "mt_test.h"




// pulled from https://github.com/meshtastic/Meshtastic-arduino/tree/master/src/



#define PB_BUFSIZE 512
pb_byte_t pb_buf[PB_BUFSIZE+4];
size_t pb_size = 0; // Number of bytes currently in the buffer


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

	return 0;
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


int mt_send_text ( const char * text, uint32_t dest, uint8_t channel_index )
{
	meshtastic_MeshPacket meshPacket = meshtastic_MeshPacket_init_default;
	meshPacket.which_payload_variant = meshtastic_MeshPacket_decoded_tag;
	meshPacket.id = random(0x7FFFFFFF);
	meshPacket.decoded.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;
	meshPacket.to = dest;
	meshPacket.channel = channel_index;
	meshPacket.want_ack = true;
	meshPacket.decoded.payload.size = strlen(text);
	memcpy(meshPacket.decoded.payload.bytes, text, meshPacket.decoded.payload.size);

	meshtastic_ToRadio toRadio = meshtastic_ToRadio_init_default;
	toRadio.which_payload_variant = meshtastic_ToRadio_packet_tag;
	toRadio.packet = meshPacket;

	
	printk("Sending text message '%s' to 0x%x w/ id 0x%X\r\n", text, dest, meshPacket.id);



	return _mt_send_toRadio(toRadio);
}

