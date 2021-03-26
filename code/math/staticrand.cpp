/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include "math/staticrand.h"
#include "math/vecmat.h"
#include "utils/Random.h"

bool Semirand_inited = false;
unsigned int Semirand[SEMIRAND_MAX];

/**
 * @brief Initialize Semirand array. Doesn't have to be called.
 */
void init_semirand()
{
	Semirand_inited = true;

	// Originally this made a 30-bit rand by sticking two 15-bit rands from myrand() together. Instead we trim Random::next() down to size.
	for (auto & number : Semirand)
		number = (unsigned) (util::Random::next() & STATIC_RAND_MAX);
}

// TODO: figure out what to do with these
/**
 * @brief Return a pseudo random 32 bit value given a reasonably small number.
 *
 * @param num Seed input number
 * @return Pseudo random 32 bit value
 */
int static_rand(int num)
{
	if (num < 0)
		num *= -1;

	if (!Semirand_inited)
		init_semirand();

	const unsigned int num_unsigned = num;
	auto a = num_unsigned & (SEMIRAND_MAX - 1);
	auto b = (num_unsigned >> SEMIRAND_MAX_LOG) & (SEMIRAND_MAX - 1);
	auto c = (num_unsigned >> (2 * SEMIRAND_MAX_LOG)) & (SEMIRAND_MAX - 1);

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
	unsigned int	a = static_rand(num);
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
	float rval = static_randf(num);
	rval = rval * (max - min) + min;

	return rval;
}

/**
 * @brief Create a random, normalized vector in unit sphere
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
 *
 * @brief Create a random, unnormalized vector in the (half) unit cube
 *
 * @param num Seed input vector
 * @param vp Vector
 */
void static_randvec_unnormalized(int num, vec3d* vp)
{
	vp->xyz.x = static_randf(num) - 0.5f;
	vp->xyz.y = static_randf(num + 1) - 0.5f;
	vp->xyz.z = static_randf(num + 2) - 0.5f;
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
void static_rand_cone(int num, vec3d *out, const vec3d* const in, float max_angle, const matrix* const orient)
{
	vec3d t1, t2;
	const matrix *rot;
	matrix m;

	// get an orientation matrix
	if(orient != nullptr){
		rot = orient;
	} else {
		vm_vector_2_matrix(&m, in, nullptr, nullptr);
		rot = &m;
	}
	
	// axis 1
	vm_rot_point_around_line(&t1, in, fl_radians(static_randf_range(num,-max_angle, max_angle)), &vmd_zero_vector, &rot->vec.fvec);
	
	// axis 2
	vm_rot_point_around_line(&t2, &t1, fl_radians(static_randf_range(num+1,-max_angle, max_angle)), &vmd_zero_vector, &rot->vec.rvec);

	// axis 3
	vm_rot_point_around_line(out, &t2, fl_radians(static_randf_range(num+2,-max_angle, max_angle)), &vmd_zero_vector, &rot->vec.uvec);
}

//generates a random vector in a cone, with a min amd max angle. 
//Clone of vm_vec_random_cone overload of the same function, adapted to use static_randf_range
void static_rand_cone(int num, vec3d* out, const vec3d* const in, float min_angle, float max_angle, const matrix* const orient)
{
	vec3d temp;
	const matrix* rot;
	matrix m;

	if (max_angle < min_angle) {
		std::swap(min_angle, max_angle);
	}

	// get an orientation matrix
	if (orient != nullptr) {
		rot = orient;
	}
	else {
		vm_vector_2_matrix(&m, in, nullptr, nullptr);
		rot = &m;
	}

	// Get properly distributed spherical coordinates (DahBlount)
	// This might not seem intuitive, but the min_angle is the angle that will have a larger z coordinate
	float z = static_randf_range(num, cosf(fl_radians(max_angle)), cosf(fl_radians(min_angle))); // Take a 2-sphere slice
	float phi = static_randf_range(num+1, 0.0f, PI2);
	vm_vec_make(&temp, sqrtf(1.0f - z * z) * cosf(phi), sqrtf(1.0f - z * z) * sinf(phi), z); // Using the z-vec as the starting point

	vm_vec_unrotate(out, &temp, rot); // We find the final vector by rotating temp to the correct orientation
}


/////////////////////////////////////////////////////////////////////
// Alternate random number generator, that doesn't affect rand() sequence
/////////////////////////////////////////////////////////////////////
constexpr unsigned int RND_MASK = 0x6000;
constexpr int RND_MAX = 0x7fff;
unsigned int Rnd_seed = 1;

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
	static unsigned int x=Rnd_seed;
	unsigned int old_x = x;
	x >>= 1u;
	if ( old_x & 1u ) {
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
