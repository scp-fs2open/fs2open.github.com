/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Math/Floating.h $
 * $Revision: 2.2 $
 * $Date: 2005-03-24 23:36:13 $
 * $Author: taylor $
 *
 * Low-level floating point math macros and routines
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/08/11 05:06:27  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     4/07/99 6:22p Dave
 * Fred and Freespace support for multiple background bitmaps and suns.
 * Fixed link errors on all subprojects. Moved encrypt_init() to
 * cfile_init() and lcl_init(), since its safe to call twice.
 * 
 * 3     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 13    2/26/98 3:28p John
 * Changed all sqrt's to use fl_sqrt.  Took out isqrt function
 * 
 * 12    1/26/98 10:43p Mike
 * Make ships not all zoom away from an impending shockwave at the same
 * time.  Based on ai class and randomness
 * 
 * 11    1/17/98 3:32p Mike
 * Add rand_range(), returns random float in min..max.
 * 
 * 10    7/29/97 2:36p Hoffoss
 * Added header file required by _isnan().
 * 
 * 9     7/29/97 2:35p Hoffoss
 * Added a NaN check macro.
 * 
 * 8     2/17/97 5:18p John
 * Added a bunch of RCS headers to a bunch of old files that don't have
 * them.
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef _FLOATING_H
#define _FLOATING_H

#include <math.h>
#include <float.h>

extern float frand();
extern int rand_chance(float frametime, float chance = 1.0f);
float frand_range(float min, float max);

// determine if a floating point number is NaN (Not a Number)
#define fl_is_nan(fl) _isnan((double)(fl))

// Handy macros to prevent type casting all over the place

#define fl_sqrt(fl) sqrtf(fl)
#define fl_isqrt(fl) (1.0f/sqrtf(fl))
#define fl_abs(fl) fabsf(fl)
#define i2fl(i) ((float)(i))
#define fl2i(fl) ((int)(fl))
#define flceil(fl) (int)ceil(fl)
#define flfloor(fl) (int)floor(fl)
#define f2fl(fx) ((float)(fx)/65536.0f)
#define fl2f(fl) (int)((fl)*65536.0f)

// convert a measurement in degrees to radians
#define fl_radian(fl)	((float)((fl * 3.14159f)/180.0f))

// convert a measurement in radians to degrees
#define fl_degrees(fl)	((float)((fl * 180.0f)/3.14159))

// use this instead of:
// for:  (int)floor(x+0.5f) use fl_round_2048(x)
//       (int)ceil(x-0.5f)  use fl_round_2048(x)
//       (int)floor(x-0.5f) use fl_round_2048(x-1.0f)
//       (int)floor(x)      use fl_round_2048(x-0.5f)
// for values in the range -2048 to 2048
// use this instead of:
// for:  (int)floor(x+0.5f) use fl_round_2048(x)
//       (int)ceil(x-0.5f)  use fl_round_2048(x)
//       (int)floor(x-0.5f) use fl_round_2048(x-1.0f)
//       (int)floor(x)      use fl_round_2048(x-0.5f)
// for values in the range -2048 to 2048

extern const float *p_fl_magic;

inline int fl_round_2048( float x )
{
	double tmp_quad;
	tmp_quad = x + *p_fl_magic;
	return *((int *)&tmp_quad);
}

/*
inline float fl_sqrt( float x)
{
	float retval;

	_asm fld x
	_asm fsqrt
	_asm fstp retval
	
	return retval;
}

float fl_isqrt( float x )
{
	float retval;

	_asm fld x
	_asm fsqrt
	_asm fstp retval
	
	return 1.0f / retval;
} 
*/



// rounds off a floating point number to a multiple of some number
extern float fl_roundoff(float x, int multiple);


#endif
