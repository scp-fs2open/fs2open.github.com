#include "pstypes.h"

/* It is a condition of use that safe_strings.cpp, safe_strings.h, safe_strings_test.cpp remain together.
 *
 * Maintained by portej05 - contact via PM on www.hard-light.net/forums
 * Why have we got this, what is it for?
 * VC2005+ define some safe string functions which check buffer sizes before doing anything
 * Unfortunately, GCC (on Linux and Mac OS X) does not yet provide these functions, therefore, we must!
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



#if !defined(NO_SAFE_STRINGS) && ( !defined( _MSC_VER ) || ( defined( _MSC_VER ) && _MSC_VER >= 1400 /* && !defined(NDEBUG) */ ))

/* An implementation of strcpy_s 
 * We're not going to actually fully behave like the MS debug version.
 */
errno_t scp_strcpy_s( const char* file, int line, char* strDest, size_t sizeInBytes, const char* strSource )
{
	char* pDest;
	const char* pSource;
	size_t bufferLeft = sizeInBytes;
	
	if ( !strDest || !strSource )
	{
		if ( strDest )
			*strDest = '\0';
		__safe_strings_error_handler( EINVAL );
		return EINVAL;
	}

	if ( sizeInBytes == 0 )
	{
		*strDest = '\0';
		__safe_strings_error_handler( ERANGE );
		return ERANGE;
	}

	pDest = strDest;
	pSource = strSource;
	
	while ((*pDest++ = *pSource++) != 0 && --bufferLeft > 0);

	if ( bufferLeft == 0 )
	{
		*strDest = '\0';
		__safe_strings_error_handler( ERANGE );
		return ERANGE;
	}

	return 0;
}

errno_t scp_strcat_s( const char* file, int line, char* strDest, size_t sizeInBytes, const char* strSource )
{
	char* pDest;
	const char* pSource;
	size_t bufferLeft = sizeInBytes;

	if ( !strDest || !strSource )
	{
		if ( strDest )
			*strDest = '\0';
		__safe_strings_error_handler( EINVAL );
		return EINVAL;
	}

	if ( bufferLeft == 0 )
	{
		*strDest = '\0';
		__safe_strings_error_handler( ERANGE );
		return ERANGE;
	}

	/* Find the terminating NULL of the input string */
	pDest = strDest;
	pSource = strSource;
	while ( *pDest )
	{
		pDest++;
		bufferLeft--;
	}

	if ( bufferLeft == 0 )
	{
		*strDest = '\0';
		__safe_strings_error_handler( ERANGE );
		return ERANGE;
	}

	/* Concatenate the strings */
	while ((*pDest++ = *pSource++) != 0 && --bufferLeft > 0);

	if ( bufferLeft == 0 )
	{
		*strDest = '\0';
		__safe_strings_error_handler( ERANGE );
		return ERANGE;
	}

	return 0;
}

#endif
