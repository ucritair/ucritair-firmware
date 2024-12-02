#ifndef _SBCDEC_H_
#define _SBCDEC_H_

#include <stdint.h>

#define SBC_MAX_SAMPLES_OUT		(16 * 8)


void sbcDecInit(void);
uint32_t sbcDecode(int16_t *out, const uint8_t **packetP);	//return num samples produced, up to SBC_MAX_SAMPLES_OUT






#endif
