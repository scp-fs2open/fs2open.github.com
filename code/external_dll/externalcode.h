#ifndef EXTERNALCODE_H_INCLUDED_
#define EXTERNALCODE_H_INCLUDED_

#include "globalincs/pstypes.h"

/* We must have windows.h anywhere SCP_ExternalCode is used under windows */
#ifdef _WIN32
#include <windows.h>
#endif

/* This class loads external libraries for FSO use.
 * Different platforms have different ways of doing this, so use your ifdefs!
 */
class SCP_ExternalCode
{
public:
	SCP_ExternalCode( )
#ifdef _WIN32
		: m_dll( NULL )
#endif
	{
	}

	virtual ~SCP_ExternalCode( )
	{
#ifdef _WIN32
		if ( m_dll )
			::FreeLibrary( m_dll );
#endif
	}

protected:
	BOOL LoadExternal( const char* externlib )
	{
		if ( !externlib )
			return FALSE;
		
#ifdef _WIN32
		m_dll = ::LoadLibrary( externlib );
		
		if ( m_dll )
			return TRUE;
#endif

		return FALSE;
	}

	void* LoadFunction( const char* functionname )
	{
#ifdef _WIN32
		if ( m_dll != NULL && functionname != NULL )
			return ::GetProcAddress( m_dll, functionname );
#endif
		return NULL;
	}
private:
#ifdef _WIN32
	HMODULE m_dll;
#endif
};

/* These are available if you're compiling an external DLL
 * So far we have trackir, and speech is on the way
 */
#ifdef _WIN32
#	define SCP_EXT_CALLCONV __cdecl
#	ifdef SCPDLL_EXTERNAL_LIB
#		define SCPDLL_EXTERNAL __declspec( dllexport )
#	else
#		define SCPDLL_EXTERNAL __declspec( dllimport )
#	endif
/* Must be in a CPP file in your DLL code
 * If you want to write your own, you shouldn't.
 */
#	define SCPDLL_DLLMAIN( ) \
		SCP_EXTERN_C BOOL APIENTRY DllMain( HANDLE, DWORD, LPVOID ) { return TRUE; }
#else
#	define SCP_EXT_CALLCONV
#	define SCPDLL_EXTERNAL
#	define SCPDLL_DLLMAIN( )
#endif

#ifdef __cplusplus
#	define SCP_EXTERN_C extern "C"
#else
#	define SCP_EXTERN_C
#endif

/* Version information */
typedef struct _SCPDLL_Version
{
	int major;
	int minor;
	int patch;
} SCPDLL_Version;

typedef int ( SCP_EXT_CALLCONV *SCPDLL_PFVERSION )( SCPDLL_Version* );

/* Must be in a CPP file in your DLL code */
#define SCPDLL_VERSION_FUNCTION( Major, Minor, Patch ) \
	SCP_EXTERN_C int SCPDLL_EXTERNAL SCPDLL_GetVersion( SCPDLL_Version* v ) { \
		if ( !v ) return -1;\
		v->major = Major; v->minor = Minor; v->patch = Patch;\
		return 0; }

#endif /* EXTERNALCODE_H_INCLUDED_ */
