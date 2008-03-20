
#ifndef _TRACKIR_H
#define _TRACKIR_H

#include "globalincs/pstypes.h"
#include "osapi/osapi.h"

#ifdef WIN32
#include <windows.h>
#endif

// Set up function pointers
typedef int (*TRACKIR_INIT_FUNC)( HWND );
typedef int (*TRACKIR_QUERY_FUNC)();
typedef float (*TRACKIR_GETYAW_FUNC)();
typedef float (*TRACKIR_GETPITCH_FUNC)();
typedef float (*TRACKIR_GETX_FUNC)();
typedef float (*TRACKIR_GETY_FUNC)();
typedef float (*TRACKIR_GETZ_FUNC)();
typedef int (*TRACKIR_SHUTDOWN_FUNC)();

extern TRACKIR_INIT_FUNC TrackIR_Init_Proc; // Returns 0 if TrackIR has successfully initialized. Non-zero if error.
extern TRACKIR_QUERY_FUNC TrackIR_Query_Proc; 
extern TRACKIR_GETYAW_FUNC TrackIR_GetYaw_Proc;
extern TRACKIR_GETPITCH_FUNC TrackIR_GetPitch_Proc;
extern TRACKIR_GETX_FUNC TrackIR_GetX_Proc;
extern TRACKIR_GETY_FUNC TrackIR_GetY_Proc;
extern TRACKIR_GETZ_FUNC TrackIR_GetZ_Proc;
extern TRACKIR_SHUTDOWN_FUNC TrackIR_ShutDown_Proc;

// 0 if TrackIR is not installed or is disabled. 
extern int trackir_enabled;

#ifdef WIN32

extern HMODULE trackIR_dll;

#endif

void initialize_trackir(); // The function that loads the FS2_TrackIR DLL and appropriates the virtual functions accordingly.

// These are safe to call even if the DLL isn't loaded or if we're not running on Windows.
int TrackIR_Init( HWND hwnd); 
int TrackIR_Query(); 
float TrackIR_GetYaw();
float TrackIR_GetPitch();
float TrackIR_GetX();
float TrackIR_GetY();
float TrackIR_GetZ();
int TrackIR_ShutDown();

#endif
