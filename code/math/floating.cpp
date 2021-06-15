/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include <cstdlib>

#include "io/timer.h"
#include "math/floating.h"
#include "math/staticrand.h"
#include "utils/Random.h"


/**
 * @brief Rounds off a floating point number to a multiple of some number
 * @param x Input floating point number
 * @param multiple Multiple to round to
 */
float fl_roundoff(float x, int multiple)
{
	float half = (float) multiple / 2.0f;

	if (x < 0)
		half = -half;

	x += half;
	return (float) (((int) x / multiple) * multiple);
}

/**
 * @brief Return random value in range 0.0..1.0- (1.0- means the closest number less than 1.0)
 */
float frand()
{
	int i_rval;
	do {
		i_rval = Random::next();
	} while (i_rval == Random::MAX_VALUE);
	float rval = i2fl(i_rval) * Random::INV_F_MAX_VALUE;
	return rval;
}

/**
 * @brief Return a floating point number in the range min..max.
 * @param min Minimum (inclusive)
 * @param max Maxiumum (inclusive) 
 */
float frand_range(float min, float max)
{
	float	rval;
	
	rval = frand();
	rval = rval * (max - min) + min;

	return rval;
}

/**
 * @brief Call this in the frame interval to get TRUE chance times per second.
 * @details If you want it to return TRUE 3 times per second, call it in the frame interval like so: rand_chance(flFrametime, 3.0f);
 * @param frametime
 * @param chance
 */
int rand_chance(float frametime, float chance)
{
	while (--chance > 0.0f)
		if (frand() < frametime)
			return 1;

	return frand() < (frametime * (chance + 1.0f));
}

static float accum_golden_ratio_rand_seed = 0.0f;

// generates a quasirandom, low discrepancy number sequence
// acts very much like a random number generator, but has the property of being very well distributed
// use if you need quasirandom numbers, but don't want ugly 'runs' or 'clumps' of similar numbers 
float golden_ratio_rand() {
	accum_golden_ratio_rand_seed += GOLDEN_RATIO;
	if (accum_golden_ratio_rand_seed >= 1.0f)
		accum_golden_ratio_rand_seed -= 1.0f;
	return accum_golden_ratio_rand_seed;
}

float acosf_safe(float x) {
	CLAMP(x, -1.f, 1.f);
	return acosf(x);
}

float asinf_safe(float x) {
	CLAMP(x, -1.f, 1.f);
	return asinf(x);
}
