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

//////////////////////////////////////////////////////////////////////////
// BASICS

int min(int a, int b);
int max(int a, int b);
int clamp(int v, int a, int b);

//////////////////////////////////////////////////////////////////////////
// RANDOM

void rand_init();
int rand_int(int a, int b);
float rand_float(float a, float b);

//////////////////////////////////////////////////////////////////////////
// COLLISION

bool CAT_test_overlap(CAT_ivec2 a_min, CAT_ivec2 a_max, CAT_ivec2 b_min, CAT_ivec2 b_max);
bool CAT_test_contain(CAT_ivec2 a_min, CAT_ivec2 a_max, CAT_ivec2 b_min, CAT_ivec2 b_max);

#endif
