#pragma once

#include <stdbool.h>
#include <stdint.h>

//////////////////////////////////////////////////////////////////////////
// BASICS

#ifndef M_PI
#define M_PI 3.14159265359
#endif

#ifndef max
#define max(a, b) ((b) > (a) ? (b) : (a))
#endif

#ifndef min
#define min(a, b) ((b) < (a) ? (b) : (a))
#endif

#ifndef clamp
#define clamp(v, a, b) ((v) < (a) ? (a) : ((v) > (b) ? (b) : (v)))
#endif

float lerp(float a, float b, float t);
float inv_lerp(float t, float a, float b);
float minf(float a, float b);
float maxf(float a, float b);
float clampf(float v, float a, float b);
int quantize(float t, float range, int steps);


//////////////////////////////////////////////////////////////////////////
// RANDOM

void CAT_rand_seed();
int CAT_rand_int(int a, int b);
float CAT_rand_float(float a, float b);
bool CAT_rand_chance(int N);


//////////////////////////////////////////////////////////////////////////
// VEC2

typedef struct CAT_vec2
{
	float x;
	float y;
} CAT_vec2;

CAT_vec2 CAT_vec2_add(CAT_vec2 a, CAT_vec2 b);
CAT_vec2 CAT_vec2_sub(CAT_vec2 a, CAT_vec2 b);
CAT_vec2 CAT_vec2_mul(CAT_vec2 a, float b);
CAT_vec2 CAT_vec2_div(CAT_vec2 a, float b);

float CAT_vec2_dot(CAT_vec2 a, CAT_vec2 b);
float CAT_vec2_mag2(CAT_vec2 a);
CAT_vec2 CAT_vec2_unit(CAT_vec2 a);
float CAT_vec2_dist2(CAT_vec2 a, CAT_vec2 b);


//////////////////////////////////////////////////////////////////////////
// IVEC2

typedef struct CAT_ivec2
{
	int32_t x;
	int32_t y;
} CAT_ivec2;

CAT_ivec2 CAT_ivec2_add(CAT_ivec2 a, CAT_ivec2 b);
CAT_ivec2 CAT_ivec2_sub(CAT_ivec2 a, CAT_ivec2 b);
CAT_ivec2 CAT_ivec2_mul(CAT_ivec2 a, float b);
CAT_ivec2 CAT_ivec2_div(CAT_ivec2 a, float b);

int CAT_ivec2_dot(CAT_ivec2 a, CAT_ivec2 b);
int CAT_ivec2_mag2(CAT_ivec2 a);
float CAT_ivec2_dist2(CAT_ivec2 a, CAT_ivec2 b);


//////////////////////////////////////////////////////////////////////////
// COLLISION

typedef struct CAT_rect
{
	CAT_ivec2 min;
	CAT_ivec2 max;
} CAT_rect;

CAT_rect CAT_rect_place(CAT_ivec2 start, CAT_ivec2 shape);
bool CAT_rect_overlaps(CAT_rect a, CAT_rect b);
bool CAT_rect_contains(CAT_rect a, CAT_rect b);
CAT_rect CAT_rect_center(int x, int y, int w, int h);
CAT_rect CAT_rect_overlap(CAT_rect a, CAT_rect b);


//////////////////////////////////////////////////////////////////////////
// RENDERING

#define RGB8882565(r, g, b) ((((r) & 0b11111000) << 8) | (((g) & 0b11111100) << 3) | ((b) >> 3))
#define RGB5652BGR565(c) (((c) >> 8) | (((c) & 0xff) << 8))
#define SCALEBYTE(b, f) ((uint8_t) ((b) * (f)))

typedef struct CAT_mat4
{
	float data[16];
} CAT_mat4;

typedef struct CAT_vec4
{
	float x;
	float y;
	float z;
	float w;
} CAT_vec4;

CAT_mat4 CAT_mat4_id();
CAT_vec4 CAT_matvec_mul(CAT_mat4 M, CAT_vec4 v);
void CAT_perspdiv(CAT_vec4* v);
CAT_vec4 CAT_vec4_cross(CAT_vec4 u, CAT_vec4 v);
float CAT_vec4_dot(CAT_vec4 u, CAT_vec4 v);
CAT_vec4 CAT_vec4_sub(CAT_vec4 u, CAT_vec4 v);
CAT_vec4 CAT_vec4_normalize(CAT_vec4 v);
CAT_mat4 CAT_matmul(CAT_mat4 A, CAT_mat4 B);
CAT_mat4 CAT_rotmat(float x, float y, float z);
bool CAT_is_clipped(CAT_vec4 v);
CAT_vec4 CAT_centroid(CAT_vec4 a, CAT_vec4 b, CAT_vec4 c);
CAT_vec4 CAT_vec4_add(CAT_vec4 u, CAT_vec4 v);
CAT_vec4 CAT_vec4_mul(CAT_vec4 v, float l);