#include <windows.h>

#include "movie.h"
#include "directx/dx8show.h"
#include "osapi/osapi.h"
#include "cmdline/cmdline.h"	
#include "graphics/2d.h"	

bool movie_play(char *name, int unknown_value, void *hwnd)
{
	if(!Cmdline_dshowvid)
	{
		return true;
	}

	/*
	** Check file exists
	*/
	FILE *fp = fopen(name,"rb");
		
	if(fp == NULL)
	{
		return true;
	}

	fclose(fp);

#ifdef _WIN32

	bool result = true;

	gr_activate(0);

	if(hwnd != NULL)
	{
		dx8show_set_hwnd(hwnd);
	}
	else
	{
		dx8show_set_hwnd((void *) os_get_window());
	}

	dx8show_play_cutscene(name);

	gr_activate(1);

	if((HWND) os_get_window() != NULL)
	{
		if(GetForegroundWindow() != (HWND) os_get_window())
		{
 			SetForegroundWindow((HWND) os_get_window());
		}
	}

#endif // _WIN32

	return true;
}

bool movie_play_two(char *name1, char *name2, int unknown_value)
{
	bool result = true;

	movie_play(name1, unknown_value);
	movie_play(name2, unknown_value);

	return result;
}

