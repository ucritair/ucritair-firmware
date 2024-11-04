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

CAT_ivec2 CAT_ivec2_sub(CAT_ivec2 a, CAT_ivec2 b)
{
	return (CAT_ivec2) {a.x - b.x, a.y - b.y};
}

CAT_ivec2 CAT_ivec2_mul(CAT_ivec2 a, int b)
{
	return (CAT_ivec2) {a.x * b, a.y * b};
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

float inv_lerp(float t, float a, float b)
{
	return (t-a) / (b-a);
}

//////////////////////////////////////////////////////////////////////////
// RANDOM

void CAT_rand_init()
{
#ifdef CAT_DESKTOP
	srand(time(NULL));
#endif
}

int CAT_rand_int(int a, int b)
{
	return a + rand() / (RAND_MAX / (b - a + 1) + 1);
}

float CAT_rand_float(float a, float b)
{
	float scale = rand() / (float) RAND_MAX;
	return a + scale * (b-a);
}

CAT_vec2 CAT_rand_vec2(CAT_vec2 min, CAT_vec2 max)
{
	return (CAT_vec2) {CAT_rand_float(min.x, max.x), CAT_rand_float(min.y, max.y)};
}

CAT_ivec2 CAT_rand_ivec2(CAT_ivec2 min, CAT_ivec2 max)
{
	return (CAT_ivec2) {CAT_rand_int(min.x, max.x), CAT_rand_int(min.y, max.y)};
}

bool CAT_rand_chance(float n)
{
	float thresh = 1.0f/n;
	return CAT_rand_float(0, 1) < thresh;
}

//////////////////////////////////////////////////////////////////////////
// COLLISION

CAT_rect CAT_rect_place(CAT_ivec2 start, CAT_ivec2 shape)
{
	return (CAT_rect) {start, CAT_ivec2_add(start, shape)};
}

bool CAT_test_overlaps(CAT_rect a, CAT_rect b)
{
	if(a.min.x >= b.max.x || a.max.x <= b.min.x)
		return false;
	if(a.min.y >= b.max.y || a.max.y <= b.min.y)
		return false;
	return true;
}

bool CAT_test_contains(CAT_rect a, CAT_rect b)
{
	if(b.min.x < a.min.x || b.max.x > a.max.x)
		return false;
	if(b.min.y < a.min.y || b.max.y > a.max.y)
		return false;
	return true;
}

bool CAT_test_pt_rect(CAT_ivec2 v, CAT_rect r)
{
	if(v.x < r.min.x || v.x > r.max.x)
		return false;
	if(v.y < r.min.y || v.y > r.max.y)
		return false;
	return true;
}

CAT_ivec2 CAT_clamp_pt_rect(CAT_ivec2 v, CAT_rect r)
{
	v.x = clamp(v.x, r.min.x, r.max.x);
	v.y = clamp(v.y, r.min.y, r.max.y);
	return v;
}

//////////////////////////////////////////////////////////////////////////
// CONVERSION

CAT_vec2 CAT_iv2v(CAT_ivec2 iv)
{
	return (CAT_vec2) {(float) iv.x, (float) iv.y };
}
CAT_ivec2 CAT_v2iv(CAT_vec2 v)
{
	return (CAT_ivec2) {(int) v.x, (int) v.y };
}


