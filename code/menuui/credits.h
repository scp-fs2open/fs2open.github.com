/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __CREDITS_H__
#define __CREDITS_H__

struct credits_info {
	SCP_string music;						// the mission name
	int num_images;							// the mission filename
	int start_index;						// the mission description
	float scroll_rate;						// the mission designer
	float art_display_time;					// the mission designer
	float art_fade_time;					// the mission designer
	SCP_vector<SCP_string> credit_parts;	// the lines of credits
	SCP_string credits_complete;			// the lines of credits
};

extern credits_info Credits_Info;

void credits_init();
void credits_do_frame(float frametime);
void credits_close();

void credits_parse();
void credits_scp_position();
char* credits_get_music_filename(const char* music);
static SCP_vector<SCP_string> Credit_text_parts;

void credits_stop_music(bool fade);

#endif /* __CREDITS_H__ */
