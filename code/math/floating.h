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
#define i2fl(i) ((float)(i))
#define fl2i(fl) ((int)(fl))
#define fl2ir(fl) ((int)(fl + ((fl < 0.0f) ? -0.5f : 0.5f)))
#define flceil(fl) (int)ceil(fl)
#define flfloor(fl) (int)floor(fl)
#define f2fl(fx) ((float)(fx)/65536.0f)
#define f2d(fx) (((double)(fx))/65536.0)
#define fl2f(fl) (int)((fl)*65536.0f)
#define fl_tan(fl) tanf(fl)

// convert a measurement in degrees to radians
#define fl_radians(fl)	((float)((fl) * (PI / 180.0f)))

// convert a measurement in radians to degrees
#define fl_degrees(fl)	((float)((fl) * (180.0f / PI)))

// use this instead of:
// for:  (int)floor(x+0.5f) use fl_round_2048(x)
//       (int)ceil(x-0.5f)  use fl_round_2048(x)
//       (int)floor(x-0.5f) use fl_round_2048(x-1.0f)
//       (int)floor(x)      use fl_round_2048(x-0.5f)
// for values in the range -2048 to 2048
// use this instead of:
// for:  (int)floor(x+0.5f) use fl_round_2048(x)
//       (int)ceil(x-0.5f)  use fl_round_2048(x)
//       (int)floor(x-0.5f) use fl_round_2048(x-1.0f)
//       (int)floor(x)      use fl_round_2048(x-0.5f)
// for values in the range -2048 to 2048

/*
extern const float *p_fl_magic;

inline int fl_round_2048( float x )
{
	double tmp_quad;
	tmp_quad = x + *p_fl_magic;
	return *((int *)&tmp_quad);
}

inline float fl_sqrt( float x)
{
	float retval;

	_asm fld x
	_asm fsqrt
	_asm fstp retval
	
	return retval;
}

float fl_isqrt( float x )
{
	float retval;

	_asm fld x
	_asm fsqrt
	_asm fstp retval
	
	return 1.0f / retval;
} 
*/

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
