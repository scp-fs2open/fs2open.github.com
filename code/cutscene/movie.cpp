#include <windows.h>

#include "movie.h"
#include "directx/dx8show.h"
#include "osapi/osapi.h"
#include "cmdline/cmdline.h"	
#include "graphics/2d.h"
#include "debugconsole/dbugfile.h" 

bool movie_shutdown_fgx = false;

void movie_set_shutdown_fgx(bool state)
{
	movie_shutdown_fgx = state;
}

bool movie_play(char *name, int unknown_value)
{
	// This should not be uncommented
 	if(!Cmdline_dshowvid)
	{
  		return true;
	}

	/*
	** Check file exists
	*/
	FILE *fp = fopen(name,"rb");
	DBUGFILE_OUTPUT_1("About to play: %s",name);
		
	if(fp == NULL)
	{
		DBUGFILE_OUTPUT_0("MOVIE ERROR: Cant open movie file");
		return true;
	}

	fclose(fp);

	if(movie_shutdown_fgx == true)
	{
		gr_activate(0);
	}

#ifdef _WIN32
	 os_app_activate_set(false);

	dx8show_set_hwnd((void *) os_get_window());

 //		ShowWindow((HWND) os_get_window(),SW_HIDE);
 // 	ShowWindow((HWND) os_get_window(),SW_MINIMIZE);

  	dx8show_play_cutscene(name);

 //		ShowWindow((HWND) os_get_window(),SW_RESTORE);
 //		ShowWindow((HWND) os_get_window(),SW_SHOW);

	os_app_activate_set(true);

	/*
	// Comment this back in if the freespace window isnt coming back after the movie ends

	if((HWND) os_get_window() != NULL)
	{
		if(GetForegroundWindow() != (HWND) os_get_window())
		{
			DBUGFILE_OUTPUT_0("Using SetForegroundWindow last");
 			SetForegroundWindow((HWND) os_get_window());
		}
	}
	*/
 
#endif // _WIN32

	if(movie_shutdown_fgx == true)
	{
		gr_activate(1);
	}

	return true;
}

bool movie_play_two(char *name1, char *name2, int unknown_value)
{
	bool result = true;

	// This should not be uncommented
 	if(!Cmdline_dshowvid)
	{
  		return true;
	}

	/*
	** Check file exists
	*/
	FILE *fp = fopen(name1,"rb");
	DBUGFILE_OUTPUT_1("About to play: %s",name1);
		
	if(fp == NULL)
	{
		DBUGFILE_OUTPUT_0("MOVIE ERROR: Cant open movie file");
		
		/*
		** Check second file exists
		*/
		FILE *fp2 = fopen(name2,"rb");
		DBUGFILE_OUTPUT_1("About to play: %s",name2);
		
		if(fp2 == NULL)
		{
			DBUGFILE_OUTPUT_0("MOVIE ERROR: Cant open movie file");
			return true;
		}

		fclose(fp2);
	}

	fclose(fp);

	if(movie_shutdown_fgx == true)
	{
		gr_activate(0);
	}

#ifdef _WIN32
	 os_app_activate_set(false);
		 
  	dx8show_play_cutscene(name1);

	{
		MSG msg;
   		while(PeekMessage(&msg, (HWND) os_get_window(), 0, 0, PM_REMOVE))
   		{
   			TranslateMessage(&msg);
   			DispatchMessage(&msg);
   		}
	}

	dx8show_play_cutscene(name2);

	os_app_activate_set(true);
#endif

	if(movie_shutdown_fgx == true)
	{
		gr_activate(1);
	}

	return result;
}

