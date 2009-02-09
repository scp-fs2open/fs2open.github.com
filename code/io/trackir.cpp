#include "globalincs/pstypes.h"
#include "io/trackir.h"
#include "osapi/osapi.h"

#ifdef WIN32
#include <windows.h>
#endif

// Zero if TrackIR is not installed or is disabled. Non-zero otherwise
int trackir_enabled;

void initialize_trackir()
{
#ifdef WIN32
	if(!TrackIR_Init((HWND)os_get_window()))
	{
		trackir_enabled = 1;
		return;
	}
#endif
	trackir_enabled = 0;
}

#ifndef WIN32

int TrackIR_Query()
{
	return 1;
}
float TrackIR_GetYaw()
{
	return 0.0f;
}
float TrackIR_GetYawRaw()
{
	return 0.0f;
}
float TrackIR_GetPitch()
{
	return 0.0f;
}
float TrackIR_GetPitchRaw()
{
	return 0.0f;
}
float TrackIR_GetRoll()
{
	return 0.0f;
}
float TrackIR_GetRollRaw()
{
	return 0.0f;
}
float TrackIR_GetX()
{
	return 0.0f;
}
float TrackIR_GetXRaw()
{
	return 0.0f;
}
float TrackIR_GetY()
{
	return 0.0f;
}
float TrackIR_GetYRaw()
{
	return 0.0f;
}
float TrackIR_GetZ()
{
	return 0.0f;
}
float TrackIR_GetZRaw()
{
	return 0.0f;
}
int TrackIR_ShutDown()
{
	return 1;
}
#endif
