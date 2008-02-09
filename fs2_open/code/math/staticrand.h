/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Math/StaticRand.h $
 * $Revision: 2.5 $
 * $Date: 2005-08-31 06:25:27 $
 * $Author: Goober5000 $
 *
 * header for Static Random functions
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2005/07/13 03:15:50  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.3  2005/04/05 05:53:18  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.2  2004/08/11 05:06:27  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2004/06/05 19:14:42  phreak
 * added static_random_cone which is used for spawn angle features in multi
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
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
 * 
*/

#ifndef _STATIC_RAND_H

#define	SEMIRAND_MAX_LOG	4
#define	SEMIRAND_MAX		(2 << SEMIRAND_MAX_LOG)	//	Do not change this!  Change SEMIRAND_MAX_LOG!

extern int Semirand[SEMIRAND_MAX];			// this array is saved by the ai code on save/restore

extern void init_semirand();
extern int static_rand(int num);
extern float static_randf(int num);
extern void static_randvec(int num, vec3d *vp);
extern float static_randf_range(int num, float min, float max);
void static_rand_cone(int num, vec3d *out, vec3d *in, float max_angle, matrix *orient = NULL);

// Alternate random number generator that doesn't affect rand() sequence

void	srand_alt(int seed);	// Seed the random number generator
int	rand_alt();				// Get a random integer between 1 and RND_MAX
float	frand_alt();				// Get a random float between 0 and 1.0

#endif
