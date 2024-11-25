#pragma once

#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
// BASICS

int min(int a, int b);
int max(int a, int b);
int clamp(int v, int a, int b);
float lerp(float a, float b, float t);
float inv_lerp(float t, float a, float b);
float minf(float a, float b);
float maxf(float a, float b);
float clampf(float v, float a, float b);
int quantize(float t, float range, int steps);


//////////////////////////////////////////////////////////////////////////
// RANDOM

int CAT_rand_int(int a, int b);
float CAT_rand_float(float a, float b);


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

float CAT_vec2_dot(CAT_vec2 a, CAT_vec2 b);
float CAT_vec2_mag2(CAT_vec2 a);

CAT_vec2 CAT_vec2_unit(CAT_vec2 a);
float CAT_vec2_dist2(CAT_vec2 a, CAT_vec2 b);


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
// COLLISION

typedef struct CAT_rect
{
	CAT_ivec2 min;
	CAT_ivec2 max;
} CAT_rect;

CAT_rect CAT_rect_place(CAT_ivec2 start, CAT_ivec2 shape);
bool CAT_rect_overlaps(CAT_rect a, CAT_rect b);
bool CAT_rect_contains(CAT_rect a, CAT_rect b);

typedef struct CAT_quadtree
{
	
} CAT_quadtree;
