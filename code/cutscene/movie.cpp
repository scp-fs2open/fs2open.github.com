#include <windows.h>

#include "movie.h"
#include "directx/dx8show.h"
#include "osapi/osapi.h"
#include "cmdline/cmdline.h"	
#include "graphics/2d.h"
#include "debugconsole/dbugfile.h" 
#include "graphics/gropengl.h"

// This module links freespace movie calls to the actual API calls the play the movie.
// This module handles all the different requires of OS and gfx API and finding the file to play
#ifdef _WIN32

void process_messages()
{
	MSG msg;
   	while(PeekMessage(&msg, (HWND) os_get_window(), 0, 0, PM_REMOVE))
   	{
   		TranslateMessage(&msg);
   		DispatchMessage(&msg);
   	}
}

#include "graphics/grd3dinternal.h"

// Play one movie
bool movie_play(char *name)
{
 	if(Cmdline_dnoshowvid) return false;

	char full_name[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, full_name);
	strcat(full_name, "\\");
	strcat(full_name, name);

	// Check file exists, assume its not in a vp file
	FILE *fp = fopen(full_name,"rb");
	DBUGFILE_OUTPUT_1("About to play: %s",full_name);
		
	if(fp == NULL)
	{
		DBUGFILE_OUTPUT_1("MOVIE ERROR: Cant open movie file %s",full_name);
		return false;
	}

	fclose(fp);

 	if(gr_screen.mode == GR_DIRECT3D)
   		GlobalD3DVars::D3D_activate = 0;

	process_messages();

	OpenClip((HWND) os_get_window(), full_name);

	do
	{
		// Give system threads time to run (and don't sample user input madly)
		Sleep(100);

		MSG msg;
		while(PeekMessage(&msg, (HWND) os_get_window(), 0, 0, PM_REMOVE))
		{
 			TranslateMessage(&msg);


			if(msg.message == WM_ERASEBKGND)
			{
				continue;
			}

			PassMsgToVideoWindow((HWND) os_get_window(), msg.message, msg.wParam, msg.lParam);

			if(msg.message == WM_PAINT)
			{
			}

			if(msg.message != WM_KEYDOWN)
			{
			  //	DefWindowProc((HWND) os_get_window(), msg.message, msg.wParam, msg.lParam);
		   	  	DispatchMessage(&msg);
				continue;
			}
		
			switch(msg.wParam )
			{
				// Quits movie if escape, space or return is pressed
				case VK_ESCAPE:
				case VK_SPACE:
				case VK_RETURN:
				{
					// Terminate movie playback early
					CloseClip((HWND) os_get_window());
			   //		LockWindowUpdate( NULL);
			   		GlobalD3DVars::D3D_activate = 1;
					gr_set_clear_color(0,0,0);
					process_messages();
					return true;
				}
			}
 		}
	}
	// Stream bit of the movie then handle windows messages
	while(dx8show_stream_movie() == false);

	// We finished playing the movie
	CloseClip((HWND) os_get_window());
 	GlobalD3DVars::D3D_activate = 1;

	gr_flip();
	gr_set_clear_color(0,0,0);
	return true;
}

bool movie_play_two(char *name1, char *name2)
{
	bool result1 = movie_play(name1);
	process_messages();
	bool result2 = movie_play(name2);

	return result1 && result2;
}

#else

// Stubs, preprocessor calls were making the win32 code more complex for no good reason.
// Using stubs instead.
bool movie_play(char *name) {return true;}
bool movie_play_two(char *name1, char *name2) {return true;}

#endif