#ifndef CAT_MATH_H
#define CAT_MATH_H

#include <stdbool.h>

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
CAT_vec2 CAT_vec2_neg(CAT_vec2 a);
float CAT_vec2_dot(CAT_vec2 a, CAT_vec2 b);
float CAT_vec2_mag2(CAT_vec2 a);
float CAT_vec2_mag(CAT_vec2 a);
CAT_vec2 CAT_vec2_unit(CAT_vec2 a);
float CAT_vec2_dist2(CAT_vec2 a, CAT_vec2 b);
float CAT_vec2_dist(CAT_vec2 a, CAT_vec2 b);

//////////////////////////////////////////////////////////////////////////
// IVEC2

typedef struct CAT_ivec2
{
	int x;
	int y;
} CAT_ivec2;

CAT_ivec2 CAT_ivec2_add(CAT_ivec2 a, CAT_ivec2 b);
CAT_ivec2 CAT_ivec2_sub(CAT_ivec2 a, CAT_ivec2 b);
CAT_ivec2 CAT_ivec2_mul(CAT_ivec2 a, int b);

//////////////////////////////////////////////////////////////////////////
// BASICS

int min(int a, int b);
int max(int a, int b);
int clamp(int v, int a, int b);

//////////////////////////////////////////////////////////////////////////
// RANDOM

void CAT_rand_init();
int CAT_rand_int(int a, int b);
float CAT_rand_float(float a, float b);
CAT_vec2 CAT_rand_vec2(CAT_vec2 min, CAT_vec2 max);
CAT_ivec2 CAT_rand_ivec2(CAT_ivec2 min, CAT_ivec2 max);
bool CAT_rand_chance(float n);

//////////////////////////////////////////////////////////////////////////
// COLLISION

typedef struct CAT_rect
{
	CAT_ivec2 min;
	CAT_ivec2 max;
} CAT_rect;

CAT_rect CAT_rect_place(CAT_ivec2 start, CAT_ivec2 shape);
bool CAT_test_overlaps(CAT_rect a, CAT_rect b);
bool CAT_test_contains(CAT_rect a, CAT_rect b);
bool CAT_test_pt_rect(CAT_ivec2 v, CAT_rect r);
CAT_ivec2 CAT_clamp_pt_rect(CAT_ivec2 v, CAT_rect r);

//////////////////////////////////////////////////////////////////////////
// CONVERSION

CAT_vec2 CAT_iv2v(CAT_ivec2 iv);
CAT_ivec2 CAT_v2iv(CAT_vec2 v);

#endif
