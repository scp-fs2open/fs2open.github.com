/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __MISSIONSHIPCHOICE_H__
#define __MISSIONSHIPCHOICE_H__

struct p_object;

///////////////////////////////////////////////////////
// Ships selection hot spots
///////////////////////////////////////////////////////
#define SHIP_SELECT_SHIP_SCROLL_UP				8
#define SHIP_SELECT_SHIP_SCROLL_DOWN			9

#define SHIP_SELECT_ICON_0							10
#define SHIP_SELECT_ICON_1							11
#define SHIP_SELECT_ICON_2							12
#define SHIP_SELECT_ICON_3							13

#define WING_0_SHIP_0								14
#define WING_0_SHIP_1								15
#define WING_0_SHIP_2								16
#define WING_0_SHIP_3								17
#define WING_1_SHIP_0								18
#define WING_1_SHIP_1								19
#define WING_1_SHIP_2								20
#define WING_1_SHIP_3								21
#define WING_2_SHIP_0								22
#define WING_2_SHIP_1								23
#define WING_2_SHIP_2								24
#define WING_2_SHIP_3								25

#define SHIP_SELECT_RESET							39

#define NUM_SHIP_SELECT_REGIONS					(NUM_COMMON_REGIONS + 19)

#define MAX_NUM_SHIP_DESC_LINES						10
#define SHIP_SELECT_SHIP_INFO_MAX_LINE_LEN			150

extern int Ship_select_open;	// This game-wide global flag is set to 1 to indicate that the ship
										// select screen has been opened and memory allocated.  This flag
										// is needed so we can know if ship_select_close() needs to called if
										// restoring a game from the Options screen invoked from ship select

extern int Commit_pressed;	// flag to indicate that the commit button was pressed
									// use a flag, so the ship_create() can be done at the end of the loop

extern char default_player_ship[255];
extern int Select_default_ship;

extern float ShipSelectScreenShipRot;
extern int	 ShipSelectModelNum;

void draw_wing_block(int wb_num, int hot_slot, int selected_slot, int class_select);
void ship_select_init();
void ship_select_do(float frametime);
void ship_select_close();
void ship_select_common_init();
void ship_select_common_close();
int ss_get_ship_class(int ship_entry_index);
int ss_get_selected_ship();

void ss_blit_ship_icon(int x,int y,int ship_class,int bmap_num);

// called from weapon select
int	ss_return_ship(int wing_block, int wing_slot, int *ship_index, p_object **ppobjp);
void	ss_return_name(int wing_block, int wing_slot, char *name);
int	ss_return_original_ship_class(int slot_num);
int	ss_return_saindex(int slot_num);
int	ss_disabled_slot(int slot_num);
int	ss_valid_slot(int slot_num);
int	ss_wing_slot_is_console_player(int index);

// lock/unlock any necessary slots for multiplayer
void ss_recalc_multiplayer_slots();

int	create_default_player_ship( int use_last_flown = 1 );
void	update_player_ship(int si_index);

void ss_synch_interface();

// called by multiplayer team select to set the slot based flags
void ss_make_slot_empty(int slot_index);
void ss_make_slot_full(int slot_index);

int ss_dump_to_list(int from_slot, int to_list, int *sound);
int ss_swap_slot_slot(int from_slot, int to_slot, int *sound);
int ss_grab_from_list(int from_list, int to_slot, int *sound);
int ss_swap_list_slot(int from_list, int to_slot, int *sound);

void ss_apply(int mode, int from_slot,int from_index,int to_slot,int to_index,int player_index = -1);
void ss_drop(int from_slot,int from_index,int to_slot,int to_index,int player_index = -1);

#endif  /* __MISSIONSHIPCHOICE_H__ */
