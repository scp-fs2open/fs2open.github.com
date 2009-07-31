#ifndef _SAFE_STRINGS_H_INCLUDED_
#define _SAFE_STRINGS_H_INCLUDED_

/* It is a condition of use that safe_strings.cpp, safe_strings.h, safe_strings_test.cpp remain together.
 *
 * Maintained by portej05 - contact via PM on www.hard-light.net/forums
 * Why have we got this, what is it for?
 * VC2005+ define some safe string functions which check buffer sizes before doing anything
 * Unfortunately, GCC and MACOS do not provide these functions, therefore, we must!
 * (if only to reduce the amount of noise the static analysis tools are spitting out)
 * They are part of ISO/IEC TR 24731 and may find their way into the CRTs at some point, at which
 * point these functions must be removed from the engine.
 * While these functions do not add a huge amount of benefit for heap-allocated strings, they
 * can protect against a class of buffer overruns in stack allocated situations.
 *
 */

/* Rules of use:
 * 1) VC2005 & VC2008 define their own, so don't do anything if they're around
 * 2) MacOS and GCC will use safe_strings
 * 3) All others will be redirected to non-safe versions
 */

/* errno_t, EINVAL, ERANGE, etc.. */
#include <errno.h>
#include <stdlib.h> /* size_t */

/* Because errno_t is not (yet) standard, we define it here like this */
typedef int errno_t;

#if !defined( _MSC_VER ) && !defined(NO_SAFE_STRINGS) /* These are already present in 2005, 2008 - if you're using anything else, you don't get these */

/* In order to compile safe_strings_test.cpp, you must have this defined on the command line */
/* #define SAFESTRINGS_TEST_APP */

/* Unlike their CRT counterparts, these do not call the invalid parameter handler
 * However, they do call this macro
 */
#ifndef SAFESTRINGS_TEST_APP

#	ifndef __safe_strings_error_handler
#		ifdef _DEBUG
#			define __safe_strings_error_handler( val ) Assertion(0,"%s: String error @ %s (%d). Please Report\nTrying to put into %d byte buffer:\n%s", #val, file, line,sizeInBytes,strSource) /* Crash hard here - no better option outside of a cross platform framework */
#		else
#			define __safe_strings_error_handler( val ) 1/0 /* Crash hard here - no better option outside of a cross platform framework */
#		endif
#	endif

#else

/* For testing only */
#	define __safe_strings_error_handler( errnoVal ) extern void error_handler( int errnoValue, const char* errnoStr,  const char* file, const char* function, int line );\
																error_handler( errnoVal, #errnoVal, __FILE__, __FUNCTION__, __LINE__ );
#endif

#ifdef NDEBUG

extern errno_t strcpy_s( char* strDest, size_t sizeInBytes, const char* strSource );
extern errno_t strcat_s( char* strDest, size_t sizeInBytes, const char* strSource );

/* Template helpers */
template< size_t size>
inline
errno_t strcpy_s( char (&strDest)[ size ], const char* strSource )
{
	return strcpy_s( strDest, size, strSource );
}

template< size_t size >
inline
errno_t strcat_s( char (&strDest)[ size ], const char* strSource )
{
	return strcat_s( strDest, size, strSource );
}

#else

extern errno_t scp_strcpy_s( const char* file, int line, char* strDest, size_t sizeInBytes, const char* strSource );
extern errno_t scp_strcat_s( const char* file, int line, char* strDest, size_t sizeInBytes, const char* strSource );

template< size_t size>
inline
errno_t scp_strcpy_s( const char* file, int line, char (&strDest)[ size ], const char* strSource )
{
	return scp_strcpy_s( file, line, strDest, size, strSource );
}

template< size_t size >
inline
errno_t scp_strcat_s( const char* file, int line, char (&strDest)[ size ], const char* strSource )
{
	return scp_strcat_s( file, line, strDest, size, strSource );
}

#define strcpy_s( ... ) scp_strcpy_s( __FILE__, __LINE__, __VA_ARGS__ )
#define strcat_s( ... ) scp_strcpy_s( __FILE__, __LINE__, __VA_ARGS__ )

#endif

/* Safe strings for VS2005+ in debug builds (these give more info than the MS CRT ones) */
#elif defined( _MSC_VER ) && _MSC_VER >= 1400 && !defined(NDEBUG) && !defined(NO_SAFE_STRINGS)

#ifndef __safe_strings_error_handler
#	define __safe_strings_error_handler( val ) Assertion(0,"%s: String error @ %s (%d). Please Report\nTrying to put into %d byte buffer:\n%s", #val, file, line,sizeInBytes,strSource) /* Crash hard here - no better option outside of a cross platform framework */
#endif

extern errno_t scp_strcpy_s( const char* file, int line, char* strDest, size_t sizeInBytes, const char* strSource );
extern errno_t scp_strcat_s( const char* file, int line, char* strDest, size_t sizeInBytes, const char* strSource );

template< size_t size>
inline
errno_t scp_strcpy_s( const char* file, int line, char (&strDest)[ size ], const char* strSource )
{
	return scp_strcpy_s( file, line, strDest, size, strSource );
}

template< size_t size >
inline
errno_t scp_strcat_s( const char* file, int line, char (&strDest)[ size ], const char* strSource )
{
	return scp_strcat_s( file, line, strDest, size, strSource );
}

#define strcpy_s( ... ) scp_strcpy_s( __FILE__, __LINE__, __VA_ARGS__ )
#define strcat_s( ... ) scp_strcat_s( __FILE__, __LINE__, __VA_ARGS__ )

#else

#pragma message("safe_strings disabled - this is not good!")

inline errno_t strcpy_s( char* strDest, size_t sizeInBytes, const char* strSource )
{ 
	strcpy( strDest, strSource ); 
	return 0;
}

inline errno_t strcat_s( char* strDest, size_t sizeInBytes, const char* strSource )
{
	strcat( strDest, strSource ); 
	return 0;
}

inline errno_t strcpy_s( char* strDest, const char* strSource )
{ 
	strcpy( strDest, strSource );
	return 0;
}

inline errno_t strcat_s( char* strDest, const char* strSource )
{ 
	strcat( strDest, strSource ); 
	return 0;
}

#endif

#endif // _SAFE_STRINGS_H_INCLUDED_
