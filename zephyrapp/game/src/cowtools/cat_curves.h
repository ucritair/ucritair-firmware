#pragma once

#include <math.h>

//////////////////////////////////////////////////////////////////////////
// EASING CURVES

static inline float CAT_ease_in_sine(float t)
{
	return 1 - cos(0.5f * M_PI * t);
}

static inline float CAT_ease_out_sine(float t)
{
	return sin(0.5f * M_PI * t);
}

static inline float CAT_ease_inout_sine(float t)
{
	return -cos(M_PI * t - 1) * 0.5f;
}

static inline float CAT_ease_in_quad(float t)
{
	return t*t;
}

static inline float CAT_ease_out_quad(float t)
{
	return 1 - (1-t)*(1-t);
}

static inline float CAT_ease_inout_quad(float t)
{
	return
	t < 0.5f ?
	2 * t*t :
	1 - (-2*t+2) * (-2*t+2) * 0.5f;
}

static inline float CAT_ease_in_cubic(float t)
{
	return t*t*t;
}

static inline float CAT_ease_out_cubic(float t)
{
	return 1 - pow(1-t, 3);
}

static inline float CAT_ease_inout_cubic(float t)
{
	return
	t < 0.5f ?
	4 * t*t*t :
	1 - pow(-2*t+2, 3) * 0.5f;
}

static inline float CAT_ease_in_quart(float t)
{
	return t*t*t*t;
}

static inline float CAT_ease_out_quart(float t)
{
	return 1 - pow(1-t, 4);
}

static inline float CAT_ease_inout_quart(float t)
{
	return t < 0.5f ?
	8 * t*t*t*t :
	1 - pow(-2*t+2, 4) * 0.5f;
}

static inline float CAT_ease_in_quint(float t)
{
	return t*t*t*t*t;
}

static inline float CAT_ease_out_quint(float t)
{
	return 1 - pow(1-t, 5);
}

static inline float CAT_ease_inout_quint(float t)
{
	return t < 0.5f ?
	16 * t*t*t*t*t :
	1 - pow(-2*t+2, 5) * 0.5f;
}

static inline float CAT_ease_in_expo(float t)
{
	return
	t == 0 ? 0 :
	pow(2, 10*t-10);
}

static inline float CAT_ease_out_expo(float t)
{
	return
	t == 1 ? 1 :
	1 - pow(2, -10*t);
}

static inline float CAT_ease_inout_expo(float t)
{
	return
	t == 0 ? 0 :
	t == 1 ? 1 :
	t < 0.5f ?
	pow(2, 20*t-10) * 0.5f :
	(2 - pow(2, -20*t+10)) * 0.5f;
}

static inline float CAT_ease_in_circ(float t)
{
	return 1 - sqrt(1 - t*t);
}

static inline float CAT_ease_out_circ(float t)
{
	return sqrt(1 - (t-1)*(t-1));
}

static inline float CAT_ease_inout_circ(float t)
{
	return
	t < 0.5f ?
	(1 - sqrt(1 - (2*t)*(2*t))) * 0.5f :
	(sqrt(1 - (-2*t+2)*(-2*t+2)) + 1) * 0.5f;
}

static inline float CAT_ease_in_back(float t)
{
	const float c1 = 1.70158f;
	const float c2 = c1 + 1;
	return c2*t*t*t - c1*t*t;
}

static inline float CAT_ease_out_back(float t)
{
	const float c1 = 1.70158f;
	const float c2 = c1 + 1;
	return 1 + c2*pow(t-1, 3) + c1*pow(t-1, 2);
}

static inline float CAT_ease_inout_back(float t)
{
	const float c1 = 1.70158f;
	const float c2 = c1 + 1.525f;
	return
	t < 0.5f ?
	(2*t)*(2*t) * ((c2+1) * 2*t - c2) * 0.5f :
	((2*t-2)*(2*t-2) * ((c2+1) * (2*t-2) + c2) + 2) * 0.5f;
}

static inline float CAT_ease_in_elastic(float t)
{
	const float c = 2*M_PI / 3.0f;
	return
	t == 0 ? 0 :
	t == 1 ? 1 :
	-pow(2, 10*t-10) * sin(c*(t*10-10.75f));
}

static inline float CAT_ease_out_elastic(float t)
{
	const float c = 2*M_PI / 3.0f;
	return
	t == 0 ? 0 :
	t == 1 ? 1 :
	pow(2, -10*t) * sin(c*(t*10-0.75)) + 1;
}

static inline float CAT_ease_inout_elastic(float t)
{
	const float c = 2*M_PI / 4.5f;
	return
	t == 0 ? 0 :
	t == 1 ? 1 :
	t < 0.5f ?
	-pow(2, 20*t-10) * sin(c*(20*t-11.125)) * 0.5f :
	pow(2, -20*t+10) * sin(c*(20*t-11.125)) * 0.5f + 1;
}


//////////////////////////////////////////////////////////////////////////
// TIMED CURVES

static inline bool CAT_pulse(float period)
{
	int T = period * 1000.0f;
	int t = CAT_get_uptime_ms();
	return t % (2*T) < T;
}
