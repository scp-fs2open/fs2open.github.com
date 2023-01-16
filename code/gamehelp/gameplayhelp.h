/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __GAMEPLAY_HELP_H__
#define __GAMEPLAY_HELP_H__

struct gameplay_help_section {
	SCP_string title;
	SCP_string header;
	SCP_vector<SCP_string> key;
	SCP_vector<SCP_string> text;
};

SCP_vector<gameplay_help_section> gameplay_help_init_text();

void gameplay_help_init();
void gameplay_help_do_frame(float frametime);

#endif
