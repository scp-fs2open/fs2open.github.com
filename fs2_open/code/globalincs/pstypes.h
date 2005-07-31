/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/GlobalIncs/PsTypes.h $
 * $Revision: 2.32 $
 * $Date: 2005-07-31 01:30:48 $
 * $Author: taylor $
 * $Revision: 2.32 $
 * $Date: 2005-07-31 01:30:48 $
 * $Author: taylor $
 *
 * Header file containg global typedefs, constants and macros
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.31  2005/07/13 02:50:48  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.30  2005/05/12 17:49:11  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.29  2005/04/24 12:56:42  taylor
 * really are too many changes here:
 *  - remove all bitmap section support and fix problems with previous attempt
 *  ( code/bmpman/bmpman.cpp, code/bmpman/bmpman.h, code/globalincs/pstypes.h,
 *    code/graphics/2d.cpp, code/graphics/2d.h code/graphics/grd3dbmpman.cpp,
 *    code/graphics/grd3dinternal.h, code/graphics/grd3drender.cpp, code/graphics/grd3dtexture.cpp,
 *    code/graphics/grinternal.h, code/graphics/gropengl.cpp, code/graphics/gropengl.h,
 *    code/graphics/gropengllight.cpp, code/graphics/gropengltexture.cpp, code/graphics/gropengltexture.h,
 *    code/graphics/tmapper.h, code/network/multi_pinfo.cpp, code/radar/radarorb.cpp
 *    code/render/3ddraw.cpp )
 *  - use CLAMP() define in gropengl.h for gropengllight instead of single clamp() function
 *  - remove some old/outdated code from gropengl.cpp and gropengltexture.cpp
 *
 * Revision 2.28  2005/04/05 05:53:16  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.27  2005/03/10 08:00:04  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.26  2005/03/07 13:10:20  bobboau
 * commit of render target code, d3d should be totaly functional,
 * OGL still needs implementation.
 *
 * Revision 2.25  2005/03/02 21:18:18  taylor
 * better support for Inferno builds (in PreProcDefines.h now, no networking support)
 * make sure NO_NETWORK builds are as friendly on Windows as it is on Linux/OSX
 * revert a timeout in Client.h back to the original value before Linux merge
 *
 * Revision 2.24  2005/03/01 23:08:24  taylor
 * make sure starfield bitmaps render when not in HTL mode
 * slight header fix for osapi.h
 * add some string overflow protection to modelread and bmpman
 * s/NO_NETWORKING/NO_NETWORK/g  (Inferno builds)
 *
 * Revision 2.23  2005/02/08 23:49:58  taylor
 * update/add .cvsignore files for project file changes
 * silence warning about depreciated strings.h stuff for MSVC 2005
 * final model_unload() stuff for WMCoolmon, put in missionweaponchoice.cpp
 * remove really old project files
 *
 * Revision 2.22  2005/02/04 10:12:29  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.21  2004/12/15 15:14:03  Goober5000
 * added Verify, an error checking macro that works in both release and debug modes
 * --Goober5000
 *
 * Revision 2.20  2004/10/31 21:32:27  taylor
 * no networking with Inferno builds, basic 64-bit OS support
 *
 * Revision 2.19  2004/08/11 05:06:24  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.18  2004/07/05 05:09:18  bobboau
 * FVF code, only the data that is needed is sent off to the card,,
 * OGL can take advantage of this if they want but it won't break
 * anything if they don't. also state block code has been implemented,
 * that's totaly internal to D3D no high level code is involved.
 *
 * Revision 2.17  2004/04/03 18:11:20  Kazan
 * FRED fixes
 *
 * Revision 2.16  2004/03/05 09:01:52  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.15  2003/11/19 20:37:23  randomtiger
 * Almost fully working 32 bit pcx, use -pcx32 flag to activate.
 * Made some commandline variables fit the naming standard.
 * Changed timerbar system not to run pushes and pops if its not in use.
 * Put in a note about not uncommenting asserts.
 * Fixed up a lot of missing POP's on early returns?
 * Perhaps the motivation for Assert functionality getting commented out?
 * Fixed up some bad asserts.
 * Changed nebula poofs to render in 2D in htl, it makes it look how it used to in non htl. (neb.cpp,1248)
 * Before the poofs were creating a nasty stripe effect where they intersected with ships hulls.
 * Put in a special check for the signs of that D3D init bug I need to lock down.
 *
 * Revision 2.14  2003/11/17 06:52:51  bobboau
 * got assert to work again
 *
 * Revision 2.13  2003/09/26 14:37:13  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.12  2003/08/16 03:52:23  bobboau
 * update for the specmapping code includeing
 * suport for seperate specular levels on lights and
 * optional strings for the stars table
 * code has been made more organised,
 * though there seems to be a bug in the state selecting code
 * resulting in the HUD being rendered incorectly
 * and specmapping failing ocasionaly
 *
 * Revision 2.11  2003/08/12 03:18:33  bobboau
 * Specular 'shine' mapping;
 * useing a phong lighting model I have made specular highlights
 * that are mapped to the model,
 * rendering them is still slow, but they look real purdy
 *
 * also 4 new (untested) comand lines, the XX is a floating point value
 * -spec_exp XX
 * the n value, you can set this from 0 to 200 (actualy more than that, but this is the recomended range), it will make the highlights bigger or smaller, defalt is 16.0 so start playing around there
 * -spec_point XX
 * -spec_static XX
 * -spec_tube XX
 * these are factors for the three diferent types of lights that FS uses, defalt is 1.0,
 * static is the local stars,
 * point is weapons/explosions/warp in/outs,
 * tube is beam weapons,
 * for thouse of you who think any of these lights are too bright you can configure them you're self for personal satisfaction
 *
 * Revision 2.10  2003/06/08 17:38:21  phreak
 * added a vector in the vertex struct that stores the real world position
 * the x,y,z variables do not accurately reflect the real position
 * this is for opengl fogging
 *
 * Revision 2.9  2003/03/18 10:07:02  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.8  2003/03/02 05:27:44  penguin
 * only define min() and max() if they're not already defined
 *  - penguin
 *
 * Revision 2.7  2003/01/18 19:48:19  phreak
 * added some defines that deal with mages using DXTC
 *
 * Revision 2.6  2002/12/02 23:56:12  Goober5000
 * fixed misspelling
 *
 * Revision 2.5  2002/12/02 20:49:14  Goober5000
 * fixed misspelling of "category" as "catagory"
 *
 * Revision 2.4.2.3  2002/10/19 23:56:40  randomtiger
 * Changed generic bitmap code to allow maximum dimensions to be determined by 3D's engines maximum texture size query.
 * Defaults to 256 as it was before. Also added base code for reworking the texture code to be more efficient. - RT
 *
 * Revision 2.4.2.2  2002/09/24 18:56:42  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.4  2002/08/06 01:49:08  penguin
 * Renamed ccode members to cc_or and cc_and
 *
 * Revision 2.3  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/22 01:04:37  penguin
 * took out GAME_CD_CHECK define
 *
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.4  2002/05/09 13:50:20  mharris
 * define AsmInt3() to abort()
 *
 * Revision 1.3  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.2  2002/05/03 22:07:08  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 34    6/16/00 3:16p Jefff
 * sim of the year dvd version changes, a few german soty localization
 * fixes
 * 
 * 33    10/05/99 2:29p Danw
 * 
 * 32    10/01/99 9:10a Daveb
 * V 1.1 PATCH
 * 
 * 31    9/13/99 12:22a Dave
 * Minor build update.
 * 
 * 30    8/28/99 4:54p Dave
 * Fixed directives display for multiplayer clients for wings with
 * multiple waves. Fixed hud threat indicator rendering color.
 * 
 * 29    8/09/99 4:18p Andsager
 * Make french and german defines, needed specifically to enable language
 * under launcher misc. tab
 * 
 * 28    7/20/99 1:49p Dave
 * Peter Drake build. Fixed some release build warnings.
 * 
 * 27    7/18/99 5:19p Dave
 * Jump node icon. Fixed debris fogging. Framerate warning stuff.
 * 
 * 26    7/15/99 9:21a Andsager
 * FS2_DEMO check in
 * 
 * 25    7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 24    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 23    6/14/99 10:45a Dave
 * Made beam weapons specify accuracy by skill level in the weapons.tbl
 * 
 * 22    6/03/99 10:15p Dave
 * Put in temporary main hall screen.
 * 
 * 21    5/09/99 8:57p Dave
 * Final E3 build preparations.
 * 
 * 20    4/25/99 7:43p Dave
 * Misc small bug fixes. Made sun draw properly.
 * 
 * 19    4/25/99 3:03p Dave
 * Removed E3_BUILD from pstypes
 * 
 * 18    4/25/99 3:02p Dave
 * Build defines for the E3 build.
 * 
 * 17    4/15/99 1:29p Dave
 * Remove multiplayer beta build define.
 * 
 * 16    4/15/99 1:24p Dave
 * Final Beta 1 checkin.
 * 
 * 15    4/14/99 5:28p Dave
 * Minor bug fixes.
 * 
 * 14    4/12/99 2:22p Dave
 * More checks for dogfight stats.
 * 
 * 13    4/09/99 2:21p Dave
 * Multiplayer beta stuff. CD checking.
 * 
 * 12    2/25/99 4:19p Dave
 * Added multiplayer_beta defines. Added cd_check define. Fixed a few
 * release build warnings. Added more data to the squad war request and
 * response packets.
 * 
 * 11    2/23/99 2:29p Dave
 * First run of oldschool dogfight mode. 
 * 
 * 10    2/07/99 8:51p Andsager
 * Add inner bound to asteroid field.  Inner bound tries to stay astroid
 * free.  Wrap when within and don't throw at ships inside.
 * 
 * 9     1/15/99 11:29a Neilk
 * Fixed D3D screen/texture pixel formatting problem. 
 * 
 * 8     1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 7     1/06/99 2:24p Dave
 * Stubs and release build fixes.
 * 
 * 6     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 5     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 4     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 3     10/07/98 11:28a Dave
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 75    8/20/98 5:30p Dave
 * Put in handy multiplayer logfile system. Now need to put in useful
 * applications of it all over the code.
 * 
 * 74    6/17/98 11:01a Lawrance
 * set correct offset for English special font chars
 * 
 * 73    6/12/98 4:52p Hoffoss
 * Added support for special characters in in forgeign languages.
 * 
 * 72    6/09/98 6:49p Lawrance
 * Comment out UK_BUILD
 * 
 * 71    6/09/98 5:15p Lawrance
 * French/German localization
 * 
 * 70    6/09/98 12:12p Hoffoss
 * Added XSTR localization code.
 * 
 * 69    6/05/98 9:49a Lawrance
 * OEM changes
 * 
 * 68    5/22/98 3:13p Allender
 * no Int3()'s and Asserts
 * 
 * 67    5/20/98 12:59p John
 * Turned optimizations on for debug builds.   Also turning on automatic
 * function inlining.  Turned off the unreachable code warning.
 * 
 * 66    5/04/98 1:49p Allender
 * make Int3's do nothing when InterplayQA is defined
 * 
 * 65    4/25/98 11:55p Lawrance
 * compile out Int3() and Assert() for release demo build
 * 
 *
 * $NoKeywords: $
 *
 */

#ifndef _PSTYPES_H
#define _PSTYPES_H


// Build defines.  Comment in/out for whatever build is necessary:
// #define OEM_BUILD						// enable for OEM builds
// #define MULTIPLAYER_BETA_BUILD				// enable for multiplayer beta build
// #define E3_BUILD							// enable for 3dfx E3 build						
// #define PRESS_TOUR_BUILD			// enable for press tour build
// #define FS2_DEMO					// enable demo build for FS2
// #define PD_BUILD						// fred documentation/evaluation build
//	#define FRENCH_BUILD				// build for French (obsolete)
// #define GERMAN_BUILD				// build for German (this is now used)
#define RELEASE_REAL					// this means that it is an actual release candidate, not just an optimized/release build

// uncomment this #define for DVD version (makes popups say DVD instead of CD 2 or whatever): JCF 5/10/2000
// #define DVD_MESSAGE_HACK


//  #if defined(MULTIPLAYER_BETA_BUILD) || defined(E3_BUILD) || defined(RELEASE_REAL)
//  	#define GAME_CD_CHECK
//  #endif


// 4127 is constant conditional (assert)
// 4100 is unreferenced formal parameters,
// 4514 is unreferenced inline function removed, 
// 4201 is nameless struct extension used. (used by windows header files)
// 4410 illegal size for operand... ie... 	fxch st(1)
// 4611 is _setjmp warning.  Since we use setjmp alot, and we don't really use constructors or destructors, this warning doesn't really apply to us.
// 4725 is the pentium division bug warning, and I can't seem to get rid of it, even with this pragma.
//      JS: I figured out the disabling 4725 works, but not on the first function in the module.
//      So to disable this, I add in a stub function at the top of each module that does nothing.
// 4710 is inline function not expanded (who cares?)
// 4711 tells us an inline function was expanded (who cares?)
// 4702 unreachable code.  I care, but too many to deal with
// 4201 nonstandard extension used : nameless struct/union (happens a lot in Windows include headers)
// 4390 emptry control statement (triggered by nprintf and mprintf's inside of one-line if's, etc)
// 4996 depreciated strcpy, strcat, sprintf, etc. (from MSVC 2005) - taylor
#pragma warning(disable: 4127 4100 4514 4201 4410 4611 4725 4710 4711 4702 4201 4390 4996)

#include <stdio.h>	// For NULL, etc
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#if defined( __x86_64__ ) || defined( _WIN64 )
#define IAM_64BIT 1
#endif

#include "windows_stub/config.h"

// value to represent an uninitialized state in any int or uint
#define UNINITIALIZED 0x7f8e6d9c

#if defined(DEMO) || defined(OEM_BUILD) // no change for FS2_DEMO
	#define MAX_PLAYERS	1
#else
	#define MAX_PLAYERS	12
#endif

#define USE_INLINE_ASM 1		// Define this to use inline assembly
#define STRUCT_CMP(a, b) memcmp((void *) &a, (void *) &b, sizeof(a))

#define LOCAL static			// make module local varilable static.

#ifdef IAM_64BIT
typedef __int32 _fs_time_t;  // time_t here is 64-bit and we need 32-bit
typedef __int32 fix;
// PTR compatible sizes
typedef __int64 ptr_s;
typedef unsigned __int64 ptr_u;
#else
typedef long fix;
typedef	long _fs_time_t;
typedef int ptr_s;
typedef unsigned int ptr_u;
#endif // 64-bit

typedef __int64 longlong;
typedef unsigned __int64 ulonglong;
typedef unsigned char ubyte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;


#define HARDWARE_ONLY

//Stucture to store clipping codes in a word
typedef struct ccodes {
	ubyte cc_or, cc_and;		//or is low byte, and is high byte
} ccodes;

struct vertex;

typedef struct vec3d {
	union {
		struct {
			float x,y,z;
		} xyz;
		float a1d[3];
	};
	inline void operator= (vertex&vert);
	inline void set_screen_vert(vertex&vert);
} vec3d;

// A vector referenced as an array
typedef struct vectora {
	float	xyz[3];
} vectora;

typedef struct vec2d {
	float i,j;
} vec2d;

// Used for some 2d primitives, like gr_poly
typedef struct vert2df {
	float x, y;
} vert2df;

typedef struct angles {
	float	p, b, h;
} angles_t;

typedef struct matrix {
	union {
		struct {
			vec3d	rvec, uvec, fvec;
		} vec;
		float a2d[3][3];
		float a1d[9];
	};
} matrix;

typedef struct uv_pair {
	float u,v;
} uv_pair;

// Used to store rotated points for mines.
// Has flag to indicate if projected.
typedef struct vertex {
	float		x, y, z;				// world space position
	float		sx, sy, sw;			// screen space position (sw == 1/z)
	float		u, v, u2, v2, u3, v3, u4, v4;					// texture position
	vec3d		real_pos;			// _real_ world position
	ubyte spec_a, spec_r, spec_b, spec_g;	//specular highlights -Bobboau
	ubyte		r, g, b, a;			// color.  Use b for darkening;
	ubyte		codes;				// what sides of view pyramid this point is on/off.  0 = Inside view pyramid.
	ubyte		flags;				// Projection flags.  Indicates whether it is projected or not or if projection overflowed.
	ubyte		pad[2];				// pad structure to be 4 byte aligned.
	void operator=(vec3d&vec){
		memcpy(&x,&vec, sizeof(vec3d));
	}
} vertex;

inline void vec3d::operator= (vertex&vert){
	memcpy(this,&vert.x,sizeof(vec3d));
}
//set the vector to the vertex screen position
inline void vec3d::set_screen_vert(vertex&vert){
	memcpy(this,&vert.sx,sizeof(vec3d));
}

extern int spec;

#define	BMP_AABITMAP						(1<<0)				// antialiased bitmap
#define	BMP_TEX_XPARENT						(1<<1)				// transparent texture
#define	BMP_TEX_NONDARK						(1<<2)				// nondarkening texture
#define	BMP_TEX_OTHER						(1<<3)				// so we can identify all "normal" textures
#define BMP_TEX_DXT1						(1<<4)				// dxt1 compressed 8r8g8b1a (24bit)
#define BMP_TEX_DXT3						(1<<5)				// dxt3 compressed 8r8g8b4a (32bit)
#define BMP_TEX_DXT5						(1<<6)				// dxt5 compressed 8r8g8b8a (32bit)
#define BMP_TEX_STATIC_RENDER_TARGET		(1<<7)				// a texture made for being rendered to infreqently
#define BMP_TEX_DYNAMIC_RENDER_TARGET		(1<<8)				// a texture made for being rendered to freqently
#define BMP_TEX_CUBEMAP						(1<<9)				// a texture made for cubic environment map

//compressed texture types
#define BMP_TEX_COMP			( BMP_TEX_DXT1 | BMP_TEX_DXT3 | BMP_TEX_DXT5 )

//non compressed textures
#define BMP_TEX_NONCOMP			( BMP_TEX_XPARENT | BMP_TEX_NONDARK | BMP_TEX_OTHER )

// any texture type
#define	BMP_TEX_ANY				( BMP_TEX_COMP | BMP_TEX_NONCOMP )

typedef struct bitmap {
	short	w, h;		// Width and height
	short	rowsize;	// What you need to add to go to next row
	ubyte	bpp;		// How many bits per pixel it is. (7,8,15,16,24,32) (what is requested)
	ubyte	true_bpp;	// How many bits per pixel the image actually is.
	ubyte	flags;	// See the BMP_???? defines for values
	ptr_u	data;		// Pointer to data, or maybe offset into VRAM.
	ubyte *palette;	// If bpp==8, this is pointer to palette.   If the BMP_NO_PALETTE_MAP flag
							// is not set, this palette just points to the screen palette. (gr_palette)
} bitmap;

//This are defined in MainWin.c
extern void _cdecl WinAssert(char * text,char *filename, int line);
extern void _cdecl Error( char * filename, int line, char * format, ... );
extern void _cdecl Warning( char * filename, int line, char * format, ... );

#include "osapi/outwnd.h"

// To debug printf do this:
// mprintf(( "Error opening %s\n", filename ));
#ifndef NDEBUG
#define mprintf(args) outwnd_printf2 args
#define nprintf(args) outwnd_printf args
#else
#define mprintf(args) 
#define nprintf(args) 
#endif

#define LOCATION __FILE__,__LINE__

// To flag an error, you can do this:
// Error( __FILE__, __LINE__, "Error opening %s", filename );
// or, 
// Error( LOCATION, "Error opening %s", filename );

/*******************NEVER UNCOMMENT Assert ************************************************/
// Please never uncomment the functionality of Assert in debug
// The code, as with all developement like this is littered with Asserts which are designed to throw
// up an error message if variables are out of range.

// Disabling this functionality is dangerous, crazy values can run rampent unchecked and the longer its disabled
// the more likely you are to have problems getting it working again.
#if defined(NDEBUG)
#define Assert(x) do {} while (0)
#else
void gr_activate(int);
#define Assert(x) do { if (!(x)){ gr_activate(0); WinAssert(#x,__FILE__,__LINE__); gr_activate(1); } } while (0)
#endif
/*******************NEVER UNCOMMENT Assert ************************************************/

// Goober5000 - shouldn't the above be never COMMENT (that is, never DISABLE) Assert?

// Goober5000 - define Verify for use in both release and debug mode
#define Verify(x) do { if (!(x)){ Error(LOCATION, "Verify failure: %s\n", #x); } } while(0)


//#define Int3() _asm { int 3 }

#ifdef INTERPLAYQA
	// Interplay QA version of Int3
	#define Int3() do { } while (0) 

	// define to call from Warning function above since it calls Int3, so without this, we
	// get put into infinite dialog boxes
   #ifdef _WIN32
	  #define AsmInt3() _asm { int 3 }
   #else
     #define AsmInt3() abort()
   #endif

#else
	#if defined(NDEBUG)
		// No debug version of Int3
		#define Int3() do { } while (0) 
	#else
		void debug_int3(char *file, int line);

		// Debug version of Int3
		#define Int3() debug_int3(__FILE__, __LINE__)
	#endif	// NDEBUG && DEMO
#endif	// INTERPLAYQA

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif


#define PI					3.141592654f
#define PI2					(3.141592654f*2.0f)	// PI*2
#define ANG_TO_RAD(x)	((x)*PI/180)


extern int	Fred_running;  // Is Fred running, or FreeSpace?
extern int Pofview_running;
extern int Nebedit_running;


//======================================================================================
//======          D E B U G    C O N S O L E   S T U F F        ========================
//======================================================================================

// Here is a a sample command to toggle something that would
// be called by doing "toggle it" from the debug console command window/

/*
DCF(toggle_it,"description")
{
	if (Dc_command)	{
		This_var = !This_var;
	}

	if (Dc_help)	{
		dc_printf( "Usage: sample\nToggles This_var on/off.\n" );
	}

	if (Dc_status)	{
		dc_printf( "The status is %d.\n", This_var );
	}
*/

class debug_command {
	public:
	char *name;
	char *help;
	void (*func)();
	debug_command(char *name,char *help,void (*func)());	// constructor
};

#define DCF(function_name,help_text)			\
		void dcf_##function_name();	\
		debug_command dc_##function_name(#function_name,help_text,dcf_##function_name);	\
		void dcf_##function_name()		

// Starts the debug console
extern void debug_console( void (*func)() = NULL );

// The next three variables tell your function what to do.  It should
// only change something if the dc_command is set.   A minimal function
// needs to process the dc_command.   Usually, these will be called in
// these combinations:
// dc_command=true, dc_status=true  means process it and show status
// dc_help=true means show help only
// dc_status=true means show status only
// I would recommend doing this in each function:
// if (dc_command) { process command }
// if (dc_help) { print out help }
// if (dc_status) { print out status }
// with the last two being optional

extern int Dc_command;	// If this is set, then process the command
extern int Dc_help;		// If this is set, then print out the help text in the form, "usage: ... \nLong description\n" );
extern int Dc_status;		// If this is set, then print out the current status of the command.

void dc_get_arg(uint flags);		// Gets the next argument.   If it doesn't match the flags, this function will print an error and not return.
extern char *Dc_arg;		// The (lowercased) string value of the argument retrieved from dc_arg
extern char *Dc_arg_org;	// Dc_arg before it got converted to lowercase
extern uint Dc_arg_type;	// The type of dc_arg.
extern char *Dc_command_line;		// The rest of the command line, from the end of the last processed arg on.
extern int Dc_arg_int;		// If Dc_arg_type & ARG_INT or ARG_HEX is set, then this is the value
extern float Dc_arg_float;	// If Dc_arg_type & ARG_FLOAT is set, then this is the value

// Outputs text to the console
void dc_printf( char *format, ... );

// Each dc_arg_type can have one or more of these flags set.
// This is because some things can fit into two categories.
// Like 1 can be an integer, a float, a string, or a true boolean
// value.
#define ARG_NONE		(1<<0)	// no argument
#define ARG_ANY		0xFFFFFFFF	// Anything.
#define ARG_STRING	(1<<1)	// any valid string
#define ARG_QUOTE		(1<<2)	// a quoted string
#define ARG_INT		(1<<3)	// a valid integer
#define ARG_FLOAT		(1<<4)	// a valid floating point number

// some specific commonly used predefined types. Can add up to (1<<31)
#define ARG_HEX		(1<<5)	// a valid hexadecimal integer. Note that ARG_INT will always be set also in this case.
#define ARG_TRUE		(1<<6)	// on, true, non-zero number
#define ARG_FALSE		(1<<7)	// off, false, zero
#define ARG_PLUS		(1<<8)	// Plus sign
#define ARG_MINUS		(1<<9)	// Minus sign
#define ARG_COMMA		(1<<10)	// a comma

// A shortcut for boolean only variables.
// Example:  
// DCF_BOOL( lighting, Show_lighting )
//
#define DCF_BOOL( function_name, bool_variable )	\
	void dcf_##function_name();	\
	debug_command dc_##function_name(#function_name,"Toggles "#bool_variable,dcf_##function_name );	\
	void dcf_##function_name()	{	\
	if ( Dc_command )	{	\
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		\
		if ( Dc_arg_type & ARG_TRUE )	bool_variable = 1;	\
		else if ( Dc_arg_type & ARG_FALSE ) bool_variable = 0;	\
		else if ( Dc_arg_type & ARG_NONE ) bool_variable ^= 1;	\
	}	\
	if ( Dc_help )	dc_printf( "Usage: %s [bool]\nSets %s to true or false.  If nothing passed, then toggles it.\n", #function_name, #bool_variable );	\
	if ( Dc_status )	dc_printf( "%s is %s\n", #function_name, (bool_variable?"TRUE":"FALSE") );	\
}


//======================================================================================
//======================================================================================
//======================================================================================


#include "math/fix.h"
#include "math/floating.h"

// Some constants for stuff
#define MAX_FILENAME_LEN	32			// Length for filenames, ie "title.pcx"
#define MAX_PATH_LEN			128		// Length for pathnames, ie "c:\bitmaps\title.pcx"

// contants and defined for byteswapping routines (useful for mac)

#define SWAPSHORT(x)	(							\
						((ubyte)x << 8) |					\
						(((ushort)x) >> 8)			\
						)
						
#define SWAPINT(x)		(							\
						(x << 24) |					\
						(((ulong)x) >> 24) |		\
						((x & 0x0000ff00) << 8) |	\
						((x & 0x00ff0000) >> 8)		\
						)

#ifdef SCP_UNIX
#include "SDL_endian.h"
#endif

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#ifndef BYTE_ORDER
#define BYTE_ORDER	BIG_ENDIAN
#endif // !BYTE_ORDER
#endif // SDL_BYTEORDER

#if BYTE_ORDER == BIG_ENDIAN
// turn off inline asm
#undef USE_INLINE_ASM

// tigital -
inline float SWAPFLOAT(float *x)
{
#if !defined(__MWERKS__)
	// Usage:  void __stwbrx( unsigned int, unsigned int *address, int byteOffsetFromAddress );
#define __stwbrx(value, base, index) \
	__asm__ ( "stwbrx %0, %1, %2" :  : "r" (value), "b%" (index), "r" (base) : "memory" )
#endif

	union
	{
		int		i;
		float	f;
	} buf;

	// load the float into the integer unit
	register int a = ((int*) x)[0];

	// store it to the transfer union, with byteswapping
	__stwbrx(a, 0, &buf.i);

	// load it into the FPU and return it
	return buf.f;
}

#ifdef SCP_UNIX
#define INTEL_INT(x)	SDL_Swap32(x)
#define INTEL_SHORT(x)	SDL_Swap16(x)
#else
#define INTEL_INT(x)	SWAPINT(x)
#define INTEL_SHORT(x)	SWAPSHORT(x)
#endif // (unix)
#define INTEL_FLOAT(x)	SWAPFLOAT(x)

#else // Little Endian -
#define INTEL_INT(x)	x
#define INTEL_SHORT(x)	x
#define INTEL_FLOAT(x)	(*x)
#endif // BYTE_ORDER

#define TRUE	1
#define FALSE	0

int myrand();



// Callback Loading function. 
// If you pass a function to this, that function will get called
// around 10x per second, so you can update the screen.
// Pass NULL to turn it off.
// Call this with the name of a function.  That function will
// then get called around 10x per second.  The callback function
// gets passed a 'count' which is how many times game_busy has
// been called since the callback was set.   It gets called
// one last time with count=-1 when you turn off the callback
// by calling game_busy_callback(NULL).   Game_busy_callback
// returns the current count, so you can tell how many times
// game_busy got called.
// If delta_step is above 0, then it will also make sure it 
// calls the callback each time count steps 'delta_step' even
// if 1/10th of a second hasn't elapsed.
extern int game_busy_callback( void (*callback)(int count), int delta_step = -1 );

// Call whenever loading to display cursor
extern void game_busy(char *filename = NULL);


//=========================================================
// Functions to monitor performance
#ifndef NDEBUG

class monitor {
	public:
	char	*name;			
	int	value;			// Value that gets cleared to 0 each frame.
	int	min, max, sum, cnt;		// Min & Max of value.  Sum is used to calculate average 
	monitor(char *name);	// constructor
};

// Creates a monitor variable
#define MONITOR(function_name)				monitor mon_##function_name(#function_name)

// Increments a monitor variable
#define MONITOR_INC(function_name,inc)		do { mon_##function_name.value+=(inc); } while(0)

// Call this once per frame to update monitor file
void monitor_update();

#else

#define MONITOR(function_name)

#define MONITOR_INC(function_name,inc)		do { } while(0)

// Call this once per frame to update monitor file
#define monitor_update() do { } while(0)

#endif

#define NOX(s) s

char *XSTR(char *str, int index);

// Caps V between MN and MX.
template <class T> void CAP( T& v, T mn, T mx )
{
	if ( v < mn )	{
		v = mn;
	} else if ( v > mx )	{
		v = mx;
	}
}

// ========================================================
// stamp checksum stuff
// ========================================================

// here is the define for the stamp for this set of code
#define STAMP_STRING "\001\001\001\001\002\002\002\002Read the Foundation Novels from Asimov.  I liked them." 
#define STAMP_STRING_LENGTH	80
#define DEFAULT_CHECKSUM_STRING		"\001\001\001\001"
#define DEFAULT_TIME_STRING			"\002\002\002\002"

// macro to calculate the checksum for the stamp.  Put here so that we can use different methods
// for different applications.  Requires the local variable 'checksum' to be defined!!!
#define CALCULATE_STAMP_CHECKSUM() do {	\
		int i, found;	\
							\
		checksum = 0;	\
		for ( i = 0; i < (int)strlen(ptr); i++ ) {	\
			found = 0;	\
			checksum += ptr[i];	\
			if ( checksum & 0x01 )	\
				found = 1;	\
			checksum = checksum >> 1;	\
			if (found)	\
				checksum |= 0x80000000;	\
		}	\
		checksum |= 0x80000000;	\
	} while (0) ;

//=========================================================
// Memory management functions
//=========================================================

// Returns 0 if not enough RAM.
int vm_init(int min_heap_size);

// Frees all RAM.
void vm_free_all();

#ifndef NDEBUG
	// Debug versions

	// Allocates some RAM.
	void *_vm_malloc( int size, char *filename=NULL, int line=-1 );

	// allocates some RAM for a string
	char *_vm_strdup( const char *ptr, char *filename, int line );

	// Frees some RAM. 
	void _vm_free( void *ptr, char *filename=NULL, int line=-1 );

	// reallocates some RAM
	void *_vm_realloc( void *ptr, int size, char *filename = NULL, int line=-1);

	// Easy to use macros
	#define vm_malloc(size) _vm_malloc((size),__FILE__,__LINE__)
	#define vm_free(ptr) _vm_free((ptr),__FILE__,__LINE__)
	#define vm_strdup(ptr) _vm_strdup((ptr),__FILE__,__LINE__)
	#define vm_realloc(ptr, size) _vm_realloc((ptr),(size),__FILE__,__LINE__)
	
#else
	// Release versions

	// Allocates some RAM.
	void *_vm_malloc( int size );

	// allocates some RAM for a string
	char *_vm_strdup( const char *ptr );

	// Frees some RAM. 
	void _vm_free( void *ptr );

	// reallocates some RAM
	void *_vm_realloc( void *ptr, int size );

	// Easy to use macros
	#define vm_malloc(size) _vm_malloc(size)
	#define vm_free(ptr) _vm_free(ptr)
	#define vm_strdup(ptr) _vm_strdup(ptr)
	#define vm_realloc(ptr, size) _vm_realloc((ptr),(size))


#endif


#endif		// PS_TYPES_H
