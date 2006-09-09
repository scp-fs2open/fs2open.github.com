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
 * $Revision: 2.36 $
 * $Date: 2006-09-09 21:28:19 $
 * $Author: taylor $
 *
 * movie player code
 * 
 * $Log: not supported by cvs2svn $
 * Revision 2.35  2006/08/20 00:44:36  taylor
 * add decoder for 8-bit MVEs
 * a basic fix for finding AVIs over MVEs, for mod dir stuff (this needs some CFILE support added to be a true fix, it's on the TODO list)
 * little bits of cleanup for old/unused code
 * make sure MVE filenames are correct in mvelib
 *
 * Revision 2.34  2006/08/15 00:26:07  Backslash
 * add .ogg to the list of recognized movie file extensions
 *
 * Revision 2.33  2006/07/13 22:15:02  taylor
 * handle non-MVE movies a bit better in OpenGL (don't get freaky with the window, don't lose input, etc.)
 * some cleanup to OpenGL window handling, to fix min/max/full issues, and try to make shutdown a little nicer
 *
 * Revision 2.32  2006/06/27 05:07:48  taylor
 * fix various compiler warnings and things that Valgrind complained about
 *
 * Revision 2.31  2006/05/27 17:13:22  taylor
 * add NO_DIRECT3D support
 *
 * Revision 2.30  2006/05/13 06:59:48  taylor
 * MVE player (audio only works with OpenAL builds!)
 *
 * Revision 2.29  2005/12/28 22:17:01  taylor
 * deal with cf_find_file_location() changes
 * add a central parse_modular_table() function which anything can use
 * fix up weapon_expl so that it can properly handle modular tables and LOD count changes
 * add support for for a fireball TBM (handled a little different than a normal TBM is since it only changes rather than adds)
 *
 * Revision 2.28  2005/11/13 06:46:10  taylor
 * some minor movie cleanup, not what I would like but enough for now
 *
 * Revision 2.27  2005/10/16 23:15:46  wmcoolmon
 * Hardened cfile against array overflows
 *
 * Revision 2.26  2005/05/12 17:37:48  taylor
 * add an extra gr_flip() after gr_clear() to make sure the change is made active
 *
 * Revision 2.25  2005/04/24 06:52:23  wmcoolmon
 * Enabled movies in windowed mode; if this causes problems for people, they can always use -dnoshowvid
 *
 * Revision 2.24  2005/02/05 00:30:49  taylor
 * fix a few things post merge
 *
 * Revision 2.23  2004/11/27 10:42:18  taylor
 * this should get movies working again in D3D and help OGL too
 *
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

#ifdef _WIN32
#include <windows.h>
#include "directx/dx8show.h"
#include "graphics/grd3dinternal.h"
#endif

#include "cutscene/movie.h"
#include "osapi/osapi.h"
#include "cmdline/cmdline.h"	
#include "debugconsole/dbugfile.h" 
#include "cfile/cfile.h"
#include "cutscene/cutscenes.h" // cutscene_mark_viewable()
#include "freespace2/freespace.h" // for Game_mode, etc.
#include "cutscene/mvelib.h"
#include "menuui/mainhallmenu.h"

// to know if we are in a movie, needed for non-MVE only
bool Playing_movie = false;

#define MOVIE_NONE	0
#define MOVIE_AVI	1
#define MOVIE_MPG	2
#define MOVIE_OGG	3
#define MOVIE_MVE	4

// This module links freespace movie calls to the actual API calls the play the movie.
// This module handles all the different requires of OS and gfx API and finding the file to play

void process_messages()
{
#ifdef _WIN32
	MSG msg;
   	while(PeekMessage(&msg, (HWND) os_get_window(), 0, 0, PM_REMOVE))
   	{
   		TranslateMessage(&msg);
   		DispatchMessage(&msg);
   	}
#endif
}


// filename		- file to search for
// out_name		- output, full path to file
// returns non-zero if file is found
int movie_find(char *filename, char *out_name)
{
	char full_path[MAX_PATH];
	char tmp_name[MAX_PATH];
	char search_name[MAX_PATH];
	int i, size, offset = 0;
	const int NUM_EXT = 4;
 	// NOTE: search order assumes that retail movies will be in MVE format, or that all movies will be in AVI format.
 	//       this isn't pretty, but short of CFILE changes to do filter searching, it's about the only option for mods
	const char *movie_ext[NUM_EXT] = { ".avi", ".mpg", ".ogg", ".mve" };

 	if (out_name == NULL)
 		return MOVIE_NONE;

	// remove extension
	strcpy( tmp_name, filename );
	char *p = strchr( tmp_name, '.' );
	if ( p ) *p = 0;
	
	for (i=0; i<NUM_EXT; i++) {
		strcpy( search_name, tmp_name );
		strcat( search_name, movie_ext[i] );

		// try and find the file
    	if ( cf_find_file_location(search_name, CF_TYPE_ANY, sizeof(full_path) - 1, full_path, &size, &offset, 0) ) {
			// if it's not in a packfile then we're done
			// NOTE: MVEs use CFILE internally for the player, so they can work out of VPs
			if ( ((i+1) == MOVIE_MVE) || (offset == 0) ) {
				strcpy( out_name, full_path );
				return (i+1); // return value should match one of MOVIE_* defines
			}
    	}
		
		// clear old string
		memset( search_name, 0, sizeof(search_name) );
	}

	return MOVIE_NONE;
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

	extern int Mouse_hidden;
	extern int Is_standalone;
	if(Is_standalone) return false;
 	if(Cmdline_nomovies) return false;
	//Commented this out since we have dnoshowvid -WMC
 	//if(Cmdline_window) return false;


	char full_name[MAX_PATH];
	int rc = 0;

	rc = movie_find(name, full_name);

	if (!rc) {
		mprintf(("MOVIE ERROR: Unable to open movie file '%s' in any supported format.", name));
		return false;
	} else {
		DBUGFILE_OUTPUT_1("About to play: %s", full_name);
	}

	// MVE checks first since they use a different player
	if (rc == MOVIE_MVE) {
		MVESTREAM *movie = NULL;

		// NOTE the MVE code uses CFILE, so only pass the base name and it will load up the movie properly
		movie = mve_open(name);

		if (movie) {
			// kill all background sounds
			main_hall_pause();

			// clear the screen and hide the mouse cursor
			Mouse_hidden++;
			gr_reset_clip();
			gr_set_color(255, 255, 255);
			gr_set_clear_color(0, 0, 0);
			gr_zbuffer_clear(0);
			gr_clear();
			gr_flip();

			// ready to play...
			mve_init(movie);
			mve_play(movie);

			// ... done playing, close the mve and show the cursor again
			mve_shutdown();
			mve_close(movie);
			Mouse_hidden--;

			main_hall_unpause();

			return true;
		} else {
			// uh-oh, MVE is invalid... Abory, Retry, Fail?
			mprintf(("MOVIE ERROR: Found invalid MVE! (%s)\n", full_name));
		}
	}

	// no MVE version so move on to AVI/MPG specific player code
#ifdef _WIN32
	process_messages();

#ifndef NO_DIRECT3D
	// This is a bit of a hack but it works nicely
 	if(gr_screen.mode == GR_DIRECT3D)
	{
   		GlobalD3DVars::D3D_activate = 0;
		GlobalD3DVars::lpD3DDevice->EndScene();
	  	GlobalD3DVars::lpD3DDevice->Present(NULL,NULL,NULL,NULL);
		d3d_lost_device();
	}
#endif

	Playing_movie = true;

	// reset the gr_* stuff before trying to play a movie
	gr_clear();
	gr_zbuffer_clear(1);
	gr_flip(); // cycle in the clear'd buffer

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

	if (OpenClip((HWND) os_get_window(), full_name)) {
		do {
			// Give system threads time to run (and don't sample user input madly)
			Sleep(100);

			MSG msg;

			while (PeekMessage(&msg, (HWND) os_get_window(), 0, 0, PM_REMOVE)) {
 				TranslateMessage(&msg);

				if (msg.message == WM_ERASEBKGND)
					continue;

				PassMsgToVideoWindow((HWND) os_get_window(), msg.message, msg.wParam, msg.lParam);

				if (msg.message == WM_KEYDOWN) {
					switch (msg.wParam)
					{
						// Quits movie if escape, space or return is pressed
						case VK_ESCAPE:
						case VK_SPACE:
						case VK_RETURN:
						{
							// Terminate movie playback early
							CloseClip((HWND) os_get_window());
#ifndef NO_DIRECT3D
							if (gr_screen.mode == GR_DIRECT3D) {
						 		GlobalD3DVars::D3D_activate = 1;
							}
#endif
							Playing_movie = false;
							return true;
						}
					}
				} else {
			   	  	DispatchMessage(&msg);
				}
			}
		}
		// Stream bit of the movie then handle windows messages
		while (dx8show_stream_movie() == false);
	}

	// We finished playing the movie
	CloseClip((HWND) os_get_window());

#ifndef NO_DIRECT3D
	if (gr_screen.mode == GR_DIRECT3D) {
	 	GlobalD3DVars::D3D_activate = 1;
	}
#endif

	Playing_movie = false;

#else

	STUB_FUNCTION;

#endif

	return true;
}

bool movie_play_two(char *name1, char *name2)
{
	bool result1 = movie_play(name1);
	process_messages();
	bool result2 = movie_play(name2);

	return result1 && result2;
}
