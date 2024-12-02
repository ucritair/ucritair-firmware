#include "sbcdec.h"

#ifndef FUKC_ZEPHYR

	#define BITPOOL 23

#endif
/*

Copyright (c) 2012, Dmitry Grinberg (dmitrygr@gmail.com / http://dmitrygr.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

	Redistributions of source code must retain the above copyright notice, this
		list of conditions and the following disclaimer.
	Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define NUM_SUBBANDS			8
#define NUM_BLOCKS				16



///config options begin



///config options end

#define CONST(x)			(x >> 16)
#define INSAMPLE			int16_t
#define OUTSAMPLE			uint16_t
#define FIXED				int16_t
#define FIXED_S				int32_t

#define NUM_FRAC_BITS_PROTO	16
#define NUM_FRAC_BITS_COS	14




typedef struct{

	FIXED V[160];

}SbcDecoderState;

static SbcDecoderState mSDS;

static const FIXED __attribute__((aligned(4))) proto_8_80[] =
{
	CONST(0x00000000), CONST(0x0083D8D4), CONST(0x0172E691), CONST(0x034FD9E0),
	CONST(0x116860F5), CONST(0x259ED8EB), CONST(0xEE979F0B), CONST(0x034FD9E0),
	CONST(0xFE8D196F), CONST(0x0083D8D4), CONST(0x000A42E6), CONST(0x0089DE90),
	CONST(0x020E372C), CONST(0x02447D75), CONST(0x153E7D35), CONST(0x253844DE),
	CONST(0xF2625120), CONST(0x03EBE849), CONST(0xFF1ACF26), CONST(0x0074E5CF),
	CONST(0x00167EE3), CONST(0x0082B6EC), CONST(0x02AD6794), CONST(0x00BFA1FF),
	CONST(0x18FAB36D), CONST(0x24086BF5), CONST(0xF5FF2BF8), CONST(0x04270CA8),
	CONST(0xFF93E21B), CONST(0x0060C1E9), CONST(0x002458FC), CONST(0x0069F16C),
	CONST(0x03436717), CONST(0xFEBDD6E5), CONST(0x1C7762DF), CONST(0x221D9DE0),
	CONST(0xF950DCFC), CONST(0x0412523E), CONST(0xFFF44825), CONST(0x004AB4C5),
	CONST(0x0035FF13), CONST(0x003B1FA4), CONST(0x03C04499), CONST(0xFC4086B8),
	CONST(0x1F8E43F2), CONST(0x1F8E43F2), CONST(0xFC4086B8), CONST(0x03C04499),
	CONST(0x003B1FA4), CONST(0x0035FF13), CONST(0x004AB4C5), CONST(0xFFF44825),
	CONST(0x0412523E), CONST(0xF950DCFC), CONST(0x221D9DE0), CONST(0x1C7762DF),
	CONST(0xFEBDD6E5), CONST(0x03436717), CONST(0x0069F16C), CONST(0x002458FC),
	CONST(0x0060C1E9), CONST(0xFF93E21B), CONST(0x04270CA8), CONST(0xF5FF2BF8),
	CONST(0x24086BF5), CONST(0x18FAB36D), CONST(0x00BFA1FF), CONST(0x02AD6794),
	CONST(0x0082B6EC), CONST(0x00167EE3), CONST(0x0074E5CF), CONST(0xFF1ACF26),
	CONST(0x03EBE849), CONST(0xF2625120), CONST(0x253844DE), CONST(0x153E7D35),
	CONST(0x02447D75), CONST(0x020E372C), CONST(0x0089DE90), CONST(0x000A42E6)
};

static const FIXED __attribute__((aligned(4))) costab_8[] =
{
	CONST(0x2D413CCD), CONST(0xD2BEC333), CONST(0xD2BEC333), CONST(0x2D413CCD),
	CONST(0x2D413CCD), CONST(0xD2BEC333), CONST(0xD2BEC333), CONST(0x2D413CCD),
	CONST(0x238E7673), CONST(0xC13AD060), CONST(0x0C7C5C1E), CONST(0x3536CC52),
	CONST(0xCAC933AE), CONST(0xF383A3E2), CONST(0x3EC52FA0), CONST(0xDC71898D),
	CONST(0x187DE2A7), CONST(0xC4DF2862), CONST(0x3B20D79E), CONST(0xE7821D59),
	CONST(0xE7821D59), CONST(0x3B20D79E), CONST(0xC4DF2862), CONST(0x187DE2A7),
	CONST(0x0C7C5C1E), CONST(0xDC71898D), CONST(0x3536CC52), CONST(0xC13AD060),
	CONST(0x3EC52FA0), CONST(0xCAC933AE), CONST(0x238E7673), CONST(0xF383A3E2),
	CONST(0x00000000), CONST(0x00000000), CONST(0x00000000), CONST(0x00000000),
	CONST(0x00000000), CONST(0x00000000), CONST(0x00000000), CONST(0x00000000),
	CONST(0xF383A3E2), CONST(0x238E7673), CONST(0xCAC933AE), CONST(0x3EC52FA0),
	CONST(0xC13AD060), CONST(0x3536CC52), CONST(0xDC71898D), CONST(0x0C7C5C1E),
	CONST(0xE7821D59), CONST(0x3B20D79E), CONST(0xC4DF2862), CONST(0x187DE2A7),
	CONST(0x187DE2A7), CONST(0xC4DF2862), CONST(0x3B20D79E), CONST(0xE7821D59),
	CONST(0xDC71898D), CONST(0x3EC52FA0), CONST(0xF383A3E2), CONST(0xCAC933AE),
	CONST(0x3536CC52), CONST(0x0C7C5C1E), CONST(0xC13AD060), CONST(0x238E7673),
	CONST(0xD2BEC333), CONST(0x2D413CCD), CONST(0x2D413CCD), CONST(0xD2BEC333),
	CONST(0xD2BEC333), CONST(0x2D413CCD), CONST(0x2D413CCD), CONST(0xD2BEC333),
	CONST(0xCAC933AE), CONST(0x0C7C5C1E), CONST(0x3EC52FA0), CONST(0x238E7673),
	CONST(0xDC71898D), CONST(0xC13AD060), CONST(0xF383A3E2), CONST(0x3536CC52),
	CONST(0xC4DF2862), CONST(0xE7821D59), CONST(0x187DE2A7), CONST(0x3B20D79E),
	CONST(0x3B20D79E), CONST(0x187DE2A7), CONST(0xE7821D59), CONST(0xC4DF2862),
	CONST(0xC13AD060), CONST(0xCAC933AE), CONST(0xDC71898D), CONST(0xF383A3E2),
	CONST(0x0C7C5C1E), CONST(0x238E7673), CONST(0x3536CC52), CONST(0x3EC52FA0),
	CONST(0xC0000000), CONST(0xC0000000), CONST(0xC0000000), CONST(0xC0000000),
	CONST(0xC0000000), CONST(0xC0000000), CONST(0xC0000000), CONST(0xC0000000),
	CONST(0xC13AD060), CONST(0xCAC933AE), CONST(0xDC71898D), CONST(0xF383A3E2),
	CONST(0x0C7C5C1E), CONST(0x238E7673), CONST(0x3536CC52), CONST(0x3EC52FA0),
	CONST(0xC4DF2862), CONST(0xE7821D59), CONST(0x187DE2A7), CONST(0x3B20D79E),
	CONST(0x3B20D79E), CONST(0x187DE2A7), CONST(0xE7821D59), CONST(0xC4DF2862),
	CONST(0xCAC933AE), CONST(0x0C7C5C1E), CONST(0x3EC52FA0), CONST(0x238E7673),
	CONST(0xDC71898D), CONST(0xC13AD060), CONST(0xF383A3E2), CONST(0x3536CC52)
};

static const int8_t loudness_8_32k[] = { -3, 0, 0, 0, 0, 0, 1, 2 };

static void __attribute__((naked)) synth(OUTSAMPLE* dst, const INSAMPLE* src, FIXED* V)
{
	__asm__ volatile(
		"	push	{r4-r11, lr}				\n\t"
		
		//shift
		"	add		r2, r2, #2 * (160 - 16)		\n\t"
		"	movs	r3, #9						\n\t"
		"1:										\n\t"
		"	mov		r9, r2						\n\t"
		"	subs	r2, #2 * 16					\n\t"
		"	ldmia	r2, {r4-r8, r10, r12, lr}	\n\t"
		"	stmia	r9, {r4-r8, r10, r12, lr}	\n\t"
		"	subs	r3, #1						\n\t"
		"	bne		1b							\n\t"

		//matrix
		"	ldmia	r1, {r8, r9, r12, lr}		\n\t"	//src words
		"	ldr		r1, =%0						\n\t"	//costab
		"	movs	r3, #16						\n\t"
		"1:										\n\t"
		"	ldmia	r1!, {r4-r7}				\n\t"	//cos words
		"	smuad	r10, r4, r8					\n\t"
		"	smlad	r10, r5, r9, r10			\n\t"
		"	smlad	r10, r6, r12, r10			\n\t"
		"	smlad	r10, r7, lr, r10			\n\t"
		"	mov		r10, r10, asr %2			\n\t"
		"	subs	r3, #1						\n\t"
		"	strh	r10, [r2], #2				\n\t"
		"	bne		1b							\n\t"
		"	subs	r2, #2 * 16					\n\t"	//restore V

		//calculate audio samples
		"	ldr		r1, =%1						\n\t"	//proto_8_80
		"	mov		r12, #8						\n\t"
		"1:										\n\t"
		"	ldmia	r1!, {r3-r7}				\n\t"

		"	ldrh	r8, [r2], #2				\n\t"	//we'd need this += 2 later, but we do it now and decrement all indices by 1
		"	ldrh	r9, [r2, #2 * 23]			\n\t"
		"	ldrh	r10, [r2, #2 * 31]			\n\t"
		"	ldrh	lr, [r2, #2 * 55]			\n\t"
		"	smulbb	r11, r3, r8					\n\t"
		"	smlatb	r11, r3, r9, r11			\n\t"
		"	smlabb	r11, r4, r10, r11			\n\t"
		"	smlatb	r11, r4, lr, r11			\n\t"

		"	ldrh	r8, [r2, #2 * 63]			\n\t"
		"	ldrh	r9, [r2, #2 * 87]			\n\t"
		"	ldrh	r10, [r2, #2 * 95]			\n\t"
		"	ldrh	lr, [r2, #2 * 119]			\n\t"
		"	smlabb	r11, r5, r8, r11			\n\t"
		"	smlatb	r11, r5, r9, r11			\n\t"
		"	smlabb	r11, r6, r10, r11			\n\t"
		"	smlatb	r11, r6, lr, r11			\n\t"

		"	ldrh	r8, [r2, #2 * 127]			\n\t"
		"	ldrh	r9, [r2, #2 * 151]			\n\t"
		"	smlabb	r11, r7, r8, r11			\n\t"
		"	smlatb	r11, r7, r9, r11			\n\t"

		"	ssat	r11, #16, r11, asr %3		\n\t"
		"	subs	r12, #1						\n\t"
		"	strh	r11, [r0], #2				\n\t"
		
		"	bne		1b							\n\t"

		//done
		"	pop		{r4-r11, pc}				\n\t"
		:
		:"i"((uintptr_t)costab_8), "i"((uintptr_t)proto_8_80), "I"(NUM_FRAC_BITS_COS), "I"(NUM_FRAC_BITS_PROTO - 1 - 3)
		:"memory"
	);
}

static const uint8_t* sbcDecodePacket(const uint8_t* buf, SbcDecoderState* ds, int16_t *out)  //only supports mono (for simplicity)
{
	//workspace
	uint_fast8_t i, j, k;
	uint8_t scaleFactors[8];
	int8_t bitneed[8];
	uint8_t bits[8];
	int_fast8_t max_bitneed = 0;
	

	//read scale factors
	for (i = 0; i < NUM_SUBBANDS; i += 2) {
		uint_fast8_t val = *buf++;

		scaleFactors[i + 0] = val >> 4;
		scaleFactors[i + 1] = val & 0x0f;
	}
	

	for(i = 0; i < NUM_SUBBANDS; i++){

		if(scaleFactors[i]){

			signed loudness = scaleFactors[i] - loudness_8_32k[i];

			if(loudness > 0) loudness /= 2;
			bitneed[i] = loudness;
		}
		else bitneed[i] = -5;
		if(bitneed[i] > max_bitneed) max_bitneed = bitneed[i];
	}

	//fit bitslices into the bitpool
	int32_t bitcount = 0, slicecount = 0, bitslice = max_bitneed + 1;
	do{
		bitslice--;
		bitcount += slicecount;
		slicecount = 0;
		for(i = 0; i < NUM_SUBBANDS; i++){

			if(bitneed[i] > bitslice + 1 && bitneed[i] < bitslice + 16) slicecount++;
			else if(bitneed[i] == bitslice + 1) slicecount += 2;
		}

	}while(bitcount + slicecount < BITPOOL);

	//distribute bits
	for(i = 0; i < NUM_SUBBANDS; i++){

		if (bitneed[i] < bitslice + 2) bits[i] = 0;
		else{

			int_fast8_t v = bitneed[i] - bitslice;
			if (v > 16) v = 16;
			bits[i] = v;
		}
	}

	//allocate remaining bits
	for(i = 0; i < NUM_SUBBANDS && bitcount < BITPOOL; i++){

		if(bits[i] >= 2 && bits[i] < 16){

			bits[i]++;
			bitcount++;
		}
		else if(bitneed[i] == bitslice + 1 && BITPOOL > bitcount + 1){

			bits[i] = 2;
			bitcount += 2;
		}
	}
	for(i = 0; i < NUM_SUBBANDS && bitcount < BITPOOL; i++){

		if(bits[i] < 16){

			bits[i]++;
			bitcount++;
		}
	}
	
	uint32_t byte = ((uint32_t)*buf) << 24;
	uint_fast8_t bitpos = 7;


	for(j = 0; j < NUM_BLOCKS; j++){

		INSAMPLE __attribute__((aligned(4))) samples[8];

		//read data
		for(i = 0; i < NUM_SUBBANDS; i++){

			if (bits[i]) {

				uint32_t val = 0;

				k = bits[i];
				do{

					val = val * 2 + (byte >> 31);
					byte <<= 1;

					if (!bitpos--) {
						byte = ((uint32_t)*++buf) << 24;
						bitpos = 7;
					}

				} while(--k);

				val = val * 2 + 1;
				val <<= scaleFactors[i];
				val /= ((1 << bits[i]) - 1);
				val -= (1 << scaleFactors[i]);

				samples[i] = val;
			}
			else {

				samples[i] = 0;
			}
		}

		//synthesis
		synth((OUTSAMPLE*)out, samples, ds->V);
		out += NUM_SUBBANDS;
	}

	//if we used a byte partially, skip the rest of it, it is "padding"
	if (bitpos != 7)
		buf++;

	return buf;
}

void sbcDecInit(void)
{
	memset(&mSDS, 0, sizeof(mSDS));
}

uint32_t sbcDecode(int16_t *out, const uint8_t **packetP)
{
	const uint8_t *packet = *packetP;

	*packetP = sbcDecodePacket(packet, &mSDS, out);

	return NUM_SUBBANDS * NUM_BLOCKS;
}
