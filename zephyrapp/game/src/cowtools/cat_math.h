#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////
// BASICS

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288f
#endif

#ifndef M_E
#define M_E 2.71828182845904523536028747135266250f
#endif

#define CAT_RAD2DEG (180.0f / M_PI)
#define CAT_DEG2RAD (M_PI / 180.0f)

#define CAT_max(a, b) ((b) > (a) ? (b) : (a))
#define CAT_min(a, b) ((b) < (a) ? (b) : (a))
#define CAT_clamp(v, a, b) ((v) < (a) ? (a) : ((v) > (b) ? (b) : (v)))
#define CAT_sgn(a) ((a) == 0 ? 0 : (a) > 0 ? 1 : -1)
#define CAT_wrap(v, l) (((v) + (l)) % (l))
#define CAT_abs(x) ((x) < 0 ? -(x) : (x))

#define CAT_sin sinf
#define CAT_cos cosf
#define CAT_atan atan2f
#define CAT_sqrt sqrtf
#define CAT_round roundf

float lerp(float a, float b, float t);
float inv_lerp(float t, float a, float b);
int quantize(float t, float range, int steps);


//////////////////////////////////////////////////////////////////////////
// RANDOM

void CAT_rand_seed();
float CAT_rand_uniform();
int CAT_rand_die(int N);
int CAT_rand_coin(float p);

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

typedef struct CAT_ivec2
{
	int32_t x;
	int32_t y;
} CAT_ivec2;

CAT_vec2 CAT_vec2_add(CAT_vec2 a, CAT_vec2 b);
CAT_vec2 CAT_vec2_sub(CAT_vec2 a, CAT_vec2 b);
CAT_vec2 CAT_vec2_mul(CAT_vec2 a, float b);
CAT_vec2 CAT_vec2_div(CAT_vec2 a, float b);

float CAT_vec2_dot(CAT_vec2 a, CAT_vec2 b);
float CAT_vec2_mag2(CAT_vec2 a);
CAT_vec2 CAT_vec2_unit(CAT_vec2 a);
float CAT_vec2_dist2(CAT_vec2 a, CAT_vec2 b);

CAT_vec2 CAT_vec2_rotate(CAT_vec2 a, float t);
CAT_vec2 CAT_vec2_reflect(CAT_vec2 a, CAT_vec2 n);

CAT_ivec2 CAT_ivec2_add(CAT_ivec2 a, CAT_ivec2 b);
CAT_ivec2 CAT_ivec2_sub(CAT_ivec2 a, CAT_ivec2 b);
CAT_ivec2 CAT_ivec2_mul(CAT_ivec2 a, float b);
CAT_ivec2 CAT_ivec2_div(CAT_ivec2 a, float b);

int CAT_ivec2_dot(CAT_ivec2 a, CAT_ivec2 b);
int CAT_ivec2_mag2(CAT_ivec2 a);
float CAT_ivec2_dist2(CAT_ivec2 a, CAT_ivec2 b);

static inline CAT_ivec2 CAT_v2iv(CAT_vec2 a)
{
	return (CAT_ivec2) {a.x, a.y};
}

static inline CAT_vec2 CAT_iv2v(CAT_ivec2 a)
{
	return (CAT_vec2) {a.x, a.y};
}


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
// POLYGON RENDERING

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

CAT_vec4 CAT_matvec_mul(CAT_mat4 M, CAT_vec4 v);
void CAT_perspdiv(CAT_vec4* v);
CAT_vec4 CAT_vec4_cross(CAT_vec4 u, CAT_vec4 v);
float CAT_vec4_dot(CAT_vec4 u, CAT_vec4 v);
CAT_vec4 CAT_vec4_sub(CAT_vec4 u, CAT_vec4 v);
CAT_vec4 CAT_vec4_normalize(CAT_vec4 v);
CAT_mat4 CAT_matmul(CAT_mat4 A, CAT_mat4 B);
CAT_mat4 CAT_rotmat(float x, float y, float z);
bool CAT_is_clipped(CAT_vec4 v);


//////////////////////////////////////////////////////////////////////////
// COHEN-SUTHERLAND CLIPPING

typedef enum
{
	CAT_CSCLIP_FLAG_INSIDE = 0,
	CAT_CSCLIP_FLAG_LEFT = 1,
	CAT_CSCLIP_FLAG_RIGHT = 2,
	CAT_CSCLIP_FLAG_BOTTOM = 4,
	CAT_CSCLIP_FLAG_TOP = 8
} CAT_CSCLIP_flag;

void CAT_CSCLIP_set_rect(int x0, int y0, int x1, int y1);
int CAT_CSCLIP_get_flags(int x, int y);
bool CAT_CSCLIP(int* x0, int* y0, int* x1, int* y1);


//////////////////////////////////////////////////////////////////////////
// WEIGHTED RANDOM SELECTION

void CAT_WRS_begin();
void CAT_WRS_add(int idx, uint8_t weight);
void CAT_WRS_end();
int CAT_WRS_select();


//////////////////////////////////////////////////////////////////////////
// STRUCT-FREE

bool CAT_rect_contains_rect(int x00, int y00, int x01, int y01, int x10, int y10, int x11, int y11);
bool CAT_rect_rect_touching(int x00, int y00, int x01, int y01, int x10, int y10, int x11, int y11);
bool CAT_rect_rect_intersecting(int x00, int y00, int x01, int y01, int x10, int y10, int x11, int y11);
bool CAT_rect_point_touching(int x0, int y0, int x1, int y1, int x, int y);

int CAT_i2_dot(int ax, int ay, int bx, int by);
int CAT_i2_cross(int ax, int ay, int bx, int by);
