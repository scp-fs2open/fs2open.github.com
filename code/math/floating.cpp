/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include <stdlib.h>
#include <math.h>

#include "globalincs/pstypes.h"
#include "math/floating.h"
#include "io/timer.h"


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
		i_rval = myrand();
	} while (i_rval == RAND_MAX);
	float rval = i2fl(i_rval) * RAND_MAX_1f;
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
