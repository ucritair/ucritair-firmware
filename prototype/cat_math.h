#ifndef CAT_MATH_H
#define CAT_MATH_H

#include <time.h>
#include <stdlib.h>
#include <math.h>

typedef struct CAT_vec2
{
	float x;
	float y;
} CAT_vec2;

CAT_vec2 CAT_vec2_add(CAT_vec2 a, CAT_vec2 b)
{
	return {a.x+b.x, a.y+b.y};
}

CAT_vec2 CAT_vec2_sub(CAT_vec2 a, CAT_vec2 b)
{
	return {a.x-b.x, a.y-b.y};
}

CAT_vec2 CAT_vec2_mul(CAT_vec2 a, float b)
{
	return {a.x*b, a.y*b};
}

CAT_vec2 CAT_vec2_div(CAT_vec2 a, float b)
{
	return {a.x/b, a.y/b};
}

CAT_vec2 CAT_vec2_neg(CAT_vec2 a)
{
	return {-a.x, -a.y};
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
	return {a.x*inv_mag, a.y*inv_mag};
}

float CAT_vec2_dist2(CAT_vec2 a, CAT_vec2 b)
{
	return CAT_vec2_mag2(CAT_vec2_sub(b, a));
}

float CAT_vec2_dist(CAT_vec2 a, CAT_vec2 b)
{
	return CAT_vec2_mag(CAT_vec2_sub(b, a));
}

typedef struct CAT_ivec2
{
	int x;
	int y;
} CAT_ivec2;

float lerp(float a, float b, float t)
{
	return a * (1-t) + b * t;
}

void rand_init()
{
	srand(time(NULL));
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

int rand_chance(int n)
{
	return rand() < (RAND_MAX + 1u) / n;
}

#endif
