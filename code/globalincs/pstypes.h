/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _PSTYPES_H
#define _PSTYPES_H


// Build defines.  Comment in/out for whatever build is necessary:
// #define OEM_BUILD				// enable for OEM builds
// #define MULTIPLAYER_BETA_BUILD	// enable for multiplayer beta build
// #define E3_BUILD					// enable for 3dfx E3 build
// #define PRESS_TOUR_BUILD			// enable for press tour build
// #define FS2_DEMO					// enable demo build for FS2
// #define PD_BUILD					// fred documentation/evaluation build
// #define FRENCH_BUILD			// build for French (obsolete)
// #define GERMAN_BUILD				// build for German (this is now used)
#define RELEASE_REAL				// this means that it is an actual release candidate, not just an optimized/release build

// uncomment this #define for DVD version (makes popups say DVD instead of CD 2 or whatever): JCF 5/10/2000
// #define DVD_MESSAGE_HACK


//  #if defined(MULTIPLAYER_BETA_BUILD) || defined(E3_BUILD) || defined(RELEASE_REAL)
//  	#define GAME_CD_CHECK
//  #endif

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

#ifdef _WIN32
#define DIR_SEPARATOR_CHAR '\\'
#define DIR_SEPARATOR_STR  "\\"
#else
#define DIR_SEPARATOR_CHAR '/'
#define DIR_SEPARATOR_STR  "/"
#endif

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

	bool operator == (const vec3d &other);
} vec3d;

inline bool vec3d::operator == (const vec3d &other)
{
	return ( (a1d[0] == other.a1d[0]) && (a1d[1] == other.a1d[1]) && (a1d[2] == other.a1d[2]) );
}

/*
// A vector referenced as an array
typedef struct vectora {
	float xyz[3];
} vectora;
*/

typedef struct vec2d {
	float x, y;
} vec2d;

/*
// Used for some 2d primitives, like gr_poly
typedef struct vert2df {
	float x, y;
} vert2df;
*/

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
	float		x, y, z;			// world space position
	float		sx, sy, sw;			// screen space position (sw == 1/z)
	float		u, v;				// texture position
//	float		u2, v2, u3, v3, u4, v4;	// texture position
//	vec3d		real_pos;			// _real_ world position
	ubyte		r, g, b, a;			// color.  Use b for darkening;
	ubyte		spec_r, spec_g, spec_b, spec_a;	//specular highlights -Bobboau
	ubyte		codes;				// what sides of view pyramid this point is on/off.  0 = Inside view pyramid.
	ubyte		flags;				// Projection flags.  Indicates whether it is projected or not or if projection overflowed.
	ubyte		pad[2];				// pad structure to be 4 byte aligned.
	void operator=(vec3d&vec) {
		memcpy(&x,&vec, sizeof(vec3d));
	}

	bool operator == (const vertex &other);
} vertex;

inline bool vertex::operator == (const vertex &other)
{
	// NOTE: this is checking position and uv only!
	return ( (x == other.x) && (y == other.y) && (z == other.z)
				&& (u == other.u) && (v == other.v) );
}

inline void vec3d::operator= (vertex&vert) {
	memcpy(this,&vert.x,sizeof(vec3d));
}
//set the vector to the vertex screen position
inline void vec3d::set_screen_vert(vertex&vert) {
	memcpy(this,&vert.sx,sizeof(vec3d));
}

//def_list
typedef struct flag_def_list {
	char *name;
	int def;
	ubyte var;
} def_list;

//This are defined in MainWin.c
extern void _cdecl WinAssert(char * text,char *filename, int line);
void _cdecl WinAssert(char * text, char * filename, int linenum, const char * format, ... );
extern void LuaError(struct lua_State *L, char *format=NULL, ...);
extern void _cdecl Error( const char * filename, int line, const char * format, ... );
extern void _cdecl Warning( char * filename, int line, const char * format, ... );
extern void _cdecl WarningEx( char *filename, int line, const char *format, ... );

extern int Global_warning_count;
extern int Global_error_count;

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

#define ASSUME(x)

// Disabling this functionality is dangerous, crazy values can run rampent unchecked and the longer its disabled
// the more likely you are to have problems getting it working again.
#if defined(NDEBUG)
#	define Assert(x) do { ASSUME(x); } while (0)
#	ifndef _MSC_VER   // non MS compilers
#		define Assertion(x, y, ...) do {} while (0)
#	else
#		if _MSC_VER >= 1400	// VC 2005 or greater
#			define Assertion(x, y, ...) do { ASSUME(x); } while (0)
#		else
#			define Assertion(x, y) do {} while (0)
#		endif
#	endif
#else
	void gr_activate(int);
#	define Assert(x) do { if (!(x)){ WinAssert(#x,__FILE__,__LINE__); } ASSUME( x ); } while (0)

	// Assertion can only use its proper fuctionality in compilers that support variadic macro
#	ifndef _MSC_VER   // non MS compilers
#		define Assertion(x, y, ...) do { if (!(x)){ WinAssert(#x,__FILE__,__LINE__, y , ##__VA_ARGS__ ); } } while (0)
#	else
#		if _MSC_VER >= 1400	// VC 2005 or greater
#			define Assertion(x, y, ...) do { if (!(x)){ WinAssert(#x,__FILE__,__LINE__, y, __VA_ARGS__ ); } ASSUME(x); } while (0)
#		else // everything else
#			define Assertion(x, y) do { if (!(x)){ WinAssert(#x,__FILE__,__LINE__); } } while (0)
#		endif
#	endif
#endif
/*******************NEVER COMMENT Assert ************************************************/

// Goober5000 - define Verify for use in both release and debug mode
#define Verify(x) do { if (!(x)){ Error(LOCATION, "Verify failure: %s\n", #x); } ASSUME(x); } while(0)

// VerifyEx
#ifndef _MSC_VER   // non MS compilers
#	define VerifyEx(x, y, ...) do { if (!(x)) { Error(LOCATION, "Verify failure: %s with help text " #y "\n", #x, ##__VA_ARGS__); } ASSUME(x); } while(0)
#else 
#	if _MSC_VER >= 1400	// VC 2005 or greater
#		define VerifyEx(x, y, ...) do { if (!(x)) { Error(LOCATION, "Verify failure: %s with help text " #y "\n", #x, __VA_ARGS__); } ASSUME(x); } while(0)
#	else // everything else
#		define VerifyEx(x, y) Verify(x)
#	endif
#endif

//#define Int3() _asm { int 3 }

#ifdef INTERPLAYQA
	// Interplay QA version of Int3
	#define Int3() do { } while (0) 

	// define to call from Warning function above since it calls Int3, so without this, we
	// get put into infinite dialog boxes
	#ifdef _WIN32
		#define AsmInt3() _asm { int 3 }
	#else
		#define AsmInt3() exit(EXIT_FAILURE)
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


#define PI				3.141592654f
// twice values
const float PI2			= (PI*2.0f);
// half values
const float PI_2		= (PI/2.0f);
const int RAND_MAX_2	= (RAND_MAX/2);

#define ANG_TO_RAD(x)	((x)*PI/180)


extern int Fred_running;  // Is Fred running, or FreeSpace?


//======================================================================================
//======          D E B U G    C O N S O L E   S T U F F        ========================
//======================================================================================

// Here is a a sample command to toggle something that would
// be called by doing "toggle it" from the debug console command window/

/*
DCF(toggle_it,"description")
{
	if (Dc_command) {
		This_var = !This_var;
	}

	if (Dc_help) {
		dc_printf( "Usage: sample\nToggles This_var on/off.\n" );
	}

	if (Dc_status) {
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

#define DCF(function_name,help_text)	\
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

extern int Dc_command;			// If this is set, then process the command
extern int Dc_help;				// If this is set, then print out the help text in the form, "usage: ... \nLong description\n" );
extern int Dc_status;			// If this is set, then print out the current status of the command.

void dc_get_arg(uint flags);	// Gets the next argument.   If it doesn't match the flags, this function will print an error and not return.
extern char *Dc_arg;			// The (lowercased) string value of the argument retrieved from dc_arg
extern char *Dc_arg_org;		// Dc_arg before it got converted to lowercase
extern uint Dc_arg_type;		// The type of dc_arg.
extern char *Dc_command_line;	// The rest of the command line, from the end of the last processed arg on.
extern int Dc_arg_int;			// If Dc_arg_type & ARG_INT or ARG_HEX is set, then this is the value
extern ubyte Dc_arg_ubyte;		// If Dc_arg_type & ARG_UBYTE is set, then this is the value
extern float Dc_arg_float;		// If Dc_arg_type & ARG_FLOAT is set, then this is the value

// Outputs text to the console
void dc_printf( char *format, ... );

// Each dc_arg_type can have one or more of these flags set.
// This is because some things can fit into two categories.
// Like 1 can be an integer, a float, a string, or a true boolean
// value.
#define ARG_NONE		(1<<0)		// no argument
#define ARG_ANY			0xFFFFFFFF	// Anything.
#define ARG_STRING		(1<<1)		// any valid string
#define ARG_QUOTE		(1<<2)		// a quoted string
#define ARG_INT			(1<<3)		// a valid integer
#define ARG_FLOAT		(1<<4)		// a valid floating point number

// some specific commonly used predefined types. Can add up to (1<<31)
#define ARG_HEX			(1<<5)		// a valid hexadecimal integer. Note that ARG_INT will always be set also in this case.
#define ARG_TRUE		(1<<6)		// on, true, non-zero number
#define ARG_FALSE		(1<<7)		// off, false, zero
#define ARG_PLUS		(1<<8)		// Plus sign
#define ARG_MINUS		(1<<9)		// Minus sign
#define ARG_COMMA		(1<<10)		// a comma
#define ARG_UBYTE		(1<<11)		// a valid ubyte

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
#define MAX_FILENAME_LEN	32		// Length for filenames, ie "title.pcx"
#define MAX_PATH_LEN		128		// Length for pathnames, ie "c:\bitmaps\title.pcx"

// contants and defined for byteswapping routines (useful for mac)

#define SWAPSHORT(x)	(							\
						((ubyte)x << 8) |			\
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
int rand32(); // returns a random number between 0 and 0x7fffffff


// lod checker for (modular) table parsing
typedef struct lod_checker {
	char filename[MAX_FILENAME_LEN];
	int num_lods;
	int override;
} lod_checker;


// check to see that a passed sting is valid, ie:
//  - has >0 length
//  - is not "none"
//  - is not "<none>"
#define VALID_FNAME(x) ( strlen((x)) && stricmp((x), "none") && stricmp((x), "<none>") )


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
	int	value;					// Value that gets cleared to 0 each frame.
	int	min, max, sum, cnt;		// Min & Max of value.  Sum is used to calculate average 
	monitor(char *name);		// constructor
};

// Creates a monitor variable
#define MONITOR(function_name)				monitor mon_##function_name(#function_name);

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
	if ( v < mn ) {
		v = mn;
	} else if ( v > mx ) {
		v = mx;
	}
}

// faster version of CAP()
#define CLAMP(x, min, max) do { if ( (x) < (min) ) (x) = (min); else if ((x) > (max)) (x) = (max); } while(0)

// ========================================================
// stamp checksum stuff
// ========================================================

// here is the define for the stamp for this set of code
#define STAMP_STRING "\001\001\001\001\002\002\002\002Read the Foundation Novels from Asimov.  I liked them." 
#define STAMP_STRING_LENGTH			80
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
	void *_vm_malloc( int size, char *filename = NULL, int line = -1, int quiet = 0 );

	// allocates some RAM for a string
	char *_vm_strdup( const char *ptr, char *filename, int line );

	// allocates some RAM for a string of a certain length
	char *_vm_strndup( const char *ptr, int size, char *filename, int line );

	// Frees some RAM. 
	void _vm_free( void *ptr, char *filename = NULL, int line= -1 );

	// reallocates some RAM
	void *_vm_realloc( void *ptr, int size, char *filename = NULL, int line= -1, int quiet = 0 );

	// Easy to use macros
	#define vm_malloc(size) _vm_malloc((size),__FILE__,__LINE__,0)
	#define vm_free(ptr) _vm_free((ptr),__FILE__,__LINE__)
	#define vm_strdup(ptr) _vm_strdup((ptr),__FILE__,__LINE__)
	#define vm_strndup(ptr, size) _vm_strndup((ptr),(size),__FILE__,__LINE__)
	#define vm_realloc(ptr, size) _vm_realloc((ptr),(size),__FILE__,__LINE__,0)

	// quiet macro versions which don't report errors
	#define vm_malloc_q(size) _vm_malloc((size),__FILE__,__LINE__,1)
	#define vm_realloc_q(ptr, size) _vm_realloc((ptr),(size),__FILE__,__LINE__,1)
#else
	// Release versions

	// Allocates some RAM.
	void *_vm_malloc( int size, int quiet = 0 );

	// allocates some RAM for a string
	char *_vm_strdup( const char *ptr );

	// allocates some RAM for a strings of a certain length
	char *_vm_strndup( const char *ptr, int size );

	// Frees some RAM. 
	void _vm_free( void *ptr );

	// reallocates some RAM
	void *_vm_realloc( void *ptr, int size, int quiet = 0 );

	// Easy to use macros
	#define vm_malloc(size) _vm_malloc((size),0)
	#define vm_free(ptr) _vm_free(ptr)
	#define vm_strdup(ptr) _vm_strdup(ptr)
	#define vm_strndup(ptr, size) _vm_strndup((ptr),(size))
	#define vm_realloc(ptr, size) _vm_realloc((ptr),(size),0)

	// quiet macro versions which don't report errors
	#define vm_malloc_q(size) _vm_malloc((size),1)
	#define vm_realloc_q(ptr, size) _vm_realloc((ptr),(size),1)

#endif

#include "globalincs/fsmemory.h"

//=========================================================
// Scripting
//=========================================================
//-WMC
typedef struct script_hook
{
	//Override
	int o_language;
	int o_index;
	//Actual hook
	int h_language;
	int h_index;

	script_hook(){o_language=h_language=0;o_index=h_index=-1;}
	bool IsValid(){return (h_index > -1);}
}script_hook;

class camid
{
private:
	int sig;
	uint idx;
public:
	camid();
	camid(int n_idx, int n_sig);

	class camera *getCamera();
	uint getIndex();
	int getSignature();
	bool isValid();
};

/* Restrict keyword semantics are different under VC and GCC */

#ifndef NO_RESTRICT_USE
#	ifdef _MSC_VER
#		if _MSC_VER >= 1400
#			define RESTRICT __restrict
#		else
#			define RESTRICT
#		endif
#	else
#		define RESTRICT restrict
#	endif
#else
#	define RESTRICT
#endif

#include "globalincs/vmallocator.h"
#include "globalincs/safe_strings.h"


#endif		// PS_TYPES_H
