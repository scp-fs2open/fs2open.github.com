/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "globalincs/pstypes.h"
#include "math/staticrand.h"
#include "math/vecmat.h"


int Semirand_inited = 0;
int Semirand[SEMIRAND_MAX];

/**
 * @brief Initialize Semirand array. Doesn't have to be called.
 */
void init_semirand()
{
	int	i;

	Semirand_inited = 1;

	for (i=0; i<SEMIRAND_MAX; i++)
		Semirand[i] = (myrand() << 15) + myrand();
}

/**
 * @brief Return a pseudo random 32 bit value given a reasonably small number.
 *
 * @param num Seed input number
 * @return Pseudo random 32 bit value
 */
int static_rand(int num)
{
	int	a, b, c;

	if (num < 0) 
		num *= -1;

	if (!Semirand_inited)
		init_semirand();

	a = num & (SEMIRAND_MAX - 1);
	b = (num >> SEMIRAND_MAX_LOG) & (SEMIRAND_MAX - 1);
	c = (num >> (2 * SEMIRAND_MAX_LOG)) & (SEMIRAND_MAX - 1);

	return Semirand[a] ^ Semirand[b] ^ Semirand[c];
}

/**
 * @brief Return a random float in 0.0f .. 1.0f- (ie, it will never return 1.0f).
 *
 * @param num Seed input number
 * @return Random value in 0.0f .. 1.0f- (ie, it will never return 1.0f).
 */
float static_randf(int num)
{
	int	a;

	a = static_rand(num);

	return (a & 0xffff) / 65536.0f;
}

/**
 * @brief Return a random integer within a range. Note: min and max are inclusive
 *
 * @param num Seed input number
 * @param min Minimum range integer to return
 * @param max Maximum range integer to return
 * @return Random integer within the range
 */
int static_rand_range(int num, int min, int max)
{
	int	rval = static_rand(num);
	rval = (rval % (max - min + 1)) + min;
	CLAMP(rval, min, max);
	return rval;
}

/**
 * @brief Return a random float within a range.
 * Note: min and max are inclusive
 *
 * @param num Seed input number
 * @param min Minimum range float to return
 * @param max Maximum range float to return
 * @return Random float within the range
 */
float static_randf_range(int num, float min, float max)
{
	float	rval;
	
	rval = static_randf(num);
	rval = rval * (max - min) + min;

	return rval;
}

/**
 * @brief [To be described]
 *
 * @param num Seed input number
 * @param vp Vector
 */
void static_randvec(int num, vec3d *vp)
{
	vp->xyz.x = static_randf(num) - 0.5f;
	vp->xyz.y = static_randf(num+1) - 0.5f;
	vp->xyz.z = static_randf(num+2) - 0.5f;

	vm_vec_normalize_quick(vp);
}

/**
 * @brief Randomly perturb a vector around a given (normalized vector) or optional orientation matrix.
 *
 * @param num
 * @param out
 * @param in
 * @param max_angle
 * @param orient
 */
void static_rand_cone(int num, vec3d *out, vec3d *in, float max_angle, matrix *orient)
{
	vec3d t1, t2;
	matrix *rot;
	matrix m;

	// get an orientation matrix
	if(orient != NULL){
		rot = orient;
	} else {
		vm_vector_2_matrix(&m, in, NULL, NULL);
		rot = &m;
	}
	
	// axis 1
	vm_rot_point_around_line(&t1, in, fl_radians(static_randf_range(num,-max_angle, max_angle)), &vmd_zero_vector, &rot->vec.fvec);
	
	// axis 2
	vm_rot_point_around_line(&t2, &t1, fl_radians(static_randf_range(num+1,-max_angle, max_angle)), &vmd_zero_vector, &rot->vec.rvec);

	// axis 3
	vm_rot_point_around_line(out, &t2, fl_radians(static_randf_range(num+2,-max_angle, max_angle)), &vmd_zero_vector, &rot->vec.uvec);
}

/////////////////////////////////////////////////////////////////////
// Alternate random number generator, that doesn't affect rand() sequence
/////////////////////////////////////////////////////////////////////
#define RND_MASK	0x6000
#define RND_MAX	0x7fff
int Rnd_seed = 1;

/** 
 * @brief Seed the alternative random number generator. 
 * Doesn't have to be called.
 *
 * @param seed Seed input number
 */
void init_static_rand_alt(int seed)
{
	Rnd_seed = seed;
}

/**
 * @brief Get a random integer between 1 and RND_MAX.
 *
 * @return Random integer between 1 and RND_MAX
 */
int static_rand_alt()
{
	static int x=Rnd_seed;
	int old_x;
	old_x = x;
	x >>= 1;
	if ( old_x & 1 ) {
		x ^= RND_MASK;
	}
	return x;
}

/**
 * @brief Get a random integer between 0 and 1.0.
 *
 * @return Random float between 0 and 1.0
 */
float static_randf_alt()
{
	int r = static_rand_alt();
	return i2fl(r)/RND_MAX;
}
