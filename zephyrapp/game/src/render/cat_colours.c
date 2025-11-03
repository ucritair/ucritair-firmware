#include "cat_colours.h"

void CAT_RGB5652888(uint16_t c, uint8_t* r, uint8_t* g, uint8_t* b)
{
	uint16_t r5 = (c & 0b1111100000000000) >> 11;
	uint16_t g6 = (c & 0b0000011111100000) >> 5;
	uint16_t b5 = c & 0b0000000000011111;

	uint8_t r8 = (r5 * 527 + 23) >> 6;
	uint8_t g8 = (g6 * 259 + 33) >> 6;
	uint8_t b8 = (b5 * 527 + 23) >> 6;

	*r = r8;
	*g = g8;
	*b = b8;
}

uint16_t CAT_colour_lerp(uint16_t c0, uint16_t c1, float t)
{
	uint8_t r0, g0, b0;
	CAT_RGB5652888(c0, &r0, &g0, &b0);
	uint8_t r1, g1, b1;
	CAT_RGB5652888(c1, &r1, &g1, &b1);

	uint8_t r = (1.0f-t) * (float) r0 + t * (float) r1;
	uint8_t g = (1.0f-t) * (float) g0 + t * (float) g1;
	uint8_t b = (1.0f-t) * (float) b0 + t * (float) b1;

	return CAT_RGB8882565(r, g, b);
}

uint16_t CAT_colour_curve(uint16_t* points, uint8_t length, float t)
{
	float tl = t * (length-1);
	uint8_t idx_a = ((int) tl) % length;
	float frac = tl - idx_a;
	uint8_t idx_b = (idx_a + 1) % length;
	uint16_t a = points[idx_a];
	uint16_t b = points[idx_b];
	return CAT_colour_lerp(a, b, frac);
}