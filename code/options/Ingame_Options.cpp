#include "gamesequence/gamesequence.h"
#include "options/Ingame_Options.h"
#include "io/key.h"
#include "gamesequence/gamesequence.h"

#include "freespace.h"

void ingame_options_init()
{
	gr_set_clear_color(0, 0, 0);
}

void ingame_options_close()
{
	game_flush();
}

void ingame_options_do_frame(float frametime)
{
	// getLabManager()->onFrame(frametime);
	int key = game_check_key();

	if (key != 0) {
		// handle any key presses
		switch (key) {
			case KEY_ESC:
				gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
				break;
		}
	}

	gr_reset_clip();
	gr_clear();
	gr_flip();
}
