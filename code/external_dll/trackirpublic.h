#ifndef TRACKIRPUBLIC_H_INCLUDED_
#define TRACKIRPUBLIC_H_INCLUDED_

#include "external_dll/externalcode.h"
#include "globalincs/pstypes.h"

#define TRACKIRBRIDGEDLLNAME "scptrackir.dll"

#define SCP_INITRESULT_SUCCESS 0
#define SCP_INITRESULT_BADKEY 1
#define SCP_INITRESULT_BADPATH 2
#define SCP_INITRESULT_PATHTOOLONG 3
#define SCP_INITRESULT_NODLL 4
#define SCP_INITRESULT_BADSIGNATURE 5
#define SCP_INITRESULT_UNABLETOREGISTER 6
#define SCP_INITRESULT_BADDATAFIELDS 7
#define SCP_INITRESULT_BADREGISTRATION 8
#define SCP_INITRESULT_BADTRANSMISSION 9

/* Exported functions */
/* HWND comes from externalcode.h */
/* These are function pointer prototypes provided for library loading */

/* Only one function doesn't return an int or a float */
typedef int ( SCP_EXT_CALLCONV *SCPTRACKIR_PFINIT )( HWND );

/* All other trackir functions return an int or a float
 *  and take no parameters
 */
typedef int ( SCP_EXT_CALLCONV *SCPTRACKIR_PFINTVOID )( );
typedef float ( SCP_EXT_CALLCONV *SCPTRACKIR_PFFLOATVOID )( );

class TrackIRDLL : public SCP_ExternalCode
{
public:
	TrackIRDLL( )
	{
		/* Load the DLL and functions 
		 * If this is done globally, we'll be found by MSPDBDEBUGGING :)
		 */

		Reset( );

		if ( !LoadExternal( TRACKIRBRIDGEDLLNAME ) )
			return;

		m_Init = (SCPTRACKIR_PFINIT)LoadFunction( "SCPTIR_Init" );
		m_Close = (SCPTRACKIR_PFINTVOID)LoadFunction( "SCPTIR_Close" );
		m_Query = (SCPTRACKIR_PFINTVOID)LoadFunction( "SCPTIR_Query" );

		m_GetX = (SCPTRACKIR_PFFLOATVOID)LoadFunction( "SCPTIR_GetX" );
		m_GetY = (SCPTRACKIR_PFFLOATVOID)LoadFunction( "SCPTIR_GetY" );
		m_GetZ = (SCPTRACKIR_PFFLOATVOID)LoadFunction( "SCPTIR_GetZ" );
		m_GetRoll = (SCPTRACKIR_PFFLOATVOID)LoadFunction( "SCPTIR_GetRoll" );
		m_GetPitch = (SCPTRACKIR_PFFLOATVOID)LoadFunction( "SCPTIR_GetPitch" );
		m_GetYaw = (SCPTRACKIR_PFFLOATVOID)LoadFunction( "SCPTIR_GetYaw" );
		m_enabled = true;
	}

	bool Enabled( )
	{
		return m_enabled;
	}

	virtual ~TrackIRDLL( )
	{
		/* Prevent further use of the DLL */
		Reset( );
	}

	void Reset( )
	{
		m_Init = NULL;
		m_Close = NULL; 
		m_Query = NULL;
		m_GetX = NULL;
		m_GetY = NULL;
		m_GetZ = NULL;
		m_GetPitch = NULL;
		m_GetRoll = NULL;
		m_GetYaw = NULL;
		m_enabled = false;
	}

	/* Returns 0 on success */
	int Init( HWND hwnd )
	{
		if ( m_Init )
			return m_Init( hwnd );
		return 0;
	}

	int Close( )
	{
		if ( m_Close )
			return m_Close( );
		return 0;
	}

	int Query( )
	{
		if ( m_Query )
			return m_Query( );
		return 0;
	}


	float GetX( )
	{
		if ( m_GetX )
			return m_GetX( );
		return 0.0f;
	}

	float GetY( )
	{
		if ( m_GetY )
			return m_GetY( );
		return 0.0f;
	}

	float GetZ( )
	{
		if ( m_GetZ )
			return m_GetZ( );
		return 0.0f;
	}

	float GetPitch( )
	{
		if ( m_GetPitch )
			return m_GetPitch( );
		return 0.0f;
	}

	float GetRoll( )
	{
		if ( m_GetRoll )
			return m_GetRoll( );
		return 0.0f;
	}

	float GetYaw( )
	{
		if ( m_GetYaw )
			return m_GetYaw( );
		return 0.0f;
	}
private:
	/* Functions */
	SCPTRACKIR_PFINIT m_Init;
	SCPTRACKIR_PFINTVOID m_Close;
	SCPTRACKIR_PFINTVOID m_Query;

	SCPTRACKIR_PFFLOATVOID m_GetX;
	SCPTRACKIR_PFFLOATVOID m_GetY;
	SCPTRACKIR_PFFLOATVOID m_GetZ;
	SCPTRACKIR_PFFLOATVOID m_GetPitch;
	SCPTRACKIR_PFFLOATVOID m_GetRoll;
	SCPTRACKIR_PFFLOATVOID m_GetYaw;

	bool m_enabled;
};

/* Added to trackirpublic.h */
#ifndef SCPDLL_EXTERNAL_LIB
extern TrackIRDLL gTirDll_TrackIR;
#endif

#endif /* TRACKIRPUBLIC_H_INCLUDED_ */
