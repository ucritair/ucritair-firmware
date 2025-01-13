#include <stdint.h>
#include <math.h>

typedef union
{
	struct
	{
		float x;
		float y;
	};

	float data[2];
} CAT_vec2;

static inline CAT_vec2 CAT_v2(float x, float y)
{
	CAT_vec2 result;
	result.x = x;
	result.y = y;
	return result;
}

static inline CAT_vec2 CAT_v2_add(CAT_vec2 a, CAT_vec2 b)
{
	CAT_vec2 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

static inline CAT_vec2 CAT_v2_sub(CAT_vec2 a, CAT_vec2 b)
{
	CAT_vec2 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

static inline CAT_vec2 CAT_v2_mul(CAT_vec2 a, float b)
{
	CAT_vec2 result;
	result.x = b * a.x;
	result.y = b * a.y;
	return result;
}

static inline CAT_vec2 CAT_v2_div(CAT_vec2 a, float b)
{
	CAT_vec2 result;
	result.x = a.x / b;
	result.y = a.y / b;
	return result;
}

static inline float CAT_v2_dot(CAT_vec2 a, CAT_vec2 b)
{
	return a.x * b.x + a.y * b.y;
}

static inline float CAT_v2_cross(CAT_vec2 a, CAT_vec2 b)
{
	return a.x * b.y - a.y * b.x;
}

typedef union
{
	struct
	{
		CAT_vec2 min;
		CAT_vec2 max;
	};

	struct
	{
		float l;
		float b;
		float r;
		float t;
	};

	float data[4];
} CAT_rectangle;

static inline CAT_rectangle CAT_rect(float l, float b, float r, float t)
{
	CAT_rectangle result;
	result.min = CAT_v2(l, b);
	result.max = CAT_v2(r, t);
	return result;
}




