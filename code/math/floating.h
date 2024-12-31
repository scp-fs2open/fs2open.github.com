/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _FLOATING_H
#define _FLOATING_H

#include <cmath>
#include <cfloat>
#include <limits>

extern float frand();
extern int rand_chance(float frametime, float chance = 1.0f);
float frand_range(float min, float max);

// determine if a floating point number is NaN (Not a Number)
inline bool fl_is_nan(float fl) {
	return std::isnan(fl);
}

// Handy macros to prevent type casting all over the place

#define fl_sqrt(fl) sqrtf(fl)
#define fl_isqrt(fl) (1.0f/sqrtf(fl))
#define fl_abs(fl) fabsf(fl)
#define i2fl(i) (static_cast<float>(i))                                     // int to float
#define i2ch(i) (static_cast<char>(i))                                      // int to char
#define l2d(l) (static_cast<double>(l))                                     // long to double
#define fl2i(fl) (static_cast<int>(fl))                                     // float to int
#define ch2i(ch) (static_cast<int>(ch))                                     // char to int
#define d2l(d) (static_cast<long>(d))                                       // double to long
#define fl2ir(fl) (static_cast<int>(fl + (((fl) < 0.0f) ? -0.5f : 0.5f)))   // float to int, rounding
#define d2lr(d) (static_cast<long>(d + (((d) < 0.0) ? -0.5 : 0.5)))         // double to long, rounding
#define f2fl(fx) (static_cast<float>(fx)/65536.0f)                          // fix to float
#define f2d(fx) (static_cast<double>(fx)/65536.0)                           // fix to double
#define fl2f(fl) (static_cast<int>((fl)*65536.0f))                          // float to fix
#define fl_tan(fl) tanf(fl)

// convert a measurement in degrees to radians
#define fl_radians(fl)	(static_cast<float>((fl) * (PI / 180.0f)))

// convert a measurement in radians to degrees
#define fl_degrees(fl)	(static_cast<float>((fl) * (180.0f / PI)))

// sees if a floating point number is within a certain threshold (by default, epsilon) of zero
inline bool fl_near_zero(float a, float e = std::numeric_limits<float>::epsilon())
{
	return a < e && a > -e;
}

// sees if two floating point numbers are approximately equal, taking into account the argument magnitudes
// see commit c62037
// and see also this article, because fl_equal may need to be rewritten if it is recruited into more demanding situations:
// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
inline bool fl_equal(float a, float b)
{
	return fl_abs(a - b) <= FLT_EPSILON * std::max({ 1.0f, fl_abs(a), fl_abs(b) });
}

// sees if two floating point numbers are approximately equal, with a user-specifiable epsilon
inline bool fl_equal(float a, float b, float epsilon)
{
	return fl_abs(a - b) <= epsilon;
}

// rounds off a floating point number to a multiple of some number
extern float fl_roundoff(float x, int multiple);

const float GOLDEN_RATIO = 0.618033989f;

float golden_ratio_rand();

// clamps the input to -1, 1 to avoid floating point error putting it outside that range
float acosf_safe(float x);
float asinf_safe(float x);

#endif
