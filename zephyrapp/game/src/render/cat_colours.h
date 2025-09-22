#pragma once

#define CAT_RGB8882565(r, g, b) ((((r) & 0b11111000) << 8) | (((g) & 0b11111100) << 3) | ((b) >> 3))
#define CAT_RGB5652BGR565(c) (((c) >> 8) | (((c) & 0xff) << 8))

static void CAT_RGB5652888(uint16_t c, uint8_t* r, uint8_t* g, uint8_t* b)
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

static uint16_t CAT_colour_lerp(uint16_t c0, uint16_t c1, float t)
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

static uint16_t CAT_colour_curve(uint16_t* points, uint8_t length, float t)
{
	float tl = t * (length-1);
	uint8_t idx_a = ((int) tl) % length;
	float frac = tl - idx_a;
	uint8_t idx_b = (idx_a + 1) % length;
	uint16_t a = points[idx_a];
	uint16_t b = points[idx_b];
	return CAT_colour_lerp(a, b, frac);
}

#define CAT_TRANSPARENT 0xDEAD
#define CAT_BLACK 0x0000
#define CAT_WHITE 0xFFFF
#define CAT_RED 0b1111100000000000
#define CAT_GREEN 0b0000011111100000
#define CAT_BLUE 0b0000000000011111
#define CAT_GREY 0x8410

#define CAT_32_GREY CAT_RGB8882565(32, 32, 32)
#define CAT_64_GREY CAT_RGB8882565(64, 64, 64)
#define CAT_96_GREY CAT_RGB8882565(96, 96, 96)
#define CAT_128_GREY CAT_RGB8882565(128, 128, 128)
#define CAT_160_GREY CAT_RGB8882565(160, 160, 160)
#define CAT_192_GREY CAT_RGB8882565(192, 192, 192)

#define CAT_PAPER_CREAM 0xEF39
#define CAT_SKY_BLUE CAT_RGB8882565(35, 157, 235)
#define CAT_GRASS_GREEN 36144

#define CAT_CRISIS_RED 0xea01
#define CAT_CRISIS_YELLOW 0xfd45
#define CAT_CRISIS_GREEN 0x5d6d

#define CAT_VIGOUR_ORANGE CAT_RGB8882565(223, 64, 47)
#define CAT_FOCUS_BLUE CAT_RGB8882565(102, 181, 179)
#define CAT_SPIRIT_PURPLE CAT_RGB8882565(129, 91, 152)

#define CAT_GRADE_COLOUR_BAD 0xf8e3
#define CAT_GRADE_COLOUR_MID 0x9abd
#define CAT_GRADE_COLOUR_GOOD CAT_SKY_BLUE

#define CAT_GRAPH_BG CAT_RGB8882565(32, 32, 32)
#define CAT_GRAPH_FG CAT_RGB8882565(192, 192, 192)