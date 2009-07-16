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

#ifdef SAFESTRINGS_TEST_APP

#include <stdio.h>
#include <errno.h>
#include "safe_strings.h"

int last_errno = 0;
void error_handler( int errnoValue, const char* errnoStr, const char* file, const char* function, int line )
{
	last_errno = errnoValue;
	(errnoStr);
	(file);
	(function);
	(line);
}

#define _RESET_ERRNO( ) last_errno = 0

#define _EXPECTED_ERRNO( errnoVal ) if ( last_errno != errnoVal ) printf("errno value of %d not found: %d %s(%d)\n",errnoVal,last_errno,__FILE__,__LINE__)
#define _EXPECTED_LAST_ERRNO( errnoVal ) _EXPECTED_ERRNO( errnoVal )
#define _EXPECTED_RETURN( value, function ) if ( function != value ) printf("expected value not received: %s(%d)\n", __FILE__,__LINE__)

#define _EXPECTED_STRING( str, value ) if ( !strcmp( str, value ) ) printf("strings do not match: %s(%d)\n", __FILE__, __LINE__)
#define _EXPECTED_VALUE( val, value ) if ( val != value ) printf("values do not match: %s(%d)\n", __FILE__, __LINE__ )

/* Dumb memset (because we can't include string.h in this program) */
void dumb_memset( void* buf, char val, size_t bytes )
{
	char* p = (char*)buf;

	while ( bytes-- )
	{
		*(p++) = val;
	}
}

bool strcmp( const char* str1, const char* str2 )
{
	while ( *str1 && *str2 )
	{
		if ( *str1 != *str2 )
			return false;
		str1++;
		str2++;
	}

	return ( !*str1 && !*str2 );
}

void test_strcpy_s( )
{
#define _RESET_STRINGS( ) dumb_memset( strSource, 0, 15 );\
						  dumb_memset( strDest, 0, 15 );

	char strSource[ 15 ];
	char strDest[ 15 ];

	/* strcpy_s tests
	 * Must test both strcpy_s functions (even though one calls the other)
	 * 1) Copy string into larger buffer
	 *    - Expecting buffer to contain string + NULL
	 * 2) Attempt to copy string into too small buffer
	 *    - strDest[0] = NULL
	 *    - calls __safe_strings_error_handler with ERANGE
	 *    - returns ERANGE
	 * 3) strSource = NULL
	 *    - returns EINVAL
	 *    - calls __safe_strings_error_handler with EINVAL
	 *    - strDest[0] = NULL
	 * 4) strDest = NULL
	 *    - calls __safe_strings_error_handler with EINVAL
	 *    - returns EINVAL
	 * 5) strSource = NULL, strDest = NULL
	 *    - calls __safe_strings_error_handler with EINVAL
	 *	  - returns EINVAL
	 * 6) sizeInBytes = 0
	 *	  - calls __safe_strings_error_handler with ERANGE
	 *    - returns ERANGE
	 * 7) A string with the size+NULL the same as the size of the buffer
	 *    - returns 0
	 *    - strDest should contain the string
	 */

	_RESET_ERRNO( );

	/* 1 */
	_RESET_STRINGS( );
	_EXPECTED_RETURN( 0, strcpy_s( strDest, "Hello World" ) );
	_EXPECTED_LAST_ERRNO( 0 );
	_EXPECTED_STRING( strDest, "Hello World" );

	_RESET_STRINGS( );
	_EXPECTED_RETURN( 0, strcpy_s( strDest, 15, "Hello World" ) );
	_EXPECTED_LAST_ERRNO( 0 );
	_EXPECTED_STRING( strDest, "Hello World" );


	/* 2 */
	_RESET_STRINGS( );
	_EXPECTED_RETURN( ERANGE, strcpy_s( strDest, "Hello World, this is a test" ) );
	_EXPECTED_ERRNO( ERANGE );
	_EXPECTED_VALUE( strDest[ 0 ], NULL );

	_RESET_STRINGS( );
	_EXPECTED_RETURN( ERANGE, strcpy_s( strDest, 15, "Hello World, this is a test" ) );
	_EXPECTED_ERRNO( ERANGE );
	_EXPECTED_VALUE( strDest[ 0 ], NULL );

	/* 3 */
	_RESET_STRINGS( );
	_EXPECTED_RETURN( EINVAL, strcpy_s( strDest, NULL ) );
	_EXPECTED_ERRNO( EINVAL );

	_RESET_STRINGS( );
	_EXPECTED_RETURN( EINVAL, strcpy_s( strDest, 15, NULL ) );
	_EXPECTED_ERRNO( EINVAL );

	/* 4 - can't be done on template version */
	_RESET_STRINGS( );
	_EXPECTED_RETURN( EINVAL, strcpy_s( NULL, 15, "Hello World" ) );
	_EXPECTED_ERRNO( EINVAL );

	/* 5 - can't be done on template version */
	_RESET_STRINGS( );
	_EXPECTED_RETURN( EINVAL, strcpy_s( NULL, 15, NULL ) );
	_EXPECTED_ERRNO( EINVAL );

	/* 6 - can't be done on strcpy_s template version */
	_RESET_STRINGS( );
	_EXPECTED_RETURN( ERANGE, strcpy_s( strDest, 0, "Hello World") );
	_EXPECTED_ERRNO( ERANGE );

	/* 7 */
	_RESET_STRINGS( );
	_EXPECTED_RETURN( 0, strcpy_s( strDest, "Hello World th" ) );
	_EXPECTED_LAST_ERRNO( ERANGE );

#undef _RESET_STRINGS
}

void test_strcat_s( )
{
#define _RESET_STRINGS( ) dumb_memset( strSource, 0, 15 );\
						  dumb_memset( strDest, 0, 15 );

	char strSource[ 15 ];
	char strDest[ 15 ];

	/* Test cases for strcat_s
	 * 1) Normal concatenation where total string size < buffersize -1
	 *    - No change in errno (i.e. handler is not called)
	 *    - returns 0
	 * 2) strSource = NULL, sizeInBytes != 0, strDest != NULL
	 *    - errno now EINVAL
	 *    - strDest[ 0 ] == NULL
	 *    - returns EINVAL
	 * 3) strSource != NULL, sizeInBytes != 0, strDest == NULL
	 *    - errno now EINVAL
	 *    - returns EINVAL
	 * 4) sizeInBytes = 0
	 *    - errno now ERANGE
	 *    - returns ERANGE
	 * 5) final string that is 15 chars+null
	 *    - returns 0
	 *    - no change in errno
	 */

	_RESET_ERRNO( );

	/* 1 */
	_RESET_STRINGS( );
	_EXPECTED_RETURN( 0, strcpy_s( strDest, "Hello " ) );
	_EXPECTED_LAST_ERRNO( 0 );
	_EXPECTED_RETURN( 0, strcat_s( strDest, "World" ) );
	_EXPECTED_LAST_ERRNO( 0 );

	_RESET_STRINGS( );
	_EXPECTED_RETURN( 0, strcpy_s( strDest, 15, "Hello " ) );
	_EXPECTED_LAST_ERRNO( 0 );
	_EXPECTED_RETURN( 0, strcat_s( strDest, 15, "World" ) );
	_EXPECTED_LAST_ERRNO( 0 );
	
	/* 2 */
	_RESET_STRINGS( );
	_EXPECTED_RETURN( 0, strcpy_s( strDest, "World" ) );
	_EXPECTED_LAST_ERRNO(  0);
	_EXPECTED_RETURN( EINVAL, strcat_s( strDest, NULL ) );
	_EXPECTED_ERRNO( EINVAL );
	_EXPECTED_VALUE( strDest[ 0 ], NULL );

	_RESET_STRINGS( );
	_EXPECTED_RETURN( 0, strcpy_s( strDest, "World" ) );
	_EXPECTED_LAST_ERRNO( EINVAL );
	_EXPECTED_RETURN( EINVAL, strcat_s( strDest, 15, NULL ) );
	_EXPECTED_ERRNO( EINVAL );
	_EXPECTED_VALUE( strDest[ 0 ], NULL );

	/* 3 - can't be done with templated version */
	_RESET_STRINGS( );
	_EXPECTED_RETURN( 0, strcpy_s( strSource, "Hello " ) );
	_EXPECTED_LAST_ERRNO( EINVAL );
	_EXPECTED_RETURN( EINVAL, strcat_s( NULL, 15, strSource ) );
	_EXPECTED_ERRNO( EINVAL );

	/* 4 - can't be done with templated version*/
	_RESET_STRINGS( );
	_EXPECTED_RETURN( 0, strcpy_s( strDest, 15, "Hello " ) );
	_EXPECTED_LAST_ERRNO( EINVAL );
	_EXPECTED_RETURN( ERANGE, strcat_s( strDest, 0, "World" ) );
	_EXPECTED_ERRNO( ERANGE );	

	/* 5 */
	_RESET_STRINGS( );
	_EXPECTED_RETURN( 0, strcpy_s( strDest, "Hello " ) );
	_EXPECTED_LAST_ERRNO( ERANGE );
	_EXPECTED_RETURN( 0, strcpy_s( strDest, "World th" ) );
	_EXPECTED_LAST_ERRNO( ERANGE );

	_RESET_STRINGS( );
	_EXPECTED_RETURN( 0, strcpy_s( strDest, 15, "Hello " ) );
	_EXPECTED_LAST_ERRNO( ERANGE );
	_EXPECTED_RETURN( 0, strcpy_s( strDest, 15, "World th" ) );
	_EXPECTED_LAST_ERRNO( ERANGE );

#undef _RESET_STRINGS
}

int main(int argc, char* argv[])
{
	(argc);
	(argv);

	test_strcpy_s( );
	test_strcat_s( );

	printf("done.\n");

	return 0;
}

#endif