/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _STATIC_RAND_H
#define _STATIC_RAND_H

#include "globalincs/pstypes.h"

constexpr unsigned int SEMIRAND_MAX_LOG = 4;
constexpr unsigned int SEMIRAND_MAX = (2u << SEMIRAND_MAX_LOG); //Do not change this!  Change SEMIRAND_MAX_LOG!

// Static rand has a max value independent of Random::next()
#define STATIC_RAND_MAX 0x3fffffff

extern void init_semirand();
extern int static_rand(int num);
extern float static_randf(int num);
extern void static_randvec(int num, vec3d *vp);
extern void static_randvec_unnormalized(int num, vec3d* vp);
extern int static_rand_range(int num, int min, int max);
extern float static_randf_range(int num, float min, float max);
void static_rand_cone(int num, vec3d *out, const vec3d* const in, float max_angle, const matrix* const orient = nullptr);
void static_rand_cone(int num, vec3d *out, const vec3d* const in, float min_angle, float max_angle, const matrix* const orient = nullptr);

// Alternate random number generator that doesn't affect rand() sequence
/// Get a random integer between 1 and RND_MAX
int static_rand_alt();
/// Seed the random number generator
void init_static_rand_alt(int seed);
/// Get a random float between 0 and 1.0
float static_randf_alt();

#endif
