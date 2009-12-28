#ifndef _TRACKIR_H
#define _TRACKIR_H

#include "globalincs/pstypes.h"
#include "osapi/osapi.h"

#if defined( WIN32 ) && defined( TRACKIR_BUILD )
 #include <windows.h>

 #define TIR_APIENTRY __stdcall
#else
 #define TIR_APIENTRY
#endif

void initialize_trackir();

extern int trackir_enabled;

#if defined(__cplusplus)
extern "C" {
#endif

#if defined( WIN32 ) && defined( TRACKIR_BUILD )
	// HWND doesn't exist outside of Windows
 int TIR_APIENTRY TrackIR_Init(HWND FS2_Window);
#endif

int TIR_APIENTRY TrackIR_Query();
float TIR_APIENTRY TrackIR_GetPitch();
float TIR_APIENTRY TrackIR_GetPitchRaw();
float TIR_APIENTRY TrackIR_GetYaw();
float TIR_APIENTRY TrackIR_GetYawRaw();
float TIR_APIENTRY TrackIR_GetRoll();
float TIR_APIENTRY TrackIR_GetRollRaw();
float TIR_APIENTRY TrackIR_GetX();
float TIR_APIENTRY TrackIR_GetXRaw();
float TIR_APIENTRY TrackIR_GetY();
float TIR_APIENTRY TrackIR_GetYRaw();
float TIR_APIENTRY TrackIR_GetZ();
float TIR_APIENTRY TrackIR_GetZRaw();
int TIR_APIENTRY TrackIR_ShutDown();

#if defined(__cplusplus)
}  
#endif /* extern "C" */

#endif /* _TRACKIR_H */
