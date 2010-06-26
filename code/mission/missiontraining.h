/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



extern int Max_directives;
extern int Training_message_method;
extern int Training_num_lines;
extern int Training_message_visible;
extern int Training_failure;

void training_mission_init();
void training_mission_shutdown();
void training_check_objectives();
void message_training_queue(char *text, int timestamp, int length = -1);
void message_training_setup(int num, int length = -1);
void message_training_display();
void message_translate_tokens(char *buf, char *text);
void training_fail();
