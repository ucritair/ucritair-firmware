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

static inline CAT_rectangle CAT_rect_shift(CAT_rectangle r, CAT_vec2 v)
{
	r.min = CAT_v2_add(r.min, v);
	r.max = CAT_v2_add(r.max, v);
}

static inline bool CAT_rect_point(CAT_rectangle r, CAT_vec2 p)
{
	if(p.x < r.l || p.x > r.r)
		return false;
	if(p.y < r.b || p.y > r.t)
		return false;
	return true;
}

static inline bool CAT_rect_rect(CAT_rectangle a, CAT_rectangle b)
{
	if(a.r < b.l || a.l > b.r)
		return false;
	if(a.t < b.b || a.b > b.t)
		return false;
	return true;
}




