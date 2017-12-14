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
#include "globalincs/alphacolors.h"
#include "globalincs/systemvars.h"
#include "io/cursor.h"
#include "graphics/2d.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"	
#include "cutscene/cutscenes.h" // cutscene_mark_viewable()
#include "cutscene/player.h" // cutscene_mark_viewable()
#include "tracing/categories.h"
#include "tracing/tracing.h"
#include "io/timer.h"
#include "io/key.h"

extern int Game_mode;
extern int Is_standalone;

namespace {

using namespace cutscene;

struct PlaybackState {
	bool playing = true;

	vec2d posTopLeft;
	vec2d posBottomRight;
};

void processEvents(PlaybackState* state) {
	io::mouse::CursorManager::get()->showCursor(false);

	os_poll();

	int k = key_inkey();
	switch (k) {
	case KEY_ESC:
	case KEY_ENTER:
	case KEY_SPACEBAR:
		state->playing = false;
		break;
	default:
		break;
	}
}

template<typename... Args>
float print_string(float x, float y, const char* fmt, Args... params) {
	SCP_string text;
	sprintf(text, fmt, params...);

	gr_string(x, y, text.c_str(), GR_RESIZE_NONE);

	return y + font::get_current_font()->getHeight();
}

void showVideoInfo(const PlayerState& state) {
	gr_set_color_fast(&Color_white);

	float y = 200.f;
	float x = 100.f;
	y = print_string(x, y, "Movie FPS: %f", state.props.fps);
	y = print_string(x, y, "Size: %dx%d", state.props.size.width, state.props.size.height);

	y = gr_screen.max_h - 200.f;

	size_t audio_queue_size = state.decoder->getAudioQueueSize();
	if (state.hasAudio) {
		ALint queued;
		OpenAL_ErrorPrint(alGetSourcei(state.audioSid, AL_BUFFERS_QUEUED, &queued));
		audio_queue_size += queued;
	}

	y = print_string(x, y, "Audio Queue size: " SIZE_T_ARG, audio_queue_size);
	y = print_string(x, y, "Video Queue size: " SIZE_T_ARG, state.decoder->getVideoQueueSize());
	y += font::get_current_font()->getHeight();
	// Estimate the size of the video buffer
	// We use YUV420p frames so one pixel uses 1.5 bytes of storage
	size_t single_frame_size = (size_t) (state.props.size.width * state.props.size.height * 1.5);
	size_t total_size = single_frame_size * state.decoder->getVideoQueueSize();
	print_string(x, y, "Video buffer size: " SIZE_T_ARG "B", total_size);
}

void displayVideo(Player* player, PlaybackState* state) {
	TRACE_SCOPE(tracing::CutsceneDrawVideoFrame);

	gr_clear();
	player->draw(state->posTopLeft.x, state->posTopLeft.y, state->posBottomRight.x, state->posBottomRight.y);

	if (Cmdline_show_video_info) {
		showVideoInfo(player->getInternalState());
	}

	gr_flip();
}

void determine_display_positions(Player* player, PlaybackState* state) {
	auto& props = player->getMovieProperties();

	float screen_ratio = (float) gr_screen.center_w / (float) gr_screen.center_h;
	float movie_ratio = (float) props.size.width / (float) props.size.height;

	float scale_by;
	if (screen_ratio > movie_ratio) {
		scale_by = (float) gr_screen.center_h / (float) props.size.height;
	} else {
		scale_by = (float) gr_screen.center_w / (float) props.size.width;
	}

	float screenX;
	float screenY;

	if (!Cmdline_noscalevid && (scale_by != 1.0f)) {
		screenX = ((gr_screen.center_w / 2.0f + gr_screen.center_offset_x) / scale_by) - (static_cast<int>(props.size.width) / 2.0f) + 0.5f;
		screenY = ((gr_screen.center_h / 2.0f + gr_screen.center_offset_y) / scale_by) - (static_cast<int>(props.size.height) / 2.0f) + 0.5f;
	} else {
		// centers on 1024x768, fills on 640x480
		screenX = i2fl(((gr_screen.center_w - static_cast<int>(props.size.width)) / 2) + gr_screen.center_offset_x);
		screenY = i2fl(((gr_screen.center_h - static_cast<int>(props.size.height)) / 2) + gr_screen.center_offset_y);
	}

	// set additional values for screen width/height and UV coords
	float screenXW = screenX + static_cast<int>(props.size.width);
	float screenYH = screenY + static_cast<int>(props.size.height);

	if (!Cmdline_noscalevid && (scale_by != 1.0f)) {
		// Apply scaling
		screenX *= scale_by;
		screenY *= scale_by;

		screenXW *= scale_by;
		screenYH *= scale_by;
	}

	state->posTopLeft.x = screenX;
	state->posTopLeft.y = screenY;

	state->posBottomRight.x = screenXW;
	state->posBottomRight.y = screenYH;
}

void movie_display_loop(Player* player, PlaybackState* state) {
	// Compute the maximum time we will sleep to make sure we can still maintain the movie FPS
	// and not waste too much CPU time
	// We will sleep at most half the time a frame would be displayed
	auto sleepTime = static_cast<std::uint64_t>((1. / (4. * player->getMovieProperties().fps)) * 1000.);

	auto lastDisplayTimestamp = timer_get_microseconds();
	while (state->playing) {
		TRACE_SCOPE(tracing::CutsceneStep);

		auto timestamp = timer_get_microseconds();

		auto passed = timestamp - lastDisplayTimestamp;
		lastDisplayTimestamp = timestamp;

		// Play as long as the player reports that there is more to display
		state->playing = player->update(passed);

		displayVideo(player, state);

		processEvents(state);

		if (passed < sleepTime) {
			auto sleep = sleepTime - passed;

			os_sleep(static_cast<uint>(sleep));
		}
	}
}

}

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
		PlaybackState state;
		determine_display_positions(player.get(), &state);

		movie_display_loop(player.get(), &state);

		player->stopPlayback();
	} else {
		// uh-oh, movie is invalid... Abory, Retry, Fail?
		mprintf(("MOVIE ERROR: Found invalid movie! (%s)\n", name));
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
