/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Math/StaticRand.cpp $
 * $Revision: 2.5 $
 * $Date: 2005-04-05 05:53:18 $
 * $Author: taylor $
 *
 * static random functions.  Return "random" number based on integer inut
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2004/07/26 20:47:36  Kazan
 * remove MCD complete
 *
 * Revision 2.3  2004/07/12 16:32:52  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.2  2004/06/05 19:14:42  phreak
 * added static_random_cone which is used for spawn angle features in multi
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/03 22:07:08  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 4     3/17/98 12:16a Allender
 * asteroids in multiplayer -- minor problems with position being correct
 * 
 * 3     12/30/97 5:46p Lawrance
 * Rename rnd() to rand_alt().
 * 
 * 2     12/30/97 4:27p Lawrance
 * Add new rnd() function that doesn't affect rand() sequence.
 * 
 * 1     8/08/97 3:38p Allender
 */

#include "globalincs/pstypes.h"
#include "math/staticrand.h"
#include "math/vecmat.h"



int Semirand_inited = 0;
int Semirand[SEMIRAND_MAX];

//	Initialize Semirand array.
void init_semirand()
{
	int	i;

	Semirand_inited = 1;

	for (i=0; i<SEMIRAND_MAX; i++)
		Semirand[i] = (myrand() << 15) + myrand();
}


//	Return a fairly random 32 bit value given a reasonably small number.
int static_rand(int num)
{
	int	a, b, c;

	if (!Semirand_inited)
		init_semirand();

	a = num & (SEMIRAND_MAX - 1);
	b = (num >> SEMIRAND_MAX_LOG) & (SEMIRAND_MAX - 1);
	c = (num >> (2 * SEMIRAND_MAX_LOG)) & (SEMIRAND_MAX - 1);

	return Semirand[a] ^ Semirand[b] ^ Semirand[c];
}

//	Return a random value in 0.0f .. 1.0f- (ie, it will never return 1.0f).
float static_randf(int num)
{
	int	a;

	a = static_rand(num);

	return (a & 0xffff) / 65536.0f;
}

float static_randf_range(int num, float min, float max)
{
	float	rval;
	
	rval = static_randf(num);
	rval = rval * (max - min) + min;

	return rval;
}


void static_randvec(int num, vec3d *vp)
{
	vp->xyz.x = static_randf(num) - 0.5f;
	vp->xyz.y = static_randf(num+1) - 0.5f;
	vp->xyz.z = static_randf(num+2) - 0.5f;

	vm_vec_normalize_quick(vp);
}

// randomly perturb a vector around a given (normalized vector) or optional orientation matrix
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
	vm_rot_point_around_line(&t1, in, fl_radian(static_randf_range(num,-max_angle, max_angle)), &vmd_zero_vector, &rot->vec.fvec);
	
	// axis 2
	vm_rot_point_around_line(&t2, &t1, fl_radian(static_randf_range(num+1,-max_angle, max_angle)), &vmd_zero_vector, &rot->vec.rvec);

	// axis 3
	vm_rot_point_around_line(out, &t2, fl_radian(static_randf_range(num+2,-max_angle, max_angle)), &vmd_zero_vector, &rot->vec.uvec);
}

/////////////////////////////////////////////////////////////////////
// Alternate random number generator, that doesn't affect rand() sequence
/////////////////////////////////////////////////////////////////////
#define RND_MASK	0x6000
#define RND_MAX	0x7fff
int Rnd_seed = 1;

// Seed the random number generator.  Doesn't have to be called.
void srand_alt(int seed)
{
	Rnd_seed = seed;
}

// Get a random integer between 1 and RND_MAX
int rand_alt()
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

// Get a random float between 0 and 1.0
float frand_alt()
{
	int r = rand_alt();
	return i2fl(r)/RND_MAX;
}
