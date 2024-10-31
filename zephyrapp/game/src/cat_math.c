#include "cat_math.h"

#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
// VEC2

CAT_vec2 CAT_vec2_add(CAT_vec2 a, CAT_vec2 b)
{
	return (CAT_vec2) {a.x+b.x, a.y+b.y};
}

CAT_vec2 CAT_vec2_sub(CAT_vec2 a, CAT_vec2 b)
{
	return (CAT_vec2) {a.x-b.x, a.y-b.y};
}

CAT_vec2 CAT_vec2_mul(CAT_vec2 a, float b)
{
	return (CAT_vec2) {a.x*b, a.y*b};
}

CAT_vec2 CAT_vec2_div(CAT_vec2 a, float b)
{
	return (CAT_vec2) {a.x/b, a.y/b};
}

CAT_vec2 CAT_vec2_neg(CAT_vec2 a)
{
	return (CAT_vec2) {-a.x, -a.y};
}

float CAT_vec2_dot(CAT_vec2 a, CAT_vec2 b)
{
	return a.x*b.x + a.y*b.y;
}

float CAT_vec2_mag2(CAT_vec2 a)
{
	return a.x*a.x + a.y*a.y;
}

float CAT_vec2_mag(CAT_vec2 a)
{
	return sqrt(a.x*a.x + a.y*a.y);
}

CAT_vec2 CAT_vec2_unit(CAT_vec2 a)
{
	float inv_mag = 1.0f/sqrt(a.x*a.x + a.y*a.y);
	return (CAT_vec2) {a.x*inv_mag, a.y*inv_mag};
}

float CAT_vec2_dist2(CAT_vec2 a, CAT_vec2 b)
{
	return CAT_vec2_mag2(CAT_vec2_sub(b, a));
}

float CAT_vec2_dist(CAT_vec2 a, CAT_vec2 b)
{
	return CAT_vec2_mag(CAT_vec2_sub(b, a));
}

//////////////////////////////////////////////////////////////////////////
// IVEC2

CAT_ivec2 CAT_ivec2_add(CAT_ivec2 a, CAT_ivec2 b)
{
	return (CAT_ivec2) {a.x + b.x, a.y + b.y};
}

//////////////////////////////////////////////////////////////////////////
// BASICS

int min(int a, int b)
{
	return b < a ? b : a;
}

int max(int a, int b)
{
	return b > a ? b : a;
}

int clamp(int v, int a, int b)
{
	return min(max(v, a), b);
}

//////////////////////////////////////////////////////////////////////////
// RANDOM

void rand_init()
{
#ifdef CAT_DESKTOP
	srand(time(NULL));
#endif
}

int rand_int(int a, int b)
{
	return a + rand() / (RAND_MAX / (b - a + 1) + 1);
}

float rand_float(float a, float b)
{
	float scale = rand() / (float) RAND_MAX;
	return a + scale * (b-a);
}

CAT_vec2 rand_vec2(CAT_vec2 min, CAT_vec2 max)
{
	return (CAT_vec2) {rand_float(min.x, max.x), rand_float(min.y, max.y)};
}

CAT_ivec2 rand_ivec2(CAT_ivec2 min, CAT_ivec2 max)
{
	return (CAT_ivec2) {rand_int(min.x, max.x), rand_int(min.y, max.y)};
}

//////////////////////////////////////////////////////////////////////////
// COLLISION

bool CAT_test_overlap(CAT_ivec2 a_min, CAT_ivec2 a_max, CAT_ivec2 b_min, CAT_ivec2 b_max)
{
	if(a_min.x >= b_max.x || a_max.x <= b_min.x)
		return false;
	if(a_min.y >= b_max.y || a_max.y <= b_min.y)
		return false;
	return true;
}

bool CAT_test_contain(CAT_ivec2 a_min, CAT_ivec2 a_max, CAT_ivec2 b_min, CAT_ivec2 b_max)
{
	if(b_min.x < a_min.x || b_max.x > a_max.x)
		return false;
	if(b_min.y < a_min.y || b_max.y > a_max.y)
		return false;
	return true;
}

