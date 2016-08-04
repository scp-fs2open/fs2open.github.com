/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifdef _WIN32
#include <windows.h>
#endif

#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "io/cursor.h"
#include "graphics/2d.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"	
#include "cutscene/cutscenes.h" // cutscene_mark_viewable()
#include "cutscene/player.h" // cutscene_mark_viewable()

#include "cutscene/mve/mvelib.h"

extern int Game_mode;
extern int Is_standalone;

namespace movie {
// Play one movie
bool play(const char* name) {
	// mark the movie as viewable to the player when in a campaign
	// do this before anything else so that we're sure the movie is available
	// to the player even if it's not going to play right now
	if (Game_mode & GM_CAMPAIGN_MODE) {
		cutscene_mark_viewable(name);
	}

	if (Cmdline_nomovies || Is_standalone) {
		return false;
	}

	// clear the screen and hide the mouse cursor
	io::mouse::CursorManager::get()->pushStatus();
	io::mouse::CursorManager::get()->showCursor(false);
	gr_reset_clip();
	gr_set_color(255, 255, 255);
	gr_set_clear_color(0, 0, 0);
	gr_zbuffer_clear(0);
	// clear first buffer
	gr_clear();
	gr_flip();
	// clear second buffer (may not be one, but that's ok)
	gr_clear();
	gr_flip();
	// clear third buffer (may not be one, but that's ok)
	gr_clear();

	auto player = cutscene::Player::newPlayer(name);
	if (player) {
		player->startPlayback();
	} else {
		// *sigh* don't bother using MVE with the new system, it's not worth the effort...
		MVESTREAM* movie_mve = mve_open(name);

		if (movie_mve) {
			// start playing ...
			mve_init(movie_mve);
			mve_play(movie_mve);

			// ... done playing, close the movie
			mve_shutdown();
			mve_close(movie_mve);
		} else {
			// uh-oh, movie is invalid... Abory, Retry, Fail?
			mprintf(("MOVIE ERROR: Found invalid movie! (%s)\n", name));
		}
	}

	// show the mouse cursor again
	io::mouse::CursorManager::get()->popStatus();

	return true;
}

void play_two(const char* name1, const char* name2) {
	if (play(name1)) {
		play(name2);
	}
}
}
