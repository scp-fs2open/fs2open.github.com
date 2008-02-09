/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Math/Floating.cpp $
 * $Revision: 2.4 $
 * $Date: 2005-04-12 05:26:36 $
 * $Author: taylor $
 *
 * Low-level floating point math routines
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/07/26 20:47:36  Kazan
 * remove MCD complete
 *
 * Revision 2.2  2004/07/12 16:32:52  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/28 20:22:00  mharris
 * Changed frand() -- was returning negative values under Linux
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
 * 13    2/26/98 3:28p John
 * Changed all sqrt's to use fl_sqrt.  Took out isqrt function
 * 
 * 12    1/30/98 12:25p Mike
 * Make frand() not return 1.0, which can cause overflow when indexing
 * into arrays.
 * 
 * 11    1/26/98 10:43p Mike
 * Make ships not all zoom away from an impending shockwave at the same
 * time.  Based on ai class and randomness
 * 
 * 10    1/20/98 9:47a Mike
 * Suppress optimized compiler warnings.
 * Some secondary weapon work.
 * 
 * 9     1/17/98 3:32p Mike
 * Add rand_range(), returns random float in min..max.
 * 
 * 8     9/09/97 11:07a Sandeep
 * fixed warning level 4
 * 
 * 7     8/05/97 10:18a Lawrance
 * my_rand() being used temporarily instead of rand()
 * 
 * 6     2/17/97 5:18p John
 * Added a bunch of RCS headers to a bunch of old files that don't have
 * them.
 *
 * $NoKeywords: $
 */

#include <stdlib.h>
#include <math.h>

#include "globalincs/pstypes.h"
#include "math/floating.h"
#include "io/timer.h"



#define LOOKUP_BITS	6
#define EXP_POS		23
#define EXP_BIAS		127
typedef float FLOAT;

#define LOOKUP_POS	(EXP_POS-LOOKUP_BITS)
#define SEED_POS		(EXP_POS-8)
#define TABLE_SIZE	(2<<LOOKUP_BITS)
#define LOOKUP_MASK	(TABLE_SIZE-1)
#define GET_EXP(a)	(((a) >> EXP_POS) & 0xFF )
#define SET_EXP(a)	((a) << EXP_POS )
#define GET_EMANT(a)	(((a) >> LOOKUP_POS) & LOOKUP_MASK )
#define SET_MANTSEED(a)	(((unsigned long)(a)) << SEED_POS )


int fl_magic = 0x59C00000;		//representation of 2^51 + 2^52
const float *p_fl_magic = (const float *)&fl_magic;

union _flint {
	unsigned long	i;
	float				f;
} fi, fo;

/*
static unsigned char iSqrt[TABLE_SIZE];
static int iSqrt_inited = 0;

static void MakeInverseSqrtLookupTable()
{
	long f;
	unsigned char *h;
	union _flint fi, fo;

	iSqrt_inited = 1;
	for ( f=0, h=iSqrt; f < TABLE_SIZE; f++ )	{
		fi.i = ((EXP_BIAS-1)<<EXP_POS) | (f<<LOOKUP_POS);
		fo.f = 1.0f / fl_sqrt(fi.f);
		*h++ = (unsigned char)(((fo.i + (1<<(SEED_POS-2))) >>SEED_POS ) & 0xFF);
	}
	iSqrt[ TABLE_SIZE / 2 ] = 0xFF;
}
*/

// HACK!
float fl_isqrt_c( float x )
{
//	unsigned long a = ((union _flint *)(&x))->i;
//	float arg = x;
//	union _flint seed;
//	FLOAT r;

	int t1, t2, t3;
	t1 = timer_get_microseconds();
	float r1 =  1.0f / (float)sqrt((double)x);
	t2 = timer_get_microseconds();
//	float r2 = fl_isqrt_asm(x);
	t3 = timer_get_microseconds();	

	return r1;


/*	if ( !iSqrt_inited )
		MakeInverseSqrtLookupTable();

	seed.i = SET_EXP(((3*EXP_BIAS-1) - GET_EXP(a)) >> 1 ) | SET_MANTSEED(iSqrt[GET_EMANT(a)]);
	r = seed.f;
	r = (3.0f - r * r * arg ) * r * 0.5f;
	r = (3.0f - r * r * arg ) * r * 0.5f;
	return r;
*/
}

// rounds off a floating point number to a multiple of some number
float fl_roundoff(float x, int multiple)
{
	float half = (float) multiple / 2.0f;

	if (x < 0)
		half = -half;

	x += half;
	return (float) (((int) x / multiple) * multiple);
}


//	Return random value in range 0.0..1.0- (1.0- means the closest number less than 1.0)
float frand()
{
	int i_rval;
	do {
		i_rval = myrand();
	} while (i_rval == RAND_MAX);
	float rval = ((float) i_rval) / RAND_MAX;
	return rval;
}

//	Return a floating point number in the range min..max.
float frand_range(float min, float max)
{
	float	rval;
	
	rval = frand();
	rval = rval * (max - min) + min;

	return rval;
}

//	Call this in the frame interval to get TRUE chance times per second.
//	If you want it to return TRUE 3 times per second, call it in the frame interval like so:
//		rand_chance(flFrametime, 3.0f);
int rand_chance(float frametime, float chance)	//	default value for chance = 1.0f.
{
	while (--chance > 0.0f)
		if (frand() < frametime)
			return 1;

	return frand() < (frametime * (chance + 1.0f));
}

/*fix fl2f( float x )
{
	float nf;
	nf = x*65536.0f + 8390656.0f;
	return ((*((int *)&nf)) & 0x7FFFFF)-2048;
}
*/


/*
>#define  S  65536.0
>#define  MAGIC  (((S * S * 16) + (S*.5)) * S)
>
>#pragma inline float2int;
>
>ulong float2int( float d )
>{
>  double dtemp = MAGIC + d;
>  return (*(ulong *)&dtemp) - 0x80000000;
>}

*/
