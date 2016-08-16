#ifndef EXTERNALCODE_H_INCLUDED_
#define EXTERNALCODE_H_INCLUDED_

#include "globalincs/pstypes.h"

#include <SDL_loadso.h>

/* This class loads external libraries for FSO use.
* Uses SDL to do the actual loading so this should be supported on most platforms
*/
class SCP_ExternalCode
{
public:
	SCP_ExternalCode( )
		: m_library(NULL)
	{
	}

	virtual ~SCP_ExternalCode()
	{
		if (m_library)
			SDL_UnloadObject(m_library);
	}

protected:
	bool LoadExternal( const char* externlib )
	{
		if ( !externlib )
			return FALSE;
		
		m_library = SDL_LoadObject(externlib);

#ifndef NDEBUG
		if (m_library == NULL)
		{
			mprintf(("Failed to load library '%s': '%s'\n", externlib, SDL_GetError()));
		}
#endif

		return m_library != NULL;
	}

	void* LoadFunction(const char* functionname)
	{
		if (m_library != NULL && functionname != NULL)
		{
			void* func = SDL_LoadFunction(m_library, functionname);

#ifndef NDEBUG
			if (func == NULL)
			{
				mprintf(("Failed to load function '%s': '%s'\n", functionname, SDL_GetError()));
			}
#endif

			return func;
		}

		return NULL;
	}

	template<class FuncType>
	FuncType LoadFunction(const char* functionname)
	{
		return reinterpret_cast<FuncType>(LoadFunction(functionname));
	}

private:
	void* m_library;
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

typedef int (SCP_EXT_CALLCONV *SCPDLL_PFVERSION)(SCPDLL_Version*);

/* Must be in a CPP file in your DLL code */
#define SCPDLL_VERSION_FUNCTION( Major, Minor, Patch ) \
	SCP_EXTERN_C int SCPDLL_EXTERNAL SCPDLL_GetVersion( SCPDLL_Version* v ) { \
		if ( !v ) return -1;\
		v->major = Major; v->minor = Minor; v->patch = Patch;\
		return 0; }

#endif /* EXTERNALCODE_H_INCLUDED_ */