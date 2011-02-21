/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _STATIC_RAND_H

#include "globalincs/pstypes.h"

#define	SEMIRAND_MAX_LOG	4
#define	SEMIRAND_MAX		(2 << SEMIRAND_MAX_LOG)	//	Do not change this!  Change SEMIRAND_MAX_LOG!

extern int Semirand[SEMIRAND_MAX];			// this array is saved by the ai code on save/restore

extern void init_semirand();
extern int static_rand(int num);
extern float static_randf(int num);
extern void static_randvec(int num, vec3d *vp);
extern int static_rand_range(int num, int min, int max);
extern float static_randf_range(int num, float min, float max);
void static_rand_cone(int num, vec3d *out, vec3d *in, float max_angle, matrix *orient = NULL);

// Alternate random number generator that doesn't affect rand() sequence

void	init_static_rand_alt(int seed);	// Seed the random number generator
int		static_rand_alt();				// Get a random integer between 1 and RND_MAX
float	static_randf_alt();				// Get a random float between 0 and 1.0

#endif
