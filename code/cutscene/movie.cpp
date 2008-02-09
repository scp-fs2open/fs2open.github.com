/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/cutscene/movie.cpp $
 * $Revision: 2.23 $
 * $Date: 2004-11-27 10:42:18 $
 * $Author: taylor $
 *
 * movie player code
 * 
 * $Log: not supported by cvs2svn $
 * Revision 2.22  2004/07/26 20:47:26  Kazan
 * remove MCD complete
 *
 * Revision 2.21  2004/07/12 16:32:43  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.20  2004/04/26 13:09:20  taylor
 * cvs log header, mark cutscenes viewable for later, gr_flip() to fix OGL white flash
 *
 * 
 * 
 * $NoKeywords: $
 */

#include <windows.h>

#include "movie.h"
#include "directx/dx8show.h"
#include "osapi/osapi.h"
#include "cmdline/cmdline.h"	
#include "debugconsole/dbugfile.h" 
#include "cfile/cfile.h"
#include "cutscene/cutscenes.h" // cutscene_mark_viewable()
#include "freespace2/freespace.h" // for Game_mode, etc.



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

// filename		- file to search for
// out_name		- output, full path to file
// returns non-zero if file is found
int movie_find(char *filename, char *out_name)
{
	char full_path[MAX_PATH];
	char tmp_name[MAX_PATH];
	char search_name[MAX_PATH];
	int i, size, offset = 0;
	const int NUM_EXT = 2;
	const char *movie_ext[NUM_EXT] = { ".avi", ".mpg" };

	if (out_name == NULL) {
		return 0;
	}

	// remove extension
	strcpy( tmp_name, filename );
	char *p = strchr( tmp_name, '.' );
	if ( p ) {
		*p = 0;
	}
	
	for (i=0; i<NUM_EXT; i++) {
		strcpy( search_name, tmp_name );
		strcat( search_name, movie_ext[i] );

		// try and find the file
    	if ( cf_find_file_location(search_name, CF_TYPE_ANY, full_path, &size, &offset, 0) ) {
			// if it's not in a packfile then we're done
			if (offset == 0) {
				strcpy( out_name, full_path );
				return 1;
			}
    	}
		
		// clear old string
		strcpy( search_name, "" );
	}

	return 0;
}

// Play one movie
bool movie_play(char *name)
{
	// mark the movie as viewable to the player when in a campaign
	// do this before anything else so that we're sure the movie is available
	// to the player even if it's not going to play right now
	if (Game_mode & GM_CAMPAIGN_MODE) {
		cutscene_mark_viewable(name);
	}

	extern int Is_standalone;
	if(Is_standalone) return false;
 	if(Cmdline_dnoshowvid) return false;
 	if(Cmdline_window) return false;

	char full_name[MAX_PATH];
	int rc = 0;

	rc = movie_find(name, full_name);

	if (!rc) {
		DBUGFILE_OUTPUT_1("MOVIE ERROR: Cant open movie file %s", name);
		return false;
	} else {
		DBUGFILE_OUTPUT_1("About to play: %s", full_name);
	}

	process_messages();

	// This is a bit of a hack but it works nicely
 	if(gr_screen.mode == GR_DIRECT3D)
	{
   		GlobalD3DVars::D3D_activate = 0;
		GlobalD3DVars::lpD3DDevice->EndScene();
	  	GlobalD3DVars::lpD3DDevice->Present(NULL,NULL,NULL,NULL);
	//	d3d_lost_device();
	}

	// reset the gr_* stuff before trying to play a movie
	gr_flip(); // in the case of D3D this should recover the device
	gr_clear();

	// This clears the screen
 	InvalidateRect((HWND) os_get_window(), NULL, TRUE);
	PAINTSTRUCT paint_info;
	HDC clear_hdc = BeginPaint((HWND) os_get_window(),&paint_info);

	if(clear_hdc)
	{

		POINT points[4] = {
			{0,0}, 
			{gr_screen.max_w,0}, 
			{gr_screen.max_w, gr_screen.max_h}, 
			{0, gr_screen.max_h}};
		
		HBRUSH hBrush    = CreateSolidBrush(0x00000000);
		HBRUSH hOldBrush = (HBRUSH)SelectObject(clear_hdc, hBrush);
		SelectObject(clear_hdc, hBrush);
		
		Polygon(clear_hdc, points, 4);
		
		SelectObject(clear_hdc, hOldBrush);
		DeleteObject(hBrush);
		
		EndPaint((HWND) os_get_window(),&paint_info);
	}

  	process_messages();

	if(OpenClip((HWND) os_get_window(), full_name))
	{

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
			   		GlobalD3DVars::D3D_activate = 1;
					return true;
				}
			}
 		}
	}
	// Stream bit of the movie then handle windows messages
	while(dx8show_stream_movie() == false);

	}

	// We finished playing the movie
	CloseClip((HWND) os_get_window());

	if (gr_screen.mode == GR_DIRECT3D) {
	 	GlobalD3DVars::D3D_activate = 1;
	}

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
