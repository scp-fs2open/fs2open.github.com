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


#include <stdio.h>	// For NULL, etc
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "globalincs/toolchain.h"

#if defined( __x86_64__ ) || defined( _WIN64 )
#define IAM_64BIT 1
#endif


#include "windows_stub/config.h"

// value to represent an uninitialized state in any int or uint
#define UNINITIALIZED 0x7f8e6d9c

#define MAX_PLAYERS	12

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

typedef struct vec4 {
	union {
		struct {
			float x,y,z,w;
		} xyzw;
		float a1d[4];
	};
} vec4;

/** Represents a point in 3d space.

Note: this is a struct, not a class, so no member functions. */
typedef struct vec3d {
	union {
		struct {
			float x,y,z;
		} xyz;
		float a1d[3];
	};
} vec3d;

/** Compares two vec3ds */
inline bool operator==(const vec3d &self, const vec3d &other)
{
	return (self.xyz.x == other.xyz.x
		&& self.xyz.y == other.xyz.y
		&& self.xyz.z == other.xyz.z
	);
}

typedef struct vec2d {
	float x, y;
} vec2d;

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

typedef struct matrix4 {
	union {
		struct {
			vec4 rvec, uvec, fvec, pos;
		} vec;
		float a2d[4][4];
		float a1d[16];
	};
} matrix4;

typedef struct uv_pair {
	float u,v;
} uv_pair;

/** Compares two uv_pairs */
inline bool operator==(const uv_pair &left, const uv_pair &right)
{
	return (left.u == right.u) && (left.v == right.v);
}

/** Represents a point in 3d screen space. 'w' is 1/z.

Like vec3d but for screens.

Note: this is a struct, not a class, so no member functions. */
typedef struct screen3d
{
	union {
		struct {
			float x,y,w;
		} xyw;
		float a1d[3];
	};
} screen3d;

/** Compares two screen3ds */
inline bool operator==(const screen3d &self, const screen3d &other)
{
	return (self.xyw.x == other.xyw.x
		&& self.xyw.y == other.xyw.y
		&& self.xyw.w == other.xyw.w
	);
}

/** Used to store rotated points for mines. Has flag to indicate if projected.

Note: this is a struct, not a class, so no memeber functions. */
typedef struct vertex {
	vec3d		world;				// world space position
	screen3d	screen;				// screen space position (sw == 1/z)
	uv_pair		texture_position;	// texture position
	ubyte		r, g, b, a;			// color.  Use b for darkening;
	ubyte		spec_r, spec_g, spec_b, spec_a;	//specular highlights -Bobboau
	ubyte		codes;				// what sides of view pyramid this point is on/off.  0 = Inside view pyramid.
	ubyte		flags;				// Projection flags.  Indicates whether it is projected or not or if projection overflowed.
	ubyte		pad[2];				// pad structure to be 4 byte aligned.
} vertex;

typedef struct effect_vertex {
	vec3d position;
	uv_pair tex_coord;
	float radius;
	ubyte r, g, b, a;
} effect_vertex;

struct particle_pnt {
	vec3d position;
	float size;
	vec3d up;
};

struct trail_shader_info {
	vec3d pos;
	vec3d fvec;

	float intensity;
	float width;
	uv_pair tex_coord;
};

//def_list
typedef struct flag_def_list {
	char *name;
	int def;
	ubyte var;
} def_list;

// weapon count list (mainly for pilot files)
typedef struct wep_t {
	int index;
	int count;
} wep_t;

typedef struct coord2d {
	int x,y;
} coord2d;

//This are defined in MainWin.c
extern void _cdecl WinAssert(char * text,char *filename, int line);
void _cdecl WinAssert(char * text, char * filename, int linenum, SCP_FORMAT_STRING const char * format, ... ) SCP_FORMAT_STRING_ARGS(4, 5);
extern void LuaError(struct lua_State *L, SCP_FORMAT_STRING const char *format=NULL, ...) SCP_FORMAT_STRING_ARGS(2, 3);
extern void _cdecl Error( const char * filename, int line, SCP_FORMAT_STRING const char * format, ... ) SCP_FORMAT_STRING_ARGS(3, 4);
extern void _cdecl Warning( char * filename, int line, SCP_FORMAT_STRING const char * format, ... ) SCP_FORMAT_STRING_ARGS(3, 4);
extern void _cdecl WarningEx( char *filename, int line, SCP_FORMAT_STRING const char *format, ... ) SCP_FORMAT_STRING_ARGS(3, 4);
extern void _cdecl ReleaseWarning(char *filename, int line, SCP_FORMAT_STRING const char *format, ...) SCP_FORMAT_STRING_ARGS(3, 4);

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
// The code, as with all development like this is littered with Asserts which are designed to throw
// up an error message if variables are out of range.
// Disabling this functionality is dangerous, crazy values can run rampent unchecked and the longer its disabled
// the more likely you are to have problems getting it working again.
#if defined(NDEBUG)
#	define Assert(expr) do { ASSUME(expr); } while (0)
#else
	void gr_activate(int);
#	define Assert(expr) do {\
		if (!(expr)) {\
			WinAssert(#expr,__FILE__,__LINE__);\
		}\
		ASSUME( expr );\
	} while (0)
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

#if defined(NDEBUG)
	// No debug version of Int3
	#define Int3() do { } while (0)
#else
	void debug_int3(char *file, int line);

	// Debug version of Int3
	#define Int3() debug_int3(__FILE__, __LINE__)
#endif	// NDEBUG

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
const float RAND_MAX_1f	= (1.0f / RAND_MAX);

#define ANG_TO_RAD(x)	((x)*PI/180)


extern int Fred_running;  // Is Fred running, or FreeSpace?


//======================================================================================
//======================================================================================
//======================================================================================


#include "math/fix.h"
#include "math/floating.h"

// Some constants for stuff
#define MAX_FILENAME_LEN	32		// Length for filenames, ie "title.pcx"
#define MAX_PATH_LEN		256		// Length for pathnames, ie "c:\bitmaps\title.pcx"

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

#ifdef SCP_SOLARIS // Solaris
#define INTEL_INT(x)	x
#define INTEL_SHORT(x)	x
#define INTEL_FLOAT(x)	(*x)
#elif BYTE_ORDER == BIG_ENDIAN
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
extern void game_busy(const char *filename = NULL);

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

const char *XSTR(const char *str, int index);

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
} script_hook;

extern void script_hook_init(script_hook *hook);
extern bool script_hook_valid(script_hook *hook);

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

#include "globalincs/vmallocator.h"
#include "globalincs/safe_strings.h"

// DEBUG compile time catch for dangerous uses of memset/memcpy/memmove
// would prefer std::is_trivially_copyable but it's not supported by gcc yet
// ref: http://gcc.gnu.org/onlinedocs/libstdc++/manual/status.html
#ifndef NDEBUG
	#if defined(HAVE_CXX11)
	// feature support seems to be: gcc   clang   msvc
	// auto                         4.4   2.9     2010
	// std::is_trivial              4.5   ?       2012 (2010 only duplicates std::is_pod)
	// static_assert                4.3   2.9     2010
	#include <type_traits>
	#include <cstring>

	// MEMSET!
	const auto ptr_memset = std::memset;
	#define memset memset_if_trivial_else_error

	template<typename T>
	void *memset_if_trivial_else_error(T *memset_data, int ch, size_t count)
	{
		static_assert(std::is_trivial<T>::value, "memset on non-trivial object");
		return ptr_memset(memset_data, ch, count);
	}

	// assume memset on a void* is "safe"
	// only used in cutscene/mveplayer.cpp:mve_video_createbuf()
	inline void *memset_if_trivial_else_error(void *memset_data, int ch, size_t count)
	{
		return ptr_memset(memset_data, ch, count);
	}

	// MEMCPY!
	const auto ptr_memcpy = std::memcpy;
	#define memcpy memcpy_if_trivial_else_error

	template<typename T, typename U>
	void *memcpy_if_trivial_else_error(T *memcpy_dest, U *src, size_t count)
	{
		static_assert(std::is_trivial<T>::value, "memcpy on non-trivial object T");
		static_assert(std::is_trivial<U>::value, "memcpy on non-trivial object U");
		return ptr_memcpy(memcpy_dest, src, count);
	}

	// assume memcpy with void* is "safe"
	// used in:
	//   globalincs/systemvars.cpp:insertion_sort()
	//   network/chat_api.cpp:AddChatCommandToQueue()
	//   network/multi_obj.cpp:multi_oo_sort_func()
	//   parse/lua.cpp:ade_get_args() && ade_set_args()
	//
	// probably should setup a static_assert on insertion_sort as well
	template<typename U>
	void *memcpy_if_trivial_else_error(void *memcpy_dest, U *memcpy_src, size_t count)
	{
		static_assert(std::is_trivial<U>::value, "memcpy on non-trivial object U");
		return ptr_memcpy(memcpy_dest, memcpy_src, count);
	}

	template<typename T>
	void *memcpy_if_trivial_else_error(T *memcpy_dest, void *memcpy_src, size_t count)
	{
		static_assert(std::is_trivial<T>::value, "memcpy on non-trivial object T");
		return ptr_memcpy(memcpy_dest, memcpy_src, count);
	}
	template<typename T>
	void *memcpy_if_trivial_else_error(T *memcpy_dest, const void *memcpy_src, size_t count)
	{
		static_assert(std::is_trivial<T>::value, "memcpy on non-trivial object T");
		return ptr_memcpy(memcpy_dest, memcpy_src, count);
	}

	inline void *memcpy_if_trivial_else_error(void *memcpy_dest, void *memcpy_src, size_t count)
	{
		return ptr_memcpy(memcpy_dest, memcpy_src, count);
	}

	// MEMMOVE!
	const auto ptr_memmove = std::memmove;
	#define memmove memmove_if_trivial_else_error

	template<typename T, typename U>
	void *memmove_if_trivial_else_error(T *memmove_dest, U *memmove_src, size_t count)
	{
		static_assert(std::is_trivial<T>::value, "memmove on non-trivial object T");
		static_assert(std::is_trivial<U>::value, "memmove on non-trivial object U");
		return ptr_memmove(memmove_dest, memmove_src, count);
	}
	#endif // HAVE_CXX11
#endif // NDEBUG

#endif		// PS_TYPES_H
