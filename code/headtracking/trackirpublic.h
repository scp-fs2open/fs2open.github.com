#ifndef TRACKIRPUBLIC_H_INCLUDED_
#define TRACKIRPUBLIC_H_INCLUDED_

#include "external_dll/externalcode.h"
#include "globalincs/pstypes.h"
#include "osapi/osapi.h"

#include <SDL_syswm.h>

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
	TrackIRDLL();

	virtual ~TrackIRDLL();

	bool Enabled() const;

	void Reset();

	/* Returns 0 on success */
	int Init(SDL_Window* window);

	int Close();

	int Query();

	float GetX() const;

	float GetY() const;

	float GetZ() const;

	float GetPitch() const;

	float GetRoll() const;

	float GetYaw() const;
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

#endif /* TRACKIRPUBLIC_H_INCLUDED_ */
