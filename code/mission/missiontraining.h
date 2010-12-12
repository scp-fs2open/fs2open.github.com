/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#ifndef __MISSIONTRAINING_H__
#define __MISSIONTRAINING_H__

#include "hud/hud.h"

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
void message_translate_tokens(char *buf, char *text);
void training_fail();
void message_training_update_frame();

class HudGaugeDirectives: public HudGauge
{
protected:
	hud_frames directives_top;
	hud_frames directives_middle;
	hud_frames directives_bottom;

	int header_offsets[2];
	int middle_frame_offset_y;
	int text_start_offsets[2];
	int text_h;
public:
	HudGaugeDirectives();
	void initBitmaps(char *fname_top, char *fname_middle, char *fname_bottom);
	void initHeaderOffsets(int x, int y);
	void initMiddleFrameOffsetY(int y);
	void initTextStartOffsets(int x, int y);
	void initTextHeight(int h);
	void render(float frametime);
	void pageIn();
	bool canRender();
};

class HudGaugeTrainingMessages: public HudGauge
{
protected:
public:
	HudGaugeTrainingMessages();
	void render(float frametime);
	void pageIn();
	bool canRender();
};

#endif /* __MISSIONTRAINING_H__ */