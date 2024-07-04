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

extern char Credits_music_name[NAME_LENGTH];
extern int Credits_num_images;
extern int Credits_artwork_index;
extern float Credits_scroll_rate;
extern float Credits_artwork_display_time;
extern float Credits_artwork_fade_time;

//This is used to store the entire credits string for the SCPUI API
extern SCP_string credits_complete;

void credits_init();
void credits_do_frame(float frametime);
void credits_close();

void credits_parse(bool split_lines = true);
void credits_scp_position();
const char* credits_get_music_filename(const char* music);
extern SCP_vector<SCP_string> Credit_text_parts;

void credits_stop_music(bool fade);

#endif /* __CREDITS_H__ */
