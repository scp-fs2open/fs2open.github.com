#include <windows.h>

#include "trackir.h"
#include "osapi/osapi.h"

// CALLING THESE FUNCTIONS DIRECTLY IS VERY VERY DANGEROUS. 
// Use TrackIR functions sans "_Proc" since they (hopefully) won't cause any crashes if the function pointers are NULL
// - Swifty
TRACKIR_INIT_FUNC TrackIR_Init_Proc = NULL; // Returns 0 if TrackIR has successfully initialized. Non-zero if error.
TRACKIR_QUERY_FUNC TrackIR_Query_Proc = NULL;  // Must be called each frame before GetYaw and GetPitch 
TRACKIR_GETYAW_FUNC TrackIR_GetYaw_Proc = NULL; // Returns a float between -1.0 and 1.0
TRACKIR_GETPITCH_FUNC TrackIR_GetPitch_Proc = NULL; // Returns a float between -1.0 and 1.0
TRACKIR_SHUTDOWN_FUNC TrackIR_ShutDown_Proc = NULL; // Must be called before the application closes

#ifdef WIN32

HMODULE trackIR_dll;

#endif

// Zero if TrackIR is not installed or is disabled. Non-zero otherwise
int trackir_enabled;

void initialize_trackir()
{
#ifdef WIN32
	char dllPath[512];

	GetCurrentDirectory(511, dllPath);
	strcat(dllPath, "\\");
	strcat(dllPath, "FS2_TrackIR.dll"); 

	trackIR_dll = ::LoadLibrary(dllPath);

	if(trackIR_dll != NULL)
	{
		TrackIR_Init_Proc = (TRACKIR_INIT_FUNC)GetProcAddress(trackIR_dll, "FS2_TrackIR_Init");
		TrackIR_Query_Proc = (TRACKIR_QUERY_FUNC)GetProcAddress(trackIR_dll, "FS2_TrackIR_Query");
		TrackIR_GetPitch_Proc = (TRACKIR_GETPITCH_FUNC)GetProcAddress(trackIR_dll, "FS2_TrackIR_GetPitch");
		TrackIR_GetYaw_Proc = (TRACKIR_GETYAW_FUNC)GetProcAddress(trackIR_dll, "FS2_TrackIR_GetYaw");
		TrackIR_ShutDown_Proc = (TRACKIR_SHUTDOWN_FUNC)GetProcAddress(trackIR_dll, "FS2_TrackIR_ShutDown");

		if(TrackIR_Init_Proc == NULL)
		{
			trackir_enabled = 0;
			return;
		}
	}
	else
	{
		trackir_enabled = 0;
		return;
	}

	if(!TrackIR_Init((HWND)os_get_window()))
	{
		trackir_enabled = 1;
		return;
	}
#endif
	trackir_enabled = 0;

}

int TrackIR_Init( HWND hwnd)
{
	if(TrackIR_Init_Proc == NULL)
	{
		return -1; // returning a -1 since the DLL proc will return non-zero values on error and zero upon success.
	}
	else
	{
		return TrackIR_Init_Proc(hwnd);
	}
}
int TrackIR_Query()
{
	if(TrackIR_Query_Proc == NULL) 
	{
		return 1; 
	}
	else
	{
		return TrackIR_Query_Proc();
	}
}
float TrackIR_GetYaw()
{
	if(TrackIR_GetYaw_Proc == NULL)
	{
		return 0;
	}
	else
	{
		return TrackIR_GetYaw_Proc();
	}
}
float TrackIR_GetPitch()
{
	if(TrackIR_GetPitch_Proc == NULL)
	{
		return 0;
	}
	else
	{
		return TrackIR_GetPitch_Proc();
	}
}
int TrackIR_ShutDown()
{
	if(TrackIR_ShutDown_Proc == NULL)
	{
		return 0;
	}
	else
	{
		return TrackIR_ShutDown_Proc();
	}
}