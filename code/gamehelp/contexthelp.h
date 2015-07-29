/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

// ContextHelp.h
//
//

#ifndef __CONTEXTHELP_H__
#define __CONTEXTHELP_H__

// Help overlays
//
#define	MAX_HELP_OVERLAYS				30

#define	SS_OVERLAY						"ship"	// ship selection help
#define	WL_OVERLAY						"weapon"	// weapons loadout help
#define	BR_OVERLAY						"briefing"	// briefing help
#define	MH_OVERLAY						"main"	// main hall help
#define	BARRACKS_OVERLAY				"barracks"	// barracks help
#define	CONTROL_CONFIG_OVERLAY		"control"	// control config help
#define	DEBRIEFING_OVERLAY			"debrief"	// debriefing help
#define	MULTI_CREATE_OVERLAY			"multicreate"	// multi create game help
#define	MULTI_START_OVERLAY			"multistart"	// multi start game help overlay
#define	MULTI_JOIN_OVERLAY			"multijoin"	// join game help overlay
#define	MH2_OVERLAY						"main2"	// main hall 2 help overlay
#define	HOTKEY_OVERLAY					"hotkey" // hotkey assignment help overlay
#define	CAMPAIGN_ROOM_OVERLAY		"campaign" // campaign room help overlay
#define	SIM_ROOM_OVERLAY				"simulator"	// sim room help overlay
#define	TECH_ROOM_OVERLAY				"tech"	// tech room (general) help overlay
#define	CMD_BRIEF_OVERLAY				"command" // command briefing help overlay

// other help overlay constants
#define HELP_PADDING		1							//
#define HELP_MAX_NAME_LENGTH	32			// max string length for overlay name
#define HELP_MAX_STRING_LENGTH	128			// max string length for text overlay element
#define HELP_PLINE_THICKNESS		2
#define HELP_OVERLAY_FILENAME		"help.tbl"

// help overlay calls
int	help_overlay_get_index(const char* overlay_name);
int	help_overlay_active(int overlay_id);
void	help_overlay_set_state(int overlay_id, int resolution_index, int state);
void	help_overlay_maybe_blit(int overlay_id, int resolution_index);

void context_help_init();			// called once at game startup
void context_help_grey_screen();	// call to grey out a screen (normally when applying a help overlay)

void launch_context_help();


#endif /* __CONTEXTHELP_H__ */
