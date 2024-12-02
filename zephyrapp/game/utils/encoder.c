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
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef signed long i32;
typedef signed short i16;
typedef signed char i8;
typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef unsigned short BufType[16][8];

u32 readBE32(void){

	u32 v = 0;
	int i, c;
	
	for(i = 0; i < 4; i++){
	
		c = getchar();
		if(c == EOF) abort();
		v = (v << 8) | c;
	}
	
	return v;
}

u32 readBE16(void){

	u32 v = 0;
	int i, c;
	
	for(i = 0; i < 2; i++){
	
		c = getchar();
		if(c == EOF) return 0x10000;
		v = (v << 8) | c;
	}
	
	return v;
}

#define ABT(...)	do{ fprintf(stderr, __VA_ARGS__); abort(); }while(0)



#define CONST(x)		(x >> 16)
#define SAMPLE_CVT(x)		(x)
#define INSAMPLE		int16_t
#define OUTSAMPLE		uint16_t
#define FIXED			int16_t
#define FIXED_S			int32_t

#define NUM_FRAC_BITS_PROTO	16
#define NUM_FRAC_BITS_COS	14


static const FIXED proto_4_40[] =
{
	CONST(0x00000000), CONST(0x02CB3E8B), CONST(0x22B63DA5), CONST(0xDD49C25B),
	CONST(0xFD34C175), CONST(0x002329CC), CONST(0x053B7546), CONST(0x31EAB920),
	CONST(0xEC1F5E6D), CONST(0xFF3773A8), CONST(0x0061C5A7), CONST(0x07646684),
	CONST(0x3F23948D), CONST(0xF89F23A7), CONST(0x007A4737), CONST(0x00B32807),
	CONST(0x083DDC80), CONST(0x4825E4A3), CONST(0x0191E578), CONST(0x00FF11CA),
	CONST(0x00FB7991), CONST(0x069FDC59), CONST(0x4B583FE6), CONST(0x069FDC59),
	CONST(0x00FB7991), CONST(0x00FF11CA), CONST(0x0191E578), CONST(0x4825E4A3),
	CONST(0x083DDC80), CONST(0x00B32807), CONST(0x007A4737), CONST(0xF89F23A7),
	CONST(0x3F23948D), CONST(0x07646684), CONST(0x0061C5A7), CONST(0xFF3773A8),
	CONST(0xEC1F5E6D), CONST(0x31EAB920), CONST(0x053B7546), CONST(0x002329CC)

};

static const FIXED proto_8_80[] =
{
	CONST(0x00000000), CONST(0x0172E691), CONST(0x116860F5), CONST(0xEE979F0B),
	CONST(0xFE8D196F), CONST(0x000A42E6), CONST(0x020E372C), CONST(0x153E7D35),
	CONST(0xF2625120), CONST(0xFF1ACF26), CONST(0x00167EE3), CONST(0x02AD6794),
	CONST(0x18FAB36D), CONST(0xF5FF2BF8), CONST(0xFF93E21B), CONST(0x002458FC),
	CONST(0x03436717), CONST(0x1C7762DF), CONST(0xF950DCFC), CONST(0xFFF44825),
	CONST(0x0035FF13), CONST(0x03C04499), CONST(0x1F8E43F2), CONST(0xFC4086B8),
	CONST(0x003B1FA4), CONST(0x004AB4C5), CONST(0x0412523E), CONST(0x221D9DE0),
	CONST(0xFEBDD6E5), CONST(0x0069F16C), CONST(0x0060C1E9), CONST(0x04270CA8),
	CONST(0x24086BF5), CONST(0x00BFA1FF), CONST(0x0082B6EC), CONST(0x0074E5CF),
	CONST(0x03EBE849), CONST(0x253844DE), CONST(0x02447D75), CONST(0x0089DE90),
	CONST(0x0083D8D4), CONST(0x034FD9E0), CONST(0x259ED8EB), CONST(0x034FD9E0),
	CONST(0x0083D8D4), CONST(0x0089DE90), CONST(0x02447D75), CONST(0x253844DE),
	CONST(0x03EBE849), CONST(0x0074E5CF), CONST(0x0082B6EC), CONST(0x00BFA1FF),
	CONST(0x24086BF5), CONST(0x04270CA8), CONST(0x0060C1E9), CONST(0x0069F16C),
	CONST(0xFEBDD6E5), CONST(0x221D9DE0), CONST(0x0412523E), CONST(0x004AB4C5),
	CONST(0x003B1FA4), CONST(0xFC4086B8), CONST(0x1F8E43F2), CONST(0x03C04499),
	CONST(0x0035FF13), CONST(0xFFF44825), CONST(0xF950DCFC), CONST(0x1C7762DF),
	CONST(0x03436717), CONST(0x002458FC), CONST(0xFF93E21B), CONST(0xF5FF2BF8),
	CONST(0x18FAB36D), CONST(0x02AD6794), CONST(0x00167EE3), CONST(0xFF1ACF26),
	CONST(0xF2625120), CONST(0x153E7D35), CONST(0x020E372C), CONST(0x000A42E6)
};

static const FIXED costab_4[] =
{
	CONST(0x2D413CCC), CONST(0x3B20D79E), CONST(0x40000000), CONST(0x3B20D79E),
	CONST(0x2D413CCC), CONST(0x187DE2A6), CONST(0x00000000), CONST(0xE7821D5A),
	CONST(0xD2BEC334), CONST(0x187DE2A6), CONST(0x40000000), CONST(0x187DE2A6),
	CONST(0xD2BEC334), CONST(0xC4DF2862), CONST(0x00000000), CONST(0x3B20D79E),
	CONST(0xD2BEC334), CONST(0xE7821D5A), CONST(0x40000000), CONST(0xE7821D5A),
	CONST(0xD2BEC334), CONST(0x3B20D79E), CONST(0x00000000), CONST(0xC4DF2862),
	CONST(0x2D413CCC), CONST(0xC4DF2862), CONST(0x40000000), CONST(0xC4DF2862),
	CONST(0x2D413CCC), CONST(0xE7821D5A), CONST(0x00000000), CONST(0x187DE2A6)
};

static const FIXED costab_8[] =
{
	CONST(0x2D413CCC), CONST(0x3536CC52), CONST(0x3B20D79E), CONST(0x3EC52F9F),
	CONST(0x40000000), CONST(0x3EC52F9F), CONST(0x3B20D79E), CONST(0x3536CC52),
	CONST(0x2D413CCC), CONST(0x238E7673), CONST(0x187DE2A6), CONST(0x0C7C5C1E),
	CONST(0x00000000), CONST(0xF383A3E2), CONST(0xE7821D5A), CONST(0xDC71898D),
	CONST(0xD2BEC334), CONST(0xF383A3E2), CONST(0x187DE2A6), CONST(0x3536CC52),
	CONST(0x40000000), CONST(0x3536CC52), CONST(0x187DE2A6), CONST(0xF383A3E2),
	CONST(0xD2BEC334), CONST(0xC13AD061), CONST(0xC4DF2862), CONST(0xDC71898D),
	CONST(0x00000000), CONST(0x238E7673), CONST(0x3B20D79E), CONST(0x3EC52F9F),
	CONST(0xD2BEC334), CONST(0xC13AD061), CONST(0xE7821D5A), CONST(0x238E7673),
	CONST(0x40000000), CONST(0x238E7673), CONST(0xE7821D5A), CONST(0xC13AD061),
	CONST(0xD2BEC334), CONST(0x0C7C5C1E), CONST(0x3B20D79E), CONST(0x3536CC52),
	CONST(0x00000000), CONST(0xCAC933AE), CONST(0xC4DF2862), CONST(0xF383A3E2),
	CONST(0x2D413CCC), CONST(0xDC71898D), CONST(0xC4DF2862), CONST(0x0C7C5C1E),
	CONST(0x40000000), CONST(0x0C7C5C1E), CONST(0xC4DF2862), CONST(0xDC71898D),
	CONST(0x2D413CCC), CONST(0x3536CC52), CONST(0xE7821D5A), CONST(0xC13AD061),
	CONST(0x00000000), CONST(0x3EC52F9F), CONST(0x187DE2A6), CONST(0xCAC933AE),
	CONST(0x2D413CCC), CONST(0x238E7673), CONST(0xC4DF2862), CONST(0xF383A3E2),
	CONST(0x40000000), CONST(0xF383A3E2), CONST(0xC4DF2862), CONST(0x238E7673),
	CONST(0x2D413CCC), CONST(0xCAC933AE), CONST(0xE7821D5A), CONST(0x3EC52F9F),
	CONST(0x00000000), CONST(0xC13AD061), CONST(0x187DE2A6), CONST(0x3536CC52),
	CONST(0xD2BEC334), CONST(0x3EC52F9F), CONST(0xE7821D5A), CONST(0xDC71898D),
	CONST(0x40000000), CONST(0xDC71898D), CONST(0xE7821D5A), CONST(0x3EC52F9F),
	CONST(0xD2BEC334), CONST(0xF383A3E2), CONST(0x3B20D79E), CONST(0xCAC933AE),
	CONST(0x00000000), CONST(0x3536CC52), CONST(0xC4DF2862), CONST(0x0C7C5C1E),
	CONST(0xD2BEC334), CONST(0x0C7C5C1E), CONST(0x187DE2A6), CONST(0xCAC933AE),
	CONST(0x40000000), CONST(0xCAC933AE), CONST(0x187DE2A6), CONST(0x0C7C5C1E),
	CONST(0xD2BEC334), CONST(0x3EC52F9F), CONST(0xC4DF2862), CONST(0x238E7673),
	CONST(0x00000000), CONST(0xDC71898D), CONST(0x3B20D79E), CONST(0xC13AD061),
	CONST(0x2D413CCC), CONST(0xCAC933AE), CONST(0x3B20D79E), CONST(0xC13AD061),
	CONST(0x40000000), CONST(0xC13AD061), CONST(0x3B20D79E), CONST(0xCAC933AE),
	CONST(0x2D413CCC), CONST(0xDC71898D), CONST(0x187DE2A6), CONST(0xF383A3E2),
	CONST(0x00000000), CONST(0x0C7C5C1E), CONST(0xE7821D5A), CONST(0x238E7673)
};

void sbcAnal4(FIXED *__restrict X, i32 *__restrict d){

	FIXED Y[16];
	const FIXED *__restrict protoTab = proto_4_40, *__restrict cosTab = costab_4;
	FIXED_S S;
	u32 i, k;
	
	for(k = 0; k < 8; k++, X++, protoTab += 5){
		
		S = 0;
		S += (FIXED_S)X[ 0] * (FIXED_S)protoTab[0];
		S += (FIXED_S)X[ 8] * (FIXED_S)protoTab[1];
		S += (FIXED_S)X[16] * (FIXED_S)protoTab[2];
		S += (FIXED_S)X[24] * (FIXED_S)protoTab[3];
		S += (FIXED_S)X[32] * (FIXED_S)protoTab[4];
		S >>= NUM_FRAC_BITS_PROTO;
		Y[k] = S;
	}
	
	for(i = 0; i < 4; i++, cosTab += 8){
	
		S = 0;
		S += (FIXED_S)Y[ 0] * (FIXED_S)cosTab[ 0];
		S += (FIXED_S)Y[ 1] * (FIXED_S)cosTab[ 1];
		S += (FIXED_S)Y[ 2] * (FIXED_S)cosTab[ 2];
		S += (FIXED_S)Y[ 3] * (FIXED_S)cosTab[ 3];
		S += (FIXED_S)Y[ 4] * (FIXED_S)cosTab[ 4];
		S += (FIXED_S)Y[ 5] * (FIXED_S)cosTab[ 5];
		S += (FIXED_S)Y[ 6] * (FIXED_S)cosTab[ 6];
		S += (FIXED_S)Y[ 7] * (FIXED_S)cosTab[ 7];
		S >>= NUM_FRAC_BITS_COS;
		
		d[i] = S;
	}
}

void sbcAnal8(FIXED *__restrict X, i32 *__restrict d){

	FIXED Y[16];
	const FIXED *__restrict protoTab = proto_8_80, *__restrict cosTab = costab_8;
	FIXED_S S;
	u32 i, k;
	
	for(k = 0; k < 16; k++, X++, protoTab += 5){
		
		S = 0;
		S += (FIXED_S)X[ 0] * (FIXED_S)protoTab[0];
		S += (FIXED_S)X[16] * (FIXED_S)protoTab[1];
		S += (FIXED_S)X[32] * (FIXED_S)protoTab[2];
		S += (FIXED_S)X[48] * (FIXED_S)protoTab[3];
		S += (FIXED_S)X[64] * (FIXED_S)protoTab[4];
		S >>= NUM_FRAC_BITS_PROTO;
		Y[k] = S;
	}
	
	for(i = 0; i < 8; i++, cosTab += 16){
	
		S = 0;
		S += (FIXED_S)Y[ 0] * (FIXED_S)cosTab[ 0];
		S += (FIXED_S)Y[ 1] * (FIXED_S)cosTab[ 1];
		S += (FIXED_S)Y[ 2] * (FIXED_S)cosTab[ 2];
		S += (FIXED_S)Y[ 3] * (FIXED_S)cosTab[ 3];
		S += (FIXED_S)Y[ 4] * (FIXED_S)cosTab[ 4];
		S += (FIXED_S)Y[ 5] * (FIXED_S)cosTab[ 5];
		S += (FIXED_S)Y[ 6] * (FIXED_S)cosTab[ 6];
		S += (FIXED_S)Y[ 7] * (FIXED_S)cosTab[ 7];
		S += (FIXED_S)Y[ 8] * (FIXED_S)cosTab[ 8];
		S += (FIXED_S)Y[ 9] * (FIXED_S)cosTab[ 9];
		S += (FIXED_S)Y[10] * (FIXED_S)cosTab[10];
		S += (FIXED_S)Y[11] * (FIXED_S)cosTab[11];
		S += (FIXED_S)Y[12] * (FIXED_S)cosTab[12];
		S += (FIXED_S)Y[13] * (FIXED_S)cosTab[13];
		S += (FIXED_S)Y[14] * (FIXED_S)cosTab[14];
		S += (FIXED_S)Y[15] * (FIXED_S)cosTab[15];
		S >>= NUM_FRAC_BITS_COS;
		
		d[i] = S;
	}
}

static const i8 loudness_4[4][4] =
{
	{ -1, 0, 0, 0 }, { -2, 0, 0, 1 },
	{ -2, 0, 0, 1 }, { -2, 0, 0, 1 }
};

static const i8 loudness_8[4][8] =
{
	{ -2, 0, 0, 0, 0, 0, 0, 1 }, { -3, 0, 0, 0, 0, 0, 1, 2 },
	{ -4, 0, 0, 0, 0, 0, 1, 2 }, { -4, 0, 0, 0, 0, 0, 1, 2 }
};

u32 sbcenc(BufType samples, u8* buf, u8 numBlocks, u8 numBits, u8 hdrByte){	//return num bytes produced
	
	//no need to print syncbyte, header byte, etc...

	static FIXED X[80] = {0,};
	u8* bufStart = buf;
	u8 sampleRateIdx = hdrByte >> 6;
	u8 numSubbands = (hdrByte & 1) ? 8 : 4;
	u8 snrMode = !!(hdrByte & 2);
	u8 scaleFactors[8] = {0,};
	i8 bitneed[8];
	i32 outsample[16][8];
	u8 bitsUsed[8] = {0,};
	u32 i, j, k, b;

	//encode
	for(b = 0; b < numBlocks; b++){
		//input
		j = (numSubbands == 8) ? 80 : 40;
		k = (numSubbands == 8) ? 8 : 4;
		for(i = j - 1; i != k - 1; i--){
		
			X[i] = X[i - k];
		}
		while(1){
		
			X[i] = samples[b][k - i - 1];
			if(!i--) break;
		}
		
		//analyze
		if(numSubbands == 8) sbcAnal8(X, outsample[b]);
		else sbcAnal4(X, outsample[b]);
	}
	
	for(j = 0; j < numSubbands; j++){
		
		u32 v = 0;
		
		for(k = 0; k < numBlocks; k++){
			
			i32 t = outsample[k][j];
			
			if(t < 0) t = -t;
			
			if(v < t) v = t;
		}
		scaleFactors[j] = v ? 31 - __builtin_clz(v) : 0;
	}
	////////////////////////////////////////////////////////////////////////
	
	//calculate bitneed table and max_bitneed value (A2DP 12.6.3.1)
	int8_t max_bitneed = 0;
	if(snrMode){

		for(i = 0; i < numSubbands; i++){

			bitneed[i] = scaleFactors[i];
			if(bitneed[i] > max_bitneed) max_bitneed = bitneed[i];
		}
	}
	else{

		const signed char* tbl;

		if(numSubbands == 8) tbl = loudness_8[sampleRateIdx];
		else tbl = loudness_4[sampleRateIdx];

		for(i = 0; i < numSubbands; i++){

			if(scaleFactors[i]){

				int loudness = scaleFactors[i] - tbl[i];

				if(loudness > 0) loudness /= 2;
				bitneed[i] = loudness;
			}
			else bitneed[i] = -5;
			if(bitneed[i] > max_bitneed) max_bitneed = bitneed[i];
		}
	}
	
	//fit bitslices into the bitpool
	int32_t bitcount = 0, slicecount = 0, bitslice = max_bitneed + 1;
	do{
		bitslice--;
		bitcount += slicecount;
		slicecount = 0;
		for(i = 0; i < numSubbands; i++){

			if(bitneed[i] > bitslice + 1 && bitneed[i] < bitslice + 16) slicecount++;
			else if(bitneed[i] == bitslice + 1) slicecount += 2;
		}

	}while(bitcount + slicecount < numBits);

	//distribute bits
	for(i = 0; i < numSubbands; i++){

		if(bitneed[i] < bitslice + 2) bitsUsed[i] = 0;
		else{

			int8_t v = bitneed[i] - bitslice;
			if(v > 16) v = 16;
			bitsUsed[i] = v;
		}
	}

	//allocate remaining bits
	for(i = 0; i < numSubbands && bitcount < numBits; i++){

		if(bitsUsed[i] >= 2 && bitsUsed[i] < 16){

			bitsUsed[i]++;
			bitcount++;
		}
		else if(bitneed[i] == bitslice + 1 && numBits > bitcount + 1){

			bitsUsed[i] = 2;
			bitcount += 2;
		}
	}
	for(i = 0; i < numSubbands && bitcount < numBits; i++){

		if(bitsUsed[i] < 16){

			bitsUsed[i]++;
			bitcount++;
		}
	}
	
	
	///quantize
	for(k = 0 ;k < numSubbands; k++){
	
		i32 mul = (1 << bitsUsed[k]) - 1;
		i32 add = mul << (scaleFactors[k] + 1);
	
		for(i = 0; i < numBlocks; i++){
			
			i32 t = outsample[i][k];
			
		//	fprintf(stderr, "quantizing %d with scale %d to %d bits -> ", t, scaleFactors[k], bitsUsed[k]);
			
			t = (t * mul + add) >> (scaleFactors[k] + 2);
			
		//	fprintf(stderr, "%d\n", t);
		
			outsample[i][k] = t;
		}
	}
	
	//output
	{
		u32 bitpos = 0, bitmask;
		
		//"scale factors" array
		for(j = 0; j < numSubbands; j++){
		
			if(bitpos){
			
				*buf = (*buf) | scaleFactors[j];
				buf++;	
			}
			else{
			
				*buf = scaleFactors[j] << 4;
			}
			bitpos ^= 1;
		}
		
		//the data bits
		
		bitmask = bitpos ? 0x08 : 0x80;
		for(i = 0; i < numBlocks; i++) for(k = 0; k < numSubbands; k++){
		
			u32 v = outsample[i][k];
			u32 b = bitsUsed[k];
			
		//	fprintf(stderr, "v=%d(0x%x) b=%d -> ", v, v, b);
			
			v <<= (16 - b);
			
		//	fprintf(stderr,"0x%x\n", v);
			
			while(b--){
			
				if(bitmask == 0x80) *buf = 0;
			
				if(v & 0x8000) *buf |= bitmask;
				v <<= 1;
				if(!(bitmask >>= 1)){
				
					buf++;
					bitmask = 0x80;	
				}
			}
		}
		
		//stuffing for last byte
		if(bitpos) buf++;
	}

	return buf - bufStart;
}

void usage(const char* name){
	
	fprintf(stderr, "USAGE: %s bitsPerBlock < infile.au > outfile.sbc\n", name);
}

int main(int argc, char** argv){

	//read au header (and toss it mostly)
	u32 hdr = readBE32();
	u32 dataOfst = readBE32();
	u32 dataSz = readBE32();
	u32 dataEncoding = readBE32();
	u32 sampleRate = readBE32();
	u32 numChannels = readBE32();
	
	u32 numBlocks, numSubbands, numBits, useSnrMode;
	
	if(hdr != 0x2e736e64) ABT("Not an au file\n");
	if(dataOfst < 24) ABT("Invalid au file data offset\n");
	if(!dataSz) dataSz = 0xffffffff;	//handle that common special case where the value is 0 instead of 0xffffffff
	if(dataEncoding != 3) ABT("Not PCM-16 au file\n");
	if(numChannels != 1) ABT("invalid number of channels %ld\n", numChannels);
	
	BufType samples;
	u8 buf[1024];	//no real frame can be bigger
	u32 i, j, k;
	u8 hdrByte = 0;
	
	if(argc != 2) {
		usage(argv[0]);
		return -1;
	}
	else{
		const char* err = NULL;
		numBlocks = 16;
		numSubbands = 8;
		numBits = atoi(argv[1]);
		useSnrMode = 0;
		
		if(!numBits || numBits >= numSubbands * 16) err = "  bitsPerBlock should be > 0 and < subbandsPerBlock * 16";
		
		if(err){
			usage(argv[0]);
			fprintf(stderr, "%s\n", err);
			return -1;
		}
	}
	
	fprintf(stderr, "processing file with %ld samp/sec using SBC "
		"with %ld-band %ld-bit %ld-block frames\n", sampleRate,
		numSubbands, numBits, numBlocks);
	
	switch(sampleRate){
		case 32000:	hdrByte |= 0x40;	break;
		default:	ABT("invalid sampling rate: %ld\n", sampleRate);
	}
	
	switch(numBlocks){
		case 16:	hdrByte |= 0x30;	break;
		default:	ABT("invalid block num: %ld\n", numBlocks);
	}
		
	if(useSnrMode)	hdrByte |= 0x02;
	
	switch(numSubbands){
		case 8: 	hdrByte |= 0x01;	break;
		default:	ABT("invalid number of subbands: %ld\n", numSubbands);
	}

	bool done = false;
	while(!done) {

		for(i = 0; i < numBlocks; i++) for(j = 0; j < numSubbands; j++){
	
			u32 sam;
	
			if (dataSz < 2 || (sam = readBE16()) > 0xFFFF) {

				sam = 0;
				done = true;
			}
		
			samples[i][j] = sam;
		}
		
		j = sbcenc(samples, buf, numBlocks, numBits, hdrByte);
		
		for(i = 0; i < j; i++) putchar(buf[i]);
	}
	
	return 0;
}

