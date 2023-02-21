/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 





#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "globalincs/linklist.h"
#include "globalincs/systemvars.h"
#include "hud/hudescort.h"
#include "hud/hudmessage.h"
#include "hud/hudparse.h"
#include "hud/hudshield.h"
#include "hud/hudtargetbox.h"
#include "iff_defs/iff_defs.h"
#include "io/timer.h"
#include "network/multi.h"
#include "network/multiutil.h"
#include "object/object.h"
#include "parse/parselo.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "weapon/emp.h"


static int Show_escort_view;

typedef struct escort_info
{
	int					objnum;
	int					obj_signature;	// so we are sure we have a valid objnum
	int					priority;		// higher priority is higher in the list
	short				np_id;			// netplayer id (for multiplayer dogfight mode)

	// These parallel the way the shield_hit_info struct works; intentionally
	// not using it here because this is a lot simpler and less error-prone
	int					escort_hit_timer;
	int					escort_hit_next_flash;
	bool				escort_show_bright;

	escort_info() : objnum(-1), obj_signature(-1), priority(-1), np_id(-1),
		escort_hit_timer(0), escort_hit_next_flash(0), escort_show_bright(false) {}
} escort_info;

static SCP_list<escort_info> Escort_ships;
int Max_escort_ships = 3;

static int Last_target_index;	// index into Escort_gauges for last targeted via 'Next Escort Target' key


static SCP_list<escort_info>::iterator get_escort_entry_from_index(int index)
{
	int e_idx = 0;

	auto it = Escort_ships.begin();

	while ( (e_idx != index) && (it != Escort_ships.end()) ) {
		++it;
		++e_idx;
	}

	return it;
}

HudGaugeEscort::HudGaugeEscort():
HudGauge(HUD_OBJECT_ESCORT, HUD_ESCORT_VIEW, false, false, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeEscort::initHeaderText(char *text)
{
	strcpy_s(header_text, text);
}

void HudGaugeEscort::initHeaderTextOffsets(int x, int y)
{
	header_text_offsets[0] = x;
	header_text_offsets[1] = y;
}

void HudGaugeEscort::initListStartOffsets(int x, int y)
{
	list_start_offsets[0] = x;
	list_start_offsets[1] = y;
}

void HudGaugeEscort::initEntryHeight(int h)
{
	entry_h = h;
}

void HudGaugeEscort::initEntryStaggerWidth(int w)
{
	entry_stagger_w = w;
}

void HudGaugeEscort::initBottomBgOffset(int offset)
{
	bottom_bg_offset = offset;
}

void HudGaugeEscort::initShipNameOffsets(int x, int y)
{
	ship_name_offsets[0] = x;
	ship_name_offsets[1] = y;
}

void HudGaugeEscort::initShipIntegrityOffsets(int x, int y)
{
	ship_integrity_offsets[0] = x;
	ship_integrity_offsets[1] = y;
}

void HudGaugeEscort::initShipStatusOffsets(int x, int y)
{
	ship_status_offsets[0] = x;
	ship_status_offsets[1] = y;
}

void HudGaugeEscort::initShipNameMaxWidth(int w)
{
	ship_name_max_width = w;
}

void HudGaugeEscort::initRightAlignNames(bool align)
{
	right_align_names = align;
}

void HudGaugeEscort::initBitmaps(char *fname_top, char *fname_middle, char *fname_bottom)
{
	Escort_gauges[0].first_frame = bm_load_animation(fname_top, &Escort_gauges[0].num_frames);
	if (Escort_gauges[0].first_frame == -1) {
		Warning(LOCATION, "Could not load in ani: %s\n", fname_top);
		return;
	}

	Escort_gauges[1].first_frame = bm_load_animation(fname_middle, &Escort_gauges[1].num_frames);
	if (Escort_gauges[1].first_frame == -1) {
		Warning(LOCATION, "Could not load in ani: %s\n", fname_middle);
		return;
	}

	Escort_gauges[2].first_frame = bm_load_animation(fname_bottom, &Escort_gauges[2].num_frames);
	if (Escort_gauges[2].first_frame == -1) {
		Warning(LOCATION, "Could not load in ani: %s\n", fname_bottom);
		return;
	}
}

void HudGaugeEscort::pageIn()
{
	int i;

	for ( i = 0; i < NUM_ESCORT_FRAMES; i++ ) {
		bm_page_in_aabitmap( Escort_gauges[i].first_frame, Escort_gauges[i].num_frames);
	}
}

int HudGaugeEscort::setGaugeColorEscort(int index, int team)
{
	int is_flashing = 0;
	int is_bright = 0;
	int seen_from_team = (Player_ship != NULL) ? Player_ship->team : -1;
	int objnum = -1;

	// multiplayer dogfight
	if(MULTI_DOGFIGHT)
	{
		setGaugeColor();
		return 0;
	}

	auto eship = get_escort_entry_from_index(index);

	if (eship == Escort_ships.end()) {
		return 0;
	}

	if ( (Game_mode & GM_MULTIPLAYER) && (eship->np_id >= 0) ) {
		int np_index = find_player_index(eship->np_id);

		if ( (np_index >= 0) && (Net_players[np_index].m_player != nullptr) ) {
			objnum = Net_players[np_index].m_player->objnum;
		}
	} else {
		objnum = eship->objnum;
	}

	if (objnum < 0) {
		return 0;
	}

	// set flashing color
	if (!timestamp_elapsed(eship->escort_hit_timer))
	{
		is_flashing = 1;
		if (eship->escort_show_bright)
		{
			is_bright = 1;
		}
	}

	// Goober5000 - now base this on team color
	gr_set_color_fast(iff_get_color_by_team_and_object(team, seen_from_team, is_bright, &Objects[objnum]));


	// Goober5000 - an alternative; same as original but incorporating teams for non-friendlies
	/*
	if ((seen_from_team == team) || (seen_from_team < 0))	// :V: sez assume friendly if Player_ship is NULL
		hud_set_gauge_color(HUD_ESCORT_VIEW, is_bright ? HUD_C_BRIGHT : HUD_C_DIM);
	else
		gr_set_color_fast(iff_get_color_by_team(team, seen_from_team, is_bright));
	*/


	// Goober5000 - original color logic
	/*
	if ((seen_from_team == team) || (seen_from_team < 0))	// :V: sez assume friendly if Player_ship is NULL
		hud_set_gauge_color(HUD_ESCORT_VIEW, is_bright ? HUD_C_BRIGHT : HUD_C_DIM);
	else
		gr_set_color_fast(is_bright ? &Color_bright_red : &Color_red);
	*/


	return is_flashing;
}

void HudGaugeEscort::render(float  /*frametime*/)
{
	int	i = 0;

	if ( !Show_escort_view ) {
		return;
	}

	if ( Escort_ships.empty() ) {
		return;
	}

	// hud_set_default_color();
	setGaugeColor();

	// draw the top of the escort view
	renderBitmap(Escort_gauges[0].first_frame, position[0], position[1]);	
	renderString(position[0] + header_text_offsets[0], position[1] + header_text_offsets[1], header_text);

	int x = position[0] + list_start_offsets[0];
	int y = position[1] + list_start_offsets[1];

	//This is temporary
	int num_escort_ships = std::min(hud_escort_num_ships_on_list(), Max_escort_ships) - 1;
	i=0;

	if(num_escort_ships > 0)
	{
		for(; i < num_escort_ships; i++)
		{
			if(i != 0)
			{
				x += entry_stagger_w;
				y += entry_h;
			}
			renderBitmap(Escort_gauges[1].first_frame, x, y);
			
			//Now we just show the ships info
			renderIcon(x, y, i);
		}

		//Increment for last entry
		x += entry_stagger_w;
		y += entry_h;
	}

	//Show the last escort entry
	renderBitmap(Escort_gauges[2].first_frame, x, y + bottom_bg_offset);
	renderIcon(x, y, i);
}

// draw the shield icon and integrity for the escort ship
void HudGaugeEscort::renderIcon(int x, int y, int index)
{
	if(MULTI_DOGFIGHT && index <= 2)
	{
		renderIconDogfight(x, y, index);
		return;
	}

	float	shields, integrity;
	int		screen_integrity, offset, objnum = -1;
	char	buf[255];
	const player *pl = nullptr;

	auto eship = get_escort_entry_from_index(index);

	if (eship == Escort_ships.end()) {
		return;
	}

	if ( (Game_mode & GM_MULTIPLAYER) && (eship->np_id >= 0) ) {
		int np_index = find_player_index(eship->np_id);

		if (np_index >= 0) {
			pl = Net_players[np_index].m_player;
			objnum = pl->objnum;

			// this can occassionally happen in multi when a player still needs to respawn.
			if (objnum < 0 || Objects[objnum].type != OBJ_SHIP){
				return;
			}
			
		}

	} else {
		objnum = eship->objnum;
	}

	if (objnum < 0) {
		return;
	}

	object	*objp	= &Objects[objnum];
	ship	*sp		= &Ships[objp->instance];

	// determine if its "friendly" or not	
	// Goober5000 - changed in favor of just passing the team
	setGaugeColorEscort(index, sp->team);
	/*
	if(Player_ship != NULL){
		hud_escort_set_gauge_color(index, (sp->team == Player_ship->team) ? 1 : 0);
	} else {
		hud_escort_set_gauge_color(index, 1);
	}
	*/

	// draw a 'D' if a ship is disabled
	if ( (sp->flags[Ship::Ship_Flags::Disabled]) || (ship_subsys_disrupted(sp, SUBSYSTEM_ENGINE)) ) {		
		renderString( x + ship_status_offsets[0], y + ship_status_offsets[1], EG_NULL, XSTR( "D", 284));				
	}

	// print out ship name
	strcpy_s(buf, sp->get_display_name());

	// maybe add callsign if multi and player ship
	if (pl) {
		SCP_string callsign;

		callsign.reserve(32);

		callsign += " (";
		callsign += pl->short_callsign;
		callsign += ")";

		strcat_s(buf, callsign.c_str());
	}

	const int w = font::force_fit_string(buf, 255, ship_name_max_width);
	
	if (right_align_names) {
		renderString( x + ship_name_offsets[0] + ship_name_max_width - w, y + ship_name_offsets[1], EG_ESCORT1 + index, buf);
	} else {
		renderString( x + ship_name_offsets[0], y + ship_name_offsets[1], EG_ESCORT1 + index, buf);
	}

	// show ship integrity
	hud_get_target_strength(objp, &shields, &integrity);
	screen_integrity = (int)std::lround(integrity * 100);
	offset = 0;
	if ( screen_integrity < 100 ) {
		offset = 2;
		if ( screen_integrity == 0 ) {
			if ( integrity > 0 ) {
				screen_integrity = 1;
			}
		}
	}
	renderPrintf( x+ship_integrity_offsets[0] + offset, y+ship_integrity_offsets[1], EG_NULL, "%d", screen_integrity);

	//Let's be nice.
	setGaugeColor();
}

// multiplayer dogfight
void HudGaugeEscort::renderIconDogfight(int x, int y, int index)
{
	int			hull_integrity = 100;
	char			buf[255];	
	int			np_index;
	object		*objp;

	int stat_shift = 40;

	// always use the standard color to avoid confusion
	setGaugeColor();

	auto eship = get_escort_entry_from_index(index);

	if (eship == Escort_ships.end()) {
		return;
	}

	// netplayer index
	np_index = find_player_index(eship->np_id);

	if((np_index < 0) || (np_index >= MAX_PLAYERS) || (Net_players[np_index].m_player == NULL)){
		return;
	}
	
	// print out player name
	strcpy_s(buf, Net_players[np_index].m_player->callsign);
	font::force_fit_string(buf, 255, 100 - stat_shift);
	renderString( x + ship_name_offsets[0], y + ship_name_offsets[1], EG_ESCORT1 + index, buf);	

	// can we get the player object?
	objp = NULL;
	if((Net_players[np_index].m_player->objnum >= 0) && (Net_players[np_index].m_player->objnum < MAX_OBJECTS) && (Objects[Net_players[np_index].m_player->objnum].type == OBJ_SHIP)){
		objp = &Objects[Net_players[np_index].m_player->objnum];
		if((objp->instance >= 0) && (objp->instance < MAX_SHIPS) && (Ships[objp->instance].ship_info_index >= 0) && (Ships[objp->instance].ship_info_index < MAX_SHIPS)){
			//
		} else {
			return;
		}

		hull_integrity = (int)(((float)objp->hull_strength / (float)Ships[objp->instance].ship_max_hull_strength) * 100.0f);
		if(hull_integrity < 0){
			hull_integrity = 0;
		}
	}

	// show ship integrity
	if(objp == NULL){	
		renderPrintf( x+ship_integrity_offsets[0] - stat_shift, y+ship_integrity_offsets[1], EG_NULL, "%d", Net_players[np_index].m_player->stats.m_kill_count_ok);	
	} else {
		renderPrintf( x+ship_integrity_offsets[0] - stat_shift, y+ship_integrity_offsets[1], EG_NULL, "(%d%%) %d", hull_integrity, Net_players[np_index].m_player->stats.m_kill_count_ok);	
	}
}

void hud_escort_update_list()
{
	for (auto &es : Escort_ships) {
		if ( !timestamp_elapsed(es.escort_hit_timer) ) {
			if (timestamp_elapsed(es.escort_hit_next_flash)) {
				es.escort_hit_next_flash = timestamp(SHIELD_FLASH_INTERVAL);
				es.escort_show_bright = !es.escort_show_bright;	// toggle between default and bright frames
			}
		}
	}
}

// called from HUD init, loads the bitmap data in once, and resets any data for each level
void hud_escort_init()
{
	Last_target_index = -1;

	if (Max_escort_ships > MAX_COMPLETE_ESCORT_LIST) {
		Max_escort_ships = MAX_COMPLETE_ESCORT_LIST;
	}
}

// ----------------------------------------------------------------------
// hud_escort_clear_all()
//
void hud_escort_clear_all(bool clear_flags)
{
	if (clear_flags) {
		for (auto &es : Escort_ships) {
			if ( (es.objnum >= 0) && (Objects[es.objnum].type == OBJ_SHIP) && (Objects[es.objnum].instance >= 0) ) {
				Ships[Objects[es.objnum].instance].flags.remove(Ship::Ship_Flags::Escort);
			}
		}
	}

	Escort_ships.clear();
}

// internal helper function for sort.
// sorts first by priority number and then alphabetically
static bool escort_compare(const escort_info &escort1, const escort_info &escort2)
{
	if (MULTI_DOGFIGHT) {
		int n1 = find_player_index(escort1.np_id);
		int n2 = find_player_index(escort2.np_id);

		if ( (n1 < 0) || (n2 < 0) || (Net_players[n1].m_player == nullptr) || (Net_players[n2].m_player == nullptr) ) {
			return false;
		}

		return (Net_players[n1].m_player->stats.m_kill_count_ok >= Net_players[n2].m_player->stats.m_kill_count_ok);
	} else {
		// in multi, players go to the top
		if (Game_mode & GM_MULTIPLAYER) {
			if ( (escort1.np_id >= 0) && (escort2.np_id < 0) ) {
				return true;
			} else if ( (escort1.np_id < 0) && (escort2.np_id >= 0) ) {
				return false;
			}
		}

		if (escort1.priority != escort2.priority) {
			return (escort1.priority > escort2.priority);
		} else if ( (escort1.objnum >= 0) && (Objects[escort1.objnum].instance >= 0) &&
					(escort2.objnum >= 0) && (Objects[escort2.objnum].instance >= 0) )
		{
			const char *name1 = Ships[Objects[escort1.objnum].instance].ship_name;
			const char *name2 = Ships[Objects[escort2.objnum].instance].ship_name;

			return (stricmp(name1, name2) > 0);
		}
	}

	return false;
}

// create complete priority sorted escort list for all active ships
// escorts - array of escort info
// num_escorts - number of escorts requests in field of active ships
//	  This will be culled to MAX_ESCORTS, selecting the top set from escorts
void hud_create_complete_escort_list()
{
	ship_obj *so;
	object *objp;
	int idx;

	// multiplayer dogfight
	if (MULTI_DOGFIGHT) {
		for (idx = 0; idx < MAX_PLAYERS; idx++) {
			// is this a valid player
			if ( MULTI_CONNECTED(Net_players[idx]) && !MULTI_OBSERVER(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) ) {
				// add the ship
				escort_info einfo;
				einfo.np_id = Net_players[idx].player_id;

				Escort_ships.push_back(einfo);
			}
		}
	}
	// all others 
	else {
		for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
			Assert( so->objnum >= 0 && so->objnum < MAX_OBJECTS);
			if((so->objnum < 0) || (so->objnum >= MAX_OBJECTS)){
				continue;
			}
			objp = &Objects[so->objnum];
			// don't process objects that should be dead
			if (objp->flags[Object::Object_Flags::Should_be_dead]) {
				continue;
			}

			Assert( objp->type == OBJ_SHIP );
			if(objp->type != OBJ_SHIP){
				continue;
			}

			// only process ships that might be on the list
			if ( !(Ships[objp->instance].flags[Ship::Ship_Flags::Escort]) ) {
				continue;
			}

			// only process ships that can be seen by sensors
			if ( (Ships[objp->instance].flags[Ship::Ship_Flags::Hidden_from_sensors]) ) {
				continue;
			}

			// don't process most stealth ships
			if ( (Ships[objp->instance].flags[Ship::Ship_Flags::Stealth]) )
			{
				if ( Ships[objp->instance].team == Player_ship->team )
				{
					// friendly stealths are only not seen when explicitly specified
					if ( Ships[objp->instance].flags[Ship::Ship_Flags::Friendly_stealth_invis] )
					{
						continue;
					}
				}
				// non-friendly stealths are never seen
				else
				{
					continue;
				}
			}

			// add the ship
			escort_info einfo;
			einfo.objnum = so->objnum;
			einfo.obj_signature = objp->signature;
			einfo.priority = Ships[objp->instance].escort_priority;

			Escort_ships.push_back(einfo);
		}
	}
}


// ----------------------------------------------------------------------
// hud_init_escort_info()
//
// Set up the initial escort list
//
void hud_setup_escort_list(int level)
{
	// this should never be called while in a mission
	if (Game_mode & GM_IN_MISSION) {
		return;
	}

	Escort_ships.clear();

	// build escort list
	hud_create_complete_escort_list();

	// and sort it by priority
	Escort_ships.sort(escort_compare);

	// then resize the list to fit max
	if (hud_escort_num_ships_on_list() > MAX_COMPLETE_ESCORT_LIST) {
		Escort_ships.resize(MAX_COMPLETE_ESCORT_LIST);
	}

	if(level){
		Show_escort_view = 1;
	}
}

// re-sort the escort list
void hud_resort_escort_list()
{
	Escort_ships.sort(escort_compare);
}

// called once per frame to refresh the escort list if important flags changed
void hud_escort_cull_list()
{
	if ( Escort_ships.empty() ) {
		return;
	}

	int np_index;
	auto it = Escort_ships.begin();

	// multiplayer dogfight
	if(MULTI_DOGFIGHT){
		while (it != Escort_ships.end()) {
			np_index = find_player_index(it->np_id);

			// maybe remove him if he left
			if (np_index < 0) {
				it = Escort_ships.erase(it);
			} else {
				++it;
			}
		}
	}
	// everything else
	else {
		int objnum;
		bool remove;

		while (it != Escort_ships.end()) {
			remove = false;
			objnum = it->objnum;

			// player ships should stay so long as they are valid
			if ( (Game_mode & GM_MULTIPLAYER) && (it->np_id >= 0) ) {
				np_index = find_player_index(it->np_id);

				// maybe remove him if he left
				if (np_index < 0) {
					remove = true;
				}
			} else if (Objects[objnum].flags[Object::Object_Flags::Should_be_dead]) {
				remove = true;
			} else if (Objects[objnum].type == OBJ_SHIP) {
				int shipnum = Objects[objnum].instance;
				Assert( shipnum >= 0 && shipnum < MAX_SHIPS );

				if ( !(Ships[shipnum].flags[Ship::Ship_Flags::Escort]) ) {
					 remove = true;
				} else if (Ships[shipnum].flags[Ship::Ship_Flags::Hidden_from_sensors]) {
					remove = true;
				} else if (Ships[shipnum].flags[Ship::Ship_Flags::Stealth]) {
					if (Ships[shipnum].team == Player_ship->team) {
						// friendly stealths are only not seen when explicitly specified
						if ( Ships[shipnum].flags[Ship::Ship_Flags::Friendly_stealth_invis]) {
							remove = true;
						}
					}
					// non-friendly stealths are never seen
					else {
						remove = true;
					}
				}
			}

			if (remove) {
				it = Escort_ships.erase(it);
			} else {
				++it;
			}
		}
	}
}

// ----------------------------------------------------------------------
// hud_escort_view_toggle()
//
void hud_escort_view_toggle()
{
	Show_escort_view ^= 1;
	if ( Show_escort_view ) {
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, "%s", XSTR( "Escort view enabled", 286));
	} else {
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, "%s", XSTR( "Escort view disabled", 287));
	}
}

// try to add a ship to the escort list, if slot available
void hud_add_ship_to_escort(int objnum, int supress_feedback)
{
	// no ships on the escort list in multiplayer dogfight
	if (MULTI_DOGFIGHT) {
		return;
	}

	if (objnum < 0) {
		UNREACHABLE("Invalid objnum passed to hud_add_ship_to_escort()!");
		return;
	}

	if (Objects[objnum].type != OBJ_SHIP) {
		if ( !supress_feedback ) {
			snd_play( gamesnd_get_game_sound(GameSounds::TARGET_FAIL));
		}

		return;
	}

	// check if ship is already on complete escort list
	bool found = false;

	for (auto &es : Escort_ships) {
		if (es.obj_signature == Objects[objnum].signature) {
			found = true;
			break;
		}
	}

	// add new ship into complete list
	if ( !found ) {
		escort_info einfo;

		einfo.objnum = objnum;
		einfo.obj_signature = Objects[objnum].signature;
		einfo.priority = Ships[Objects[objnum].instance].escort_priority;

		// add him to escort list and re-sort
		Escort_ships.push_back(einfo);
		Escort_ships.sort(escort_compare);

		Ships[Objects[objnum].instance].flags.set(Ship::Ship_Flags::Escort);
	}

	if (hud_escort_num_ships_on_list() > MAX_COMPLETE_ESCORT_LIST) {
		Escort_ships.resize(MAX_COMPLETE_ESCORT_LIST);

		// maybe do feedback
		if ( !supress_feedback ) {
			found = false;

			// search thru list for objnum
			for (auto &es : Escort_ships) {
				if (es.objnum == objnum) {
					found = true;
					break;
				}
			}

			if ( !found ) {
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Escort list is full with %d ships", 288), hud_escort_num_ships_on_list());
				snd_play( gamesnd_get_game_sound(GameSounds::TARGET_FAIL));
			}
		}
	}

	hud_gauge_popup_start(HUD_ESCORT_VIEW);
}


// ----------------------------------------------------------------------
// hud_add_remove_ship_escort()
//
void hud_add_remove_ship_escort(int objnum, int supress_feedback)
{
	// no ships on the escort list in multiplayer dogfight
	if(MULTI_DOGFIGHT){
		return;
	}

	if ( objnum < 0 ) {
		Int3();
		return;
	}

	if ( Objects[objnum].type != OBJ_SHIP ) {
		if ( !supress_feedback ) {
			snd_play( gamesnd_get_game_sound(GameSounds::TARGET_FAIL));
		}
		return;
	}

	// if it's in the list already just remove it
	for (auto it = Escort_ships.begin(); it != Escort_ships.end(); ++it) {
		if (it->obj_signature == Objects[objnum].signature) {
			if ( (Objects[objnum].type == OBJ_SHIP) && (Objects[objnum].instance >= 0) ) {
				Ships[Objects[objnum].instance].flags.remove(Ship::Ship_Flags::Escort);
			}

			Escort_ships.erase(it);

			hud_gauge_popup_start(HUD_ESCORT_VIEW);

			return;
		}
	}

	// otherwise add ship to list
	hud_add_ship_to_escort(objnum, supress_feedback);
}

void hud_remove_ship_from_escort(int objnum)
{
	// no ships on the escort list in multiplayer dogfight
	if(MULTI_DOGFIGHT){
		return;
	}

	if ( objnum < 0 ) {
		Int3();
		return;
	}	

	for (auto it = Escort_ships.begin(); it != Escort_ships.end(); ++it) {
		if (it->obj_signature == Objects[objnum].signature) {
			if ( (Objects[objnum].type == OBJ_SHIP) && (Objects[objnum].instance >= 0) ) {
				Ships[Objects[objnum].instance].flags.remove(Ship::Ship_Flags::Escort);
			}

			Escort_ships.erase(it);

			hud_gauge_popup_start(HUD_ESCORT_VIEW);

			break;
		}
	}
}

/**
 * Called whenever a ship is hit to determine if that ship is in the escort list.  If it
 * is, then start timers to flash the name hull/shield icon for that ship.
 *
 * @param objp      The object hit
 * @param quadrant  Shield quadrant on the object that was hit, alternatively -1 if no shield
 */
void hud_escort_ship_hit(object *objp, int  /*quadrant*/)
{
	// no ships on the escort list in multiplayer dogfight
	if(MULTI_DOGFIGHT){
		return;
	}

	int objnum = OBJ_INDEX(objp);

	for (auto &es : Escort_ships) {
		if (es.objnum == objnum) {
			hud_gauge_popup_start(HUD_ESCORT_VIEW);
			es.escort_hit_timer = timestamp(SHIELD_HIT_DURATION);
			es.escort_hit_next_flash = timestamp(SHIELD_FLASH_INTERVAL);
			break;
		}
	}
}

// target the next ship in the escort list
void hud_escort_target_next()
{
	int objnum;

	if ( Escort_ships.empty() ) {
		snd_play( gamesnd_get_game_sound(GameSounds::TARGET_FAIL), 0.0f );
		return;
	}

	Last_target_index++;
	if ( Last_target_index >= hud_escort_num_ships_on_list() ) {
		Last_target_index = 0;
	}

	auto eship = get_escort_entry_from_index(Last_target_index);

	if ( (eship == Escort_ships.end()) || (eship->objnum < 0) ) {
		return;
	}

	objnum = eship->objnum;

	set_target_objnum( Player_ai,  objnum);
	hud_restore_subsystem_target(&Ships[Objects[objnum].instance]);
}

// return the number of ships currently on the escort list
int hud_escort_num_ships_on_list()
{
	return static_cast<int>(Escort_ships.size());
}

// Return the object number for the ship at index position in the escort list
int hud_escort_return_objnum(int index)
{
	int escort_objnum, escort_sig;

	if (index < 0) {
		return -1;
	}

	auto eship = get_escort_entry_from_index(index);

	if ( (eship == Escort_ships.end()) || (eship->objnum < 0) ) {
		return -1;
	}

	escort_objnum = eship->objnum;
	escort_sig = eship->obj_signature;

	// ensure this is still a valid index
	if (Objects[escort_objnum].signature != escort_sig) {
		return -1;
	}

	return escort_objnum;
}

void hud_escort_add_player(short id)
{
	Assert(Game_mode & GM_MULTIPLAYER);
	if(!(Game_mode & GM_MULTIPLAYER)){
		return;
	}	

	// just go through and add as long as its not a duplicate
	for (auto it = Escort_ships.begin(); it != Escort_ships.end(); ++it) {
		if (it->np_id == id) {
			return;
		}
	}

	escort_info einfo;

	einfo.np_id = id;

	Escort_ships.push_back(einfo);
	Escort_ships.sort(escort_compare);

	// resize the list to fit
	if (hud_escort_num_ships_on_list() > MAX_COMPLETE_ESCORT_LIST) {
		Escort_ships.resize(MAX_COMPLETE_ESCORT_LIST);
	}
}

void hud_escort_remove_player(short id)
{	
	Assert(Game_mode & GM_MULTIPLAYER);
	if(!(Game_mode & GM_MULTIPLAYER)){
		return;
	}

	// find the instance and remove it if possible
	for (auto it = Escort_ships.begin(); it != Escort_ships.end(); ++it) {
		if (it->np_id == id) {
			Escort_ships.erase(it);

			return;
		}
	}
}
