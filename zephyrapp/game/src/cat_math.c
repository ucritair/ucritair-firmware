#include "cat_math.h"

#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

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

float lerp(float a, float b, float t)
{
	return (1.0f-t) * a + t * b;
}

float inv_lerp(float t, float a, float b)
{
	return (t-a) / (b-a);
}

float minf(float a, float b)
{
	return a < b ? a : b;
}

float maxf(float a, float b)
{
	return a > b ? a : b;
}

float clampf(float v, float a, float b)
{
	return minf(maxf(v, a), b);
}

int quantize(float t, float range, int steps)
{
	return clamp(round((t / range) * (float) (steps - 1)), 0, steps - 1);
}


//////////////////////////////////////////////////////////////////////////
// RANDOM

int CAT_rand_int(int a, int b)
{
	return a + rand() / (RAND_MAX / (b - a + 1) + 1);
}

float CAT_rand_float(float a, float b)
{
	float scale = rand() / (float) RAND_MAX;
	return a + scale * (b-a);
}


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

float CAT_vec2_dot(CAT_vec2 a, CAT_vec2 b)
{
	return a.x*b.x + a.y*b.y;
}

float CAT_vec2_mag2(CAT_vec2 a)
{
	return a.x*a.x + a.y*a.y;
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
// COLLISION

CAT_rect CAT_rect_place(CAT_ivec2 start, CAT_ivec2 shape)
{
	return (CAT_rect) {start, CAT_ivec2_add(start, shape)};
}

bool CAT_rect_overlaps(CAT_rect a, CAT_rect b)
{
	if(a.min.x >= b.max.x || a.max.x <= b.min.x)
		return false;
	if(a.min.y >= b.max.y || a.max.y <= b.min.y)
		return false;
	return true;
}

bool CAT_rect_contains(CAT_rect a, CAT_rect b)
{
	if(b.min.x < a.min.x || b.max.x > a.max.x)
		return false;
	if(b.min.y < a.min.y || b.max.y > a.max.y)
		return false;
	return true;
}


//////////////////////////////////////////////////////////////////////////
// RENDERING

CAT_vec4 CAT_mvmul(CAT_mat4 M, CAT_vec4 v)
{
	return (CAT_vec4)
	{
		M.data[0] * v.x + M.data[1] * v.y + M.data[2] * v.z + M.data[3] * v.w,
		M.data[4] * v.x + M.data[5] * v.y + M.data[6] * v.z + M.data[7] * v.w,
		M.data[8] * v.x + M.data[9] * v.y + M.data[10] * v.z + M.data[11] * v.w,
		M.data[12] * v.x + M.data[13] * v.y + M.data[14] * v.z + M.data[15] * v.w
	};
}