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


/*
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


//	if ( !iSqrt_inited )
//		MakeInverseSqrtLookupTable();

//	seed.i = SET_EXP(((3*EXP_BIAS-1) - GET_EXP(a)) >> 1 ) | SET_MANTSEED(iSqrt[GET_EMANT(a)]);
//	r = seed.f;
//	r = (3.0f - r * r * arg ) * r * 0.5f;
//	r = (3.0f - r * r * arg ) * r * 0.5f;
//	return r;
}
*/

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
