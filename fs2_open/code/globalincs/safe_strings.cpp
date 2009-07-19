#include "pstypes.h"

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

/* There is no safe strings below VS2005
 * scp safe_strings are used in VS2005+ DEBUG because they give more info
 */

#if !defined( _MSC_VER ) || ( defined( _MSC_VER ) && _MSC_VER >= 1400 && !defined(NDEBUG) )

/* We don't have this here - no standard library stuff included */
#ifndef NULL
#define NULL 0
#endif

/* An implementation of strcpy_s 
 * We're not going to actually fully behave like the MS debug version.
 */
#ifndef NDEBUG
errno_t scp_strcpy_s( const char* file, int line, char* strDest, size_t sizeInBytes, const char* strSource )
#else
errno_t strcpy_s( char* strDest, size_t sizeInBytes, const char* strSource )
#endif
{
	size_t bufferLeft = sizeInBytes;
	
	if ( !strDest || !strSource )
	{
		if ( strDest )
			*strDest = NULL;
		__safe_strings_error_handler( EINVAL );
		return EINVAL;
	}

	if ( sizeInBytes == 0 )
	{
		*strDest = NULL;
		__safe_strings_error_handler( ERANGE );
		return ERANGE;
	}

	char* p = strDest;
	
	while ((*p++ = *strSource++) != 0 && --bufferLeft > 0);

	for ( ; *strSource && bufferLeft; bufferLeft-- )
	{
		*p = *strSource;
		strSource++; p++;
	}

	if ( bufferLeft == 0 )
	{
		*strDest = NULL;
		__safe_strings_error_handler( ERANGE );
		return ERANGE;
	}

	return 0;
}

#ifndef NDEBUG
errno_t scp_strcat_s( const char* file, int line, char* strDest, size_t sizeInBytes, const char* strSource )
#else
errno_t strcat_s( char* strDest, size_t sizeInBytes, const char* strSource )
#endif
{
	char* p;
	size_t bufferLeft = sizeInBytes;

	if ( !strDest || !strSource )
	{
		if ( strDest )
			*strDest = NULL;
		__safe_strings_error_handler( EINVAL );
		return EINVAL;
	}

	if ( bufferLeft == 0 )
	{
		*strDest = NULL;
		__safe_strings_error_handler( ERANGE );
		return ERANGE;
	}

	/* Find the terminating NULL of the input string */
	p = strDest;
	while ( *p )
	{
		p++;
		bufferLeft--;
	}

	if ( bufferLeft == 0 )
	{
		*strDest = NULL;
		__safe_strings_error_handler( ERANGE );
		return ERANGE;
	}

	/* Concatenate the strings */
	while ((*p++ = *strSource++) != 0 && --bufferLeft > 0);

	if ( bufferLeft == 0 )
	{
		*strDest = NULL;
		__safe_strings_error_handler( ERANGE );
		return ERANGE;
	}

	return 0;
}

#endif
