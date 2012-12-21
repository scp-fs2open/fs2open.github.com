/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __MISSIONHOTKEY_H__
#define __MISSIONHOTKEY_H__

void mission_hotkey_init();
void mission_hotkey_close();
void mission_hotkey_do_frame(float frametime);
void mission_hotkey_set_defaults();
void mission_hotkey_validate();
void mission_hotkey_maybe_save_sets();
void mission_hotkey_reset_saved();
void mission_hotkey_mf_add( int set, int objnum, int how_to_add );

void mission_hotkey_exit();

// function to return the hotkey set number of the given key
extern int mission_hotkey_get_set_num( int k );

#endif
