#include "cat_math.h"

#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
// BASICS

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

void CAT_rand_seed()
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

bool CAT_rand_chance(int N)
{
	float thresh = 1.0f / (float) N;
	return CAT_rand_float(0.0, 1.0f) <= thresh;
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

CAT_vec2 CAT_vec2_div(CAT_vec2 a, float b)
{
	return (CAT_vec2) {a.x/b, a.y/b};
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

CAT_ivec2 CAT_ivec2_mul(CAT_ivec2 a, float b)
{
	return (CAT_ivec2) {a.x * b, a.y * b};
}

CAT_ivec2 CAT_ivec2_div(CAT_ivec2 a, float b)
{
	return (CAT_ivec2) {a.x / b, a.y / b};
}

int CAT_ivec2_dot(CAT_ivec2 a, CAT_ivec2 b)
{
	return a.x*b.x + a.y*b.y;
}
int CAT_ivec2_mag2(CAT_ivec2 a)
{
	return CAT_ivec2_dot(a, a);
}

float CAT_ivec2_dist2(CAT_ivec2 a, CAT_ivec2 b)
{
	return CAT_ivec2_mag2(CAT_ivec2_sub(b, a));
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

CAT_rect CAT_rect_center(int x, int y, int w, int h)
{
	CAT_rect rect;
	rect.min.x = x - w/2;
	rect.min.y = y - h/2;
	rect.max.x = x + w/2;
	rect.max.y = y + h/2;
	return rect;
}

CAT_rect CAT_rect_overlap(CAT_rect a, CAT_rect b)
{
	return (CAT_rect)
	{
		{ max(a.min.x, b.min.x), max(a.min.y, b.min.y) },
		{ min(a.max.x, b.max.x), min(a.max.y, b.max.y) }
	};
}


//////////////////////////////////////////////////////////////////////////
// RENDERING

CAT_mat4 CAT_mat4_id()
{
	return (CAT_mat4)
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
}

CAT_vec4 CAT_matvec_mul(CAT_mat4 M, CAT_vec4 v)
{
	return (CAT_vec4)
	{
		M.data[0] * v.x + M.data[1] * v.y + M.data[2] * v.z + M.data[3] * v.w,
		M.data[4] * v.x + M.data[5] * v.y + M.data[6] * v.z + M.data[7] * v.w,
		M.data[8] * v.x + M.data[9] * v.y + M.data[10] * v.z + M.data[11] * v.w,
		M.data[12] * v.x + M.data[13] * v.y + M.data[14] * v.z + M.data[15] * v.w
	};
}

void CAT_perspdiv(CAT_vec4* v)
{
	v->x /= v->w;
	v->y /= v->w;
	v->z /= v->w;
	v->w /= v->w;
}

CAT_vec4 CAT_vec4_cross(CAT_vec4 u, CAT_vec4 v)
{
	return (CAT_vec4) {u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x, 0.0f};
}

float CAT_vec4_dot(CAT_vec4 u, CAT_vec4 v)
{
	return u.x * v.x + u.y * v.y + u.z * v.z + u.w * v.w;
}

CAT_vec4 CAT_vec4_sub(CAT_vec4 u, CAT_vec4 v)
{
	return (CAT_vec4) {u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w};
}

CAT_vec4 CAT_vec4_normalize(CAT_vec4 v)
{
	float len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
	float s = 1.0f / len;
	return (CAT_vec4) {v.x * s, v.y * s, v.z * s, v.w * s};
}

CAT_mat4 CAT_matmul(CAT_mat4 A, CAT_mat4 B)
{
	CAT_mat4 C;
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			C.data[i * 4 + j] =
			A.data[i * 4 + 0] * B.data[0 * 4 + j] +
			A.data[i * 4 + 1] * B.data[1 * 4 + j] +
			A.data[i * 4 + 2] * B.data[2 * 4 + j] +
			A.data[i * 4 + 3] * B.data[3 * 4 + j];
		}
	}
	return C;
}

CAT_mat4 CAT_rotmat(float x, float y, float z)
{
	CAT_mat4 X =
	{
		1, 0, 0, 0,
		0, cos(x), -sin(x), 0,
		0, sin(x), cos(x), 0,
		0, 0, 0, 1
	};

	CAT_mat4 Y =
	{
		cos(y), 0, sin(y), 0,
		0, 1, 0, 0,
		-sin(y), 0, cos(y), 0,
		0, 0, 0, 1
	};

	CAT_mat4 Z =
	{
		cos(z), -sin(z), 0, 0,
		sin(z), cos(z), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	return CAT_matmul(Z, CAT_matmul(Y, X));
}

bool CAT_is_clipped(CAT_vec4 v)
{
	if(v.x < -v.w || v.x > v.w)
		return true;
	if(v.y < -v.w || v.y > v.w)
		return true;
	if(v.z < 0 || v.z > v.w)
		return true;
	return false;
}

CAT_vec4 CAT_centroid(CAT_vec4 a, CAT_vec4 b, CAT_vec4 c)
{
	float x = (a.x + b.x + c.x) / 3.0f;
	float y = (a.y + b.y + c.y) / 3.0f;
	float z = (a.z + b.z + c.z) / 3.0f;
	return (CAT_vec4) {x, y, z, 1.0f};
}

CAT_vec4 CAT_vec4_add(CAT_vec4 u, CAT_vec4 v)
{
	return (CAT_vec4) {u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w};
}

CAT_vec4 CAT_vec4_mul(CAT_vec4 v, float l)
{
	return (CAT_vec4) {l * v.x, l * v.y, l * v.z, l * v.w};
}