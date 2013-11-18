/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __FREESPACE_HUDESCORT_VIEW_H__
#define __FREESPACE_HUDESCORT_VIEW_H__

#include "hud/hud.h"

//Odd def for escort frames
#define NUM_ESCORT_FRAMES 3

extern int Max_escort_ships;

class object;

void	hud_escort_update_list();
void	hud_escort_init();
void	hud_setup_escort_list(int level = 1);
void	hud_escort_view_toggle();
void	hud_add_remove_ship_escort(int objnum, int supress_feedback = 0);
void	hud_escort_clear_all(bool clear_flags = false);
void	hud_escort_ship_hit(object *objp, int quadrant);
void	hud_escort_target_next();
void	hud_escort_cull_list();
void	hud_add_ship_to_escort(int objnum, int supress_feedback);
void  hud_remove_ship_from_escort(int objnum);
int	hud_escort_num_ships_on_list();
int	hud_escort_return_objnum(int index);
void	hud_escort_add_player(short id);
void	hud_escort_remove_player(short id);

class HudGaugeEscort: public HudGauge
{
protected:
	hud_frames Escort_gauges[NUM_ESCORT_FRAMES];

	int header_text_offsets[2];			// coordinates of the header text
	char header_text[NAME_LENGTH];		// Header text for the Escort Gauge. Default is "monitoring"
	int list_start_offsets[2];					// Offset Start of the Ship List
	int entry_h;						// the height of each entry
	int entry_stagger_w;				// width of the staircase effect
	int bottom_bg_offset;
	int ship_name_offsets[2];					// Offset of the Ship Name column
	int ship_integrity_offsets[2];			// Offset of the Ship Hull column
	int ship_status_offsets[2];				// Offset of the Ship Status column
	int ship_name_max_width;			// max width of ship name entries
public:
	HudGaugeEscort();
	void initBitmaps(char *fname_top, char *fname_middle, char *fname_bottom);
	void initHeaderText(char *text);
	void initHeaderTextOffsets(int x, int y);
	void initListStartOffsets(int x, int y);
	void initEntryHeight(int h);
	void initEntryStaggerWidth(int w);
	void initBottomBgOffset(int offset);
	void initShipNameOffsets(int x, int y);
	void initShipIntegrityOffsets(int x, int y);
	void initShipStatusOffsets(int x, int y);
	void initShipNameMaxWidth(int w);
	int setGaugeColorEscort(int index, int team);
	virtual void render(float frametime);
	void pageIn();
	void renderIcon(int x, int y, int index);
	void renderIconDogfight(int x, int y, int index);
};

#endif /* __FREESPACE_HUDESCORT_VIEW_H__ */
