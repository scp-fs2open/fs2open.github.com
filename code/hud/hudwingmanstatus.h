/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __HUDWINGMAN_STATUS_H__
#define __HUDWINGMAN_STATUS_H__

#include "hud/hud.h"

struct wing;
class ship;
class p_object;

void	hud_init_wingman_status_gauge();
void	hud_wingman_status_update();
void	hud_wingman_status_init_flash();
void	hud_set_wingman_status_dead(int wing_index, int wing_pos);
void	hud_set_wingman_status_departed(int wing_index, int wing_pos);
void	hud_set_wingman_status_alive( int wing_index, int wing_pos);
void	hud_set_wingman_status_none( int wing_index, int wing_pos);
void	hud_wingman_status_start_flash(int wing_index, int wing_pos);
void	hud_wingman_status_set_index(wing *wingp, ship *shipp, p_object *pobjp);

// for resetting the gauge via sexp
void	hud_set_new_squadron_wings(const std::array<int, MAX_SQUADRON_WINGS> &new_squad_wingnums);

class HudGaugeWingmanStatus: public HudGauge
{
protected:
	hud_frames Wingman_status_left;
	hud_frames Wingman_status_middle;
	hud_frames Wingman_status_right;
	hud_frames Wingman_status_dots;

	int header_offsets[2];
	bool fixed_header_position;
	int left_frame_end_x;
	int right_frame_start_offset;
	
	int actual_origin[2];
	int single_wing_offsets[2];
	int multiple_wing_offsets[2];
	int wing_width;
	int wing_name_offsets[2];

	enum {GROW_LEFT, GROW_RIGHT, GROW_DOWN};
	enum {ALIGN_CENTER, ALIGN_LEFT, ALIGN_RIGHT};
	int grow_mode;
	int wingname_align_mode;
	bool use_full_wingnames;
	bool use_expanded_colors;

	int wingmate_offsets[MAX_SHIPS_PER_WING][2];

	int next_flash[MAX_SQUADRON_WINGS][MAX_SHIPS_PER_WING];
	int flash_status;
public:
	HudGaugeWingmanStatus();
	void initBitmaps(char *fname_left, char *fname_middle, char *fname_right, char *fname_dots);
	void initHeaderOffsets(int x, int y);
	void initFixedHeaderPosition(bool fixed);
	void initLeftFrameEndX(int x);
	void initSingleWingOffsets(int x, int y);
	void initMultipleWingOffsets(int x, int y);
	void initWingWidth(int w);
	void initRightBgOffset(int offset);
	void initWingNameOffsets(int x, int y);
	void initWingmate1Offsets(int x, int y);
	void initWingmate2Offsets(int x, int y);
	void initWingmate3Offsets(int x, int y);
	void initWingmate4Offsets(int x, int y);
	void initWingmate5Offsets(int x, int y);
	void initWingmate6Offsets(int x, int y);
	void initGrowMode(int mode);
	void initWingnameAlignMode(int mode);
	void initUseFullWingnames(bool usefullname);
	void initUseExpandedColors(bool useexpandedcolors);
	void pageIn() override;
	void initialize() override;
	void render(float frametime, bool config = false) override;
	void renderBackground(int num_wings_to_draw, bool config);
	void renderDots(int wing_index, int screen_index, int num_wings_to_draw, bool config);
	void initFlash();
	bool maybeFlashStatus(int wing_index, int wing_pos);
};

#endif
