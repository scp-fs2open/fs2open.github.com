/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "mission/missiongoals.h"
#include "mission/missionlog.h"
#include "missionui/missionscreencommon.h"
#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"
#include "hud/hud.h"
#include "hud/hudmessage.h"
#include "io/key.h"
#include "io/timer.h"
#include "parse/parselo.h"
#include "parse/sexp.h"
#include "gamesnd/eventmusic.h"
#include "network/stand_gui.h"
#include "ui/ui.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "playerman/player.h"
#include "localization/localize.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multi_team.h"
#include "network/multi_sexp.h"
#include "mod_table/mod_table.h"



// timestamp stuff for evaluating mission goals
#define GOAL_TIMESTAMP				0				// make immediately eval
#define GOAL_TIMESTAMP_TRAINING	500			// every half second

#define MAX_GOALS_PER_LIST			15
#define MAX_GOAL_LINES	200

// indicies for coordinates
#define GOAL_SCREEN_X_COORD 0
#define GOAL_SCREEN_Y_COORD 1
#define GOAL_SCREEN_W_COORD 2
#define GOAL_SCREEN_H_COORD 3

/*
#define GOAL_SCREEN_TEXT_X	81
#define GOAL_SCREEN_TEXT_Y	95
#define GOAL_SCREEN_TEXT_W	385
#define GOAL_SCREEN_TEXT_H	299
#define GOAL_SCREEN_ICON_X 45
*/

static int Goal_screen_text_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		81,95,385,299		// GR_640
	},
	{
		130,152,385,299	// GR_1024
	}
};

static int Goal_screen_icon_xcoord[GR_NUM_RESOLUTIONS] = {
	45,	// GR_640
	72		// GR_1024
};


// german version gets slightly diff coords
static int Objective_key_text_coords_gr[GR_NUM_RESOLUTIONS][3][2] = {
	{
		// GR_640
		{175, 344},
		{316, 344},
		{432, 344}
	},
	{
		// GR_1024
		{310, 546},
		{536, 546},
		{688, 546}
	}
};
static int Objective_key_icon_coords_gr[GR_NUM_RESOLUTIONS][3][2] = {
	{
		// GR_640
		{150, 339},
		{290, 339},
		{406, 339}
	},
	{
		// GR_1024
		{272, 542},
		{498, 542},
		{650, 542}
	}
};

static int Objective_key_text_coords[GR_NUM_RESOLUTIONS][3][2] = {
	{
		// GR_640
		{195, 344},
		{306, 344},
		{432, 344}
	},
	{
		// GR_1024
		{310, 546},
		{486, 546},
		{688, 546}
	}
};
static int Objective_key_icon_coords[GR_NUM_RESOLUTIONS][3][2] = {
	{
		// GR_640
		{170, 339},
		{280, 339},
		{406, 339}
	},
	{
		// GR_1024
		{272, 542},
		{448, 542},
		{650, 542}
	}
};


#define NUM_GOAL_SCREEN_BUTTONS			3  // total number of buttons
#define GOAL_SCREEN_BUTTON_SCROLL_UP	0
#define GOAL_SCREEN_BUTTON_SCROLL_DOWN	1
#define GOAL_SCREEN_BUTTON_RETURN		2

struct goal_list {
	int count;
	mission_goal *list[MAX_GOALS_PER_LIST];
	int line_offsets[MAX_GOALS_PER_LIST];
	int line_spans[MAX_GOALS_PER_LIST];

	goal_list() : count(0) {}
	void add(mission_goal *m);
	void set();
	void icons_display(int y);
};

struct goal_buttons {
	char *filename;
	int x, y;
	int hotspot;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	goal_buttons(char *name, int x1, int y1, int h) : filename(name), x(x1), y(y1), hotspot(h) {}
};

struct goal_text {
	int m_num_lines;
	int m_line_sizes[MAX_GOAL_LINES];
	char *m_lines[MAX_GOAL_LINES];

	void init();
	int add(char *text = NULL);
	void display(int n, int y);
};

int Num_mission_events;
int Num_goals = 0;								// number of goals for this mission
int Event_index;  // used by sexp code to tell what event it came from
int Mission_goal_timestamp;

mission_event Mission_events[MAX_MISSION_EVENTS];
mission_goal Mission_goals[MAX_GOALS];		// structure for the goals of this mission
static goal_text Goal_text;

#define DIRECTIVE_SOUND_DELAY			500					// time directive success sound effect is delayed
#define DIRECTIVE_SPECIAL_DELAY		7000					// mark special directives as true after 7 seconds

static int Mission_directive_sound_timestamp;	// timestamp to control when directive succcess sound gets played
static int Mission_directive_special_timestamp;	// used to specially mark a directive as true even though it's not

char *Goal_type_text(int n)
{
	switch (n) {
		case 0:	
			return XSTR( "Primary", 396);

		case 1:
			return XSTR( "Secondary", 397);

		case 2:
			return XSTR( "Bonus", 398);
	}

	return NULL;
}

static int Goal_screen_text_x;
static int Goal_screen_text_y;
static int Goal_screen_text_w;
static int Goal_screen_text_h;
static int Goal_screen_icon_x;

static goal_list Primary_goal_list;
static goal_list Secondary_goal_list;
static goal_list Bonus_goal_list;

static int Scroll_offset;
static int Goals_screen_bg_bitmap;
static int Goal_complete_bitmap;
static int Goal_incomplete_bitmap;
static int Goal_failed_bitmap;
static UI_WINDOW Goals_screen_ui_window;

goal_buttons Goal_buttons[NUM_GOAL_SCREEN_BUTTONS] = {
//XSTR:OFF
	goal_buttons("MOB_00", 475, 288, 0),
	goal_buttons("MOB_01", 475, 336, 1),
	goal_buttons("MOB_02", 553, 409, 2),
//XSTR:ON
};

void goal_screen_button_pressed(int num);
void goal_screen_scroll_up();
void goal_screen_scroll_down();

//
/// goal_list structure functions
//

void goal_list::add(mission_goal *m)
{
	Assert(count < MAX_GOALS_PER_LIST);
	list[count++] = m;
}

void goal_list::set()
{
	int i;

	for (i=0; i<count; i++) {
		line_offsets[i] = Goal_text.m_num_lines;
		line_spans[i] = Goal_text.add(list[i]->message);
		
		if (i < count - 1)
			Goal_text.add();
	}
}

void goal_list::icons_display(int yoff)
{
	int i, y, ys, bmp, font_height;

	font_height = gr_get_font_height();
	for (i=0; i<count; i++) {
		y = line_offsets[i] - yoff;

		bmp = -1;			// initialize for safety.
		switch (list[i]->satisfied) {
			case GOAL_COMPLETE:
				bmp = Goal_complete_bitmap;
				break;

			case GOAL_INCOMPLETE:
				bmp = Goal_incomplete_bitmap;
				break;

			case GOAL_FAILED:
				bmp = Goal_failed_bitmap;
				break;
		}

		if (bmp >= 0) {
			bm_get_info(bmp, NULL, &ys, NULL);
			y = Goal_screen_text_y						// offset of text window on screen
				+ y * font_height							// relative line position offset
				+ line_spans[i] * font_height / 2	// center of text offset
				- ys / 2;									// center of icon offest

			if ((y >= Goal_screen_text_y - ys / 2) && (y + ys <= Goal_screen_text_y + Goal_screen_text_h + ys / 2)) {
				gr_set_bitmap(bmp);
				gr_bitmap(Goal_screen_icon_x, y);
			}
		}
	}
}

//
/// goal_text structure functions
//

// initializes the goal text struct (empties it out)
void goal_text::init()
{
	m_num_lines = 0;
}

// Adds lines of goal text.  If passed NULL (or nothing passed) a blank line is added.  If
// the text is too long, it is automatically split into more than one line.
// Returns the number of lines added.
int goal_text::add(char *text)
{
	int max, count;

	max = MAX_GOAL_LINES - m_num_lines;
	if (max < 1) {
		Error(LOCATION, "Goal text space exhausted");
		return 0;
	}

	if (!text) {
		m_lines[m_num_lines] = NULL;
		m_line_sizes[m_num_lines++] = 0;
		return 1;
	}

	count = split_str(text, Goal_screen_text_w - Goal_screen_text_coords[gr_screen.res][GOAL_SCREEN_X_COORD] + Goal_screen_icon_xcoord[gr_screen.res], m_line_sizes + m_num_lines, m_lines + m_num_lines, max);
	m_num_lines += count;
	return count;
}

// Display a line of goal text
//   n = goal text line number
//   y = y offset to draw relative to goal text area top
void goal_text::display(int n, int y)
{
	int y1, w, h;
	char buf[MAX_GOAL_TEXT];

	if ((n < 0) || (n >= m_num_lines) || (m_line_sizes[n] < 1))
		return;  // out of range, don't draw anything

	Assert(m_line_sizes[n] < MAX_GOAL_TEXT);
	y += Goal_screen_text_y;
	if (*m_lines[n] == '*') {  // header line
		gr_set_color_fast(&Color_text_heading);
		strncpy(buf, m_lines[n] + 1, m_line_sizes[n] - 1);
		buf[m_line_sizes[n] - 1] = 0;

		gr_get_string_size(&w, &h, buf);
		y1 = y + h / 2 - 1;

		// custom_size me
		gr_line(Goal_screen_icon_x, y1, Goal_screen_text_x - 2, y1);
		gr_line(Goal_screen_text_x + w + 1, y1, Goal_screen_icon_x + Goal_screen_text_w, y1);

	} else {
		gr_set_color_fast(&Color_text_normal);
		strncpy(buf, m_lines[n], m_line_sizes[n]);
		buf[m_line_sizes[n]] = 0;
	}

	gr_printf(Goal_screen_text_x, y, buf);
}

// mission_init_goals: initializes info for goals.  Called as part of mission initialization.
void mission_init_goals()
{
	int i;

	Num_goals = 0;
	for (i=0; i<MAX_GOALS; i++) {
		Mission_goals[i].satisfied = GOAL_INCOMPLETE;
		Mission_goals[i].flags = 0;
		Mission_goals[i].team = 0;
	}

	Num_mission_events = 0;
	for (i=0; i<MAX_MISSION_EVENTS; i++) {
		Mission_events[i].result = 0;
		Mission_events[i].flags = 0;
		Mission_events[i].count = 0;
		Mission_events[i].satisfied_time = 0;
		Mission_events[i].born_on_date = 0;
		Mission_events[i].team = -1;
	}

	Mission_goal_timestamp = timestamp(GOAL_TIMESTAMP);
	Mission_directive_sound_timestamp = 0;
	Mission_directive_special_timestamp = timestamp(-1);		// need to make invalid right away
}

// called at the end of a mission for cleanup
void mission_event_shutdown()
{
	int i;

	for (i=0; i<Num_mission_events; i++) {
		if (Mission_events[i].objective_text) {
			vm_free(Mission_events[i].objective_text);
			Mission_events[i].objective_text = NULL;
		}
		if (Mission_events[i].objective_key_text) {
			vm_free(Mission_events[i].objective_key_text);
			Mission_events[i].objective_key_text = NULL;
		}
	}
}

// called once right before entering the show goals screen to do initializations.
void mission_show_goals_init()
{
	int i, type, team_num=0;		// JAS: I set team_num to 0 because it was used without being initialized.
	goal_buttons *b;

	Scroll_offset = 0;
	Primary_goal_list.count = 0;
	Secondary_goal_list.count = 0;
	Bonus_goal_list.count = 0;

	Goal_screen_text_x = Goal_screen_text_coords[gr_screen.res][GOAL_SCREEN_X_COORD];
	Goal_screen_text_y = Goal_screen_text_coords[gr_screen.res][GOAL_SCREEN_Y_COORD];
	Goal_screen_text_w = Goal_screen_text_coords[gr_screen.res][GOAL_SCREEN_W_COORD];
	Goal_screen_text_h = Goal_screen_text_coords[gr_screen.res][GOAL_SCREEN_H_COORD];
	Goal_screen_icon_x = Goal_screen_icon_xcoord[gr_screen.res];

	// fill up the lists so we can display the goals appropriately
	for (i=0; i<Num_goals; i++) {
		if (Mission_goals[i].type & INVALID_GOAL){  // don't count invalid goals here
			continue;
		}

		if ( (Game_mode & GM_MULTIPLAYER) && (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) && (Mission_goals[i].team != team_num) ){
			continue;
		}

		type = Mission_goals[i].type & GOAL_TYPE_MASK;
		switch (type) {
		case PRIMARY_GOAL:
			Primary_goal_list.add(&Mission_goals[i]);
			break;

		case SECONDARY_GOAL:
			Secondary_goal_list.add(&Mission_goals[i]);
			break;

		case BONUS_GOAL:
			if (Mission_goals[i].satisfied == GOAL_COMPLETE){
				Bonus_goal_list.add(&Mission_goals[i]);
			}
			break;
	
		default:
			Error(LOCATION, "Unknown goal priority encountered when displaying goals in mission\n");
			break;
		} // end switch
	} // end for

	Goal_text.init();

	Goal_text.add(XSTR( "*Primary Objectives", 399));
	Goal_text.add();
	Primary_goal_list.set();

	if (Secondary_goal_list.count) {
		Goal_text.add();
		Goal_text.add();
		Goal_text.add(XSTR( "*Secondary Objectives", 400));
		Goal_text.add();
		Secondary_goal_list.set();
	}

	if (Bonus_goal_list.count) {
		Goal_text.add();
		Goal_text.add();
		Goal_text.add(XSTR( "*Bonus Objectives", 401));
		Goal_text.add();
		Bonus_goal_list.set();
	}

	common_set_interface_palette("ObjectivesPalette");  // set the interface palette
	Goals_screen_ui_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Goals_screen_ui_window.set_mask_bmap("Objectives-m");

	for (i=0; i<NUM_GOAL_SCREEN_BUTTONS; i++) {
		b = &Goal_buttons[i];

		b->button.create(&Goals_screen_ui_window, "", b->x, b->y, 60, 30, (i < 2), 1);
		// set up callback for when a mouse first goes over a button
		b->button.set_highlight_action(common_play_highlight_sound);
		b->button.set_bmaps(b->filename);
		b->button.link_hotspot(b->hotspot);
	}

	// set up hotkeys for buttons so we draw the correct animation frame when a key is pressed
	Goal_buttons[GOAL_SCREEN_BUTTON_SCROLL_UP].button.set_hotkey(KEY_UP);
	Goal_buttons[GOAL_SCREEN_BUTTON_SCROLL_DOWN].button.set_hotkey(KEY_DOWN);

	Goals_screen_bg_bitmap = bm_load("ObjectivesBG");
	Goal_complete_bitmap = bm_load("ObjComp");
	Goal_incomplete_bitmap = bm_load("ObjIncomp");
	Goal_failed_bitmap = bm_load("ObjFail");

	if (Goals_screen_bg_bitmap < 0) {
		Warning(LOCATION, "Could not load the background bitmap: ObjectivesBG.pcx");
	}
}

// cleanup called when exiting the show goals screen
void mission_show_goals_close()
{
	if (Goals_screen_bg_bitmap >= 0)
		bm_release(Goals_screen_bg_bitmap);

	if (Goal_complete_bitmap)
		bm_release(Goal_complete_bitmap);
	
	if (Goal_incomplete_bitmap)
		bm_release(Goal_incomplete_bitmap);
	
	if (Goal_failed_bitmap)
		bm_release(Goal_failed_bitmap);

	Goals_screen_ui_window.destroy();
	common_free_interface_palette();		// restore game palette
	game_flush();
}
	
// called once a frame during show goals state to process events and render the screen
void mission_show_goals_do_frame(float frametime)
{
	int k, i, y, z;
	int font_height = gr_get_font_height();
	
	k = Goals_screen_ui_window.process();
	switch (k) {
		case KEY_ESC:
			mission_goal_exit();			
			break;
		
		case KEY_DOWN:
			goal_screen_scroll_down();
			break;

		case KEY_UP:
			goal_screen_scroll_up();
			break;

		default:
			// do nothing
			break;
	}	// end switch

	for (i=0; i<NUM_GOAL_SCREEN_BUTTONS; i++){
		if (Goal_buttons[i].button.pressed()){
			goal_screen_button_pressed(i);
		}
	}

	GR_MAYBE_CLEAR_RES(Goals_screen_bg_bitmap);
	if (Goals_screen_bg_bitmap >= 0) {
		gr_set_bitmap(Goals_screen_bg_bitmap);
		gr_bitmap(0, 0);
	}
	Goals_screen_ui_window.draw();

	y = 0;
	z = Scroll_offset;
	while (y + font_height <= Goal_screen_text_h) {
		Goal_text.display(z, y);
		y += font_height;
		z++;
	}

	Primary_goal_list.icons_display(Scroll_offset);
	Secondary_goal_list.icons_display(Scroll_offset);
	Bonus_goal_list.icons_display(Scroll_offset);

	gr_flip();
}

// Mission Log Objectives subscreen init.
// Called once right before entering the Mission Log screen to do initializations.
int ML_objectives_init(int x, int y, int w, int h)
{
	int i, type, team_num;

	Primary_goal_list.count = 0;
	Secondary_goal_list.count = 0;
	Bonus_goal_list.count = 0;

	Goal_screen_text_x = x - Goal_screen_icon_xcoord[gr_screen.res] + Goal_screen_text_coords[gr_screen.res][GOAL_SCREEN_X_COORD];
	Goal_screen_text_y = y;
	Goal_screen_text_w = w;
	Goal_screen_text_h = h;
	Goal_screen_icon_x = x;

	team_num = 0;			// this is the default team -- we will change it if in a multiplayer team v. team game

	if ( (Game_mode & GM_MULTIPLAYER) && (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) ){
		team_num = Net_player->p_info.team;
	}

	// fill up the lists so we can display the goals appropriately
	for (i=0; i<Num_goals; i++) {
		if (Mission_goals[i].type & INVALID_GOAL){  // don't count invalid goals here
			continue;
		}

		if ( (Game_mode & GM_MULTIPLAYER) && (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) && (Mission_goals[i].team != team_num) ){
			continue;
		}

		type = Mission_goals[i].type & GOAL_TYPE_MASK;
		switch (type) {
			case PRIMARY_GOAL:
				Primary_goal_list.add(&Mission_goals[i]);
				break;

			case SECONDARY_GOAL:
				Secondary_goal_list.add(&Mission_goals[i]);
				break;

			case BONUS_GOAL:
				if (Mission_goals[i].satisfied == GOAL_COMPLETE){
					Bonus_goal_list.add(&Mission_goals[i]);
				}
				break;
	
			default:
				Error(LOCATION, "Unknown goal priority encountered when displaying goals in mission\n");
				break;
		} // end switch
	} // end for

	Goal_text.init();

	Goal_text.add(XSTR( "*Primary Objectives", 399));
	Goal_text.add();
	Primary_goal_list.set();

	if (Secondary_goal_list.count) {
		Goal_text.add();
		Goal_text.add();
		Goal_text.add(XSTR( "*Secondary Objectives", 400));
		Goal_text.add();
		Secondary_goal_list.set();
	}

	if (Bonus_goal_list.count) {
		Goal_text.add();
		Goal_text.add();
		Goal_text.add(XSTR( "*Bonus Objectives", 401));
		Goal_text.add();
		Bonus_goal_list.set();
	}


	Goal_complete_bitmap = bm_load("ObjComp");
	Goal_incomplete_bitmap = bm_load("ObjIncomp");
	Goal_failed_bitmap = bm_load("ObjFail");

	return Goal_text.m_num_lines;
}

// cleanup called when exiting the show goals screen
void ML_objectives_close()
{
	if (Goal_complete_bitmap >= 0) {
		bm_release(Goal_complete_bitmap);
	}
	
	if (Goal_incomplete_bitmap >= 0) {
		bm_release(Goal_incomplete_bitmap);
	}
	
	if (Goal_failed_bitmap >= 0) {
		bm_release(Goal_failed_bitmap);
	}
}

void ML_objectives_do_frame(int scroll_offset)
{
	int y, z;
	int font_height = gr_get_font_height();

	y = 0;
	z = scroll_offset;
	while (y + font_height <= Goal_screen_text_h) {
		Goal_text.display(z, y);
		y += font_height;
		z++;
	}

	Primary_goal_list.icons_display(scroll_offset);
	Secondary_goal_list.icons_display(scroll_offset);
	Bonus_goal_list.icons_display(scroll_offset);
}

void ML_render_objectives_key()
{
	// display icon key at the bottom
	if (Lcl_gr) {
		gr_set_bitmap(Goal_complete_bitmap);
		gr_bitmap(Objective_key_icon_coords_gr[gr_screen.res][0][0], Objective_key_icon_coords_gr[gr_screen.res][0][1]);
		gr_set_bitmap(Goal_incomplete_bitmap);
		gr_bitmap(Objective_key_icon_coords_gr[gr_screen.res][1][0], Objective_key_icon_coords_gr[gr_screen.res][1][1]);
		gr_set_bitmap(Goal_failed_bitmap);
		gr_bitmap(Objective_key_icon_coords_gr[gr_screen.res][2][0], Objective_key_icon_coords_gr[gr_screen.res][2][1]);

		gr_string(Objective_key_text_coords_gr[gr_screen.res][0][0], Objective_key_text_coords_gr[gr_screen.res][0][1] , XSTR("Complete",	1437));
		gr_string(Objective_key_text_coords_gr[gr_screen.res][1][0], Objective_key_text_coords_gr[gr_screen.res][1][1] , XSTR("Incomplete", 1438));
		gr_string(Objective_key_text_coords_gr[gr_screen.res][2][0], Objective_key_text_coords_gr[gr_screen.res][2][1] , XSTR("Failed",		1439));
	} else {
		gr_set_bitmap(Goal_complete_bitmap);
		gr_bitmap(Objective_key_icon_coords[gr_screen.res][0][0], Objective_key_icon_coords[gr_screen.res][0][1]);
		gr_set_bitmap(Goal_incomplete_bitmap);
		gr_bitmap(Objective_key_icon_coords[gr_screen.res][1][0], Objective_key_icon_coords[gr_screen.res][1][1]);
		gr_set_bitmap(Goal_failed_bitmap);
		gr_bitmap(Objective_key_icon_coords[gr_screen.res][2][0], Objective_key_icon_coords[gr_screen.res][2][1]);

		gr_string(Objective_key_text_coords[gr_screen.res][0][0], Objective_key_text_coords[gr_screen.res][0][1] , XSTR("Complete",	1437));
		gr_string(Objective_key_text_coords[gr_screen.res][1][0], Objective_key_text_coords[gr_screen.res][1][1] , XSTR("Incomplete", 1438));
		gr_string(Objective_key_text_coords[gr_screen.res][2][0], Objective_key_text_coords[gr_screen.res][2][1] , XSTR("Failed",		1439));
	}
}

//  maybe distribute the event/goal score between the players on that team
void multi_player_maybe_add_score (int score, int team)
{
	int players_in_team = 0; 
	int idx; 

	// if i'm not the server of a multiplayer game, bail here
	if(!MULTIPLAYER_MASTER){
		return;
	}

	if (!(The_mission.ai_profile->flags & AIPF_ALLOW_MULTI_EVENT_SCORING)) {
		return;
	}				

	// first count the number of players in this team
	for (idx=0; idx<MAX_PLAYERS; idx++) {
		if (MULTI_CONNECTED(Net_players[idx]) && (Net_players[idx].p_info.team == team)) {
			players_in_team++;
		}
	}

	if (!players_in_team) {
		return;
	}

	for (idx=0; idx<MAX_PLAYERS; idx++) {
		if (MULTI_CONNECTED(Net_players[idx]) && (Net_players[idx].p_info.team == team)) {
			Net_players[idx].m_player->stats.m_score += (int)(score * scoring_get_scale_factor() / players_in_team );
		}
	}
}

/**
 * Hook for temporarily displaying objective completion/failure
 */
void mission_goal_status_change( int goal_num, int new_status)
{
	int type;

	Assert(goal_num < Num_goals);
	Assert((new_status == GOAL_FAILED) || (new_status == GOAL_COMPLETE));

	// if in a multiplayer game, send a status change to clients
	if ( MULTIPLAYER_MASTER ){
		send_mission_goal_info_packet( goal_num, new_status, -1 );
	}

	type = Mission_goals[goal_num].type & GOAL_TYPE_MASK;
	Mission_goals[goal_num].satisfied = new_status;
	if ( new_status == GOAL_FAILED ) {
		// don't display bonus goal failure
		if ( type != BONUS_GOAL ) {

			// only do HUD and music is goals are my teams goals.
			if ( (Game_mode & GM_NORMAL) || ((Net_player != NULL) && (Net_player->p_info.team == Mission_goals[goal_num].team)) ) {
				hud_add_objective_messsage(type, new_status);
				if ( !( Mission_goals[goal_num].flags & MGF_NO_MUSIC ) ) {	// maybe play event music
					event_music_primary_goal_failed();
				}
			}
		}
		mission_log_add_entry( LOG_GOAL_FAILED, Mission_goals[goal_num].name, NULL, goal_num );
	} else if ( new_status == GOAL_COMPLETE ) {
		if ( (Game_mode & GM_NORMAL) || ((Net_player != NULL) && (Net_player->p_info.team == Mission_goals[goal_num].team))) {
			hud_add_objective_messsage(type, new_status);
			// cue for Event Music
			if ( !(Mission_goals[goal_num].flags & MGF_NO_MUSIC) ) {
				event_music_primary_goals_met();
			}			
			mission_log_add_entry( LOG_GOAL_SATISFIED, Mission_goals[goal_num].name, NULL, goal_num );
		}	
		
		if(Game_mode & GM_MULTIPLAYER){
			multi_team_maybe_add_score(Mission_goals[goal_num].score, Mission_goals[goal_num].team);	
			multi_player_maybe_add_score(Mission_goals[goal_num].score, Mission_goals[goal_num].team);
		} else {
			// deal with the score
			Player->stats.m_score += (int)(Mission_goals[goal_num].score * scoring_get_scale_factor());			
		}
	}
}

// return value:
//   EVENT_UNBORN    = event has yet to be available (not yet evaluatable)
//   EVENT_CURRENT   = current (evaluatable), but not yet true
//   EVENT_SATISFIED = event has occured (true)
//   EVENT_FAILED    = event failed, can't possibly become true anymore
int mission_get_event_status(int event)
{
	// check for directive special events first.  We will always return from this part of the if statement
	if ( Mission_events[event].flags & MEF_DIRECTIVE_SPECIAL ) {

		// if this event is temporarily true, return as such
		if ( Mission_events[event].flags & MEF_DIRECTIVE_TEMP_TRUE ){
			return EVENT_SATISFIED;
		}

		// if the timestamp has elapsed, we can "mark" this directive as true although it's really not.
		if ( timestamp_elapsed(Mission_directive_special_timestamp) ) {
			Mission_events[event].satisfied_time = Missiontime;
			Mission_events[event].flags |= MEF_DIRECTIVE_TEMP_TRUE;
		}

		return EVENT_CURRENT;
	} else if (Mission_events[event].flags & MEF_CURRENT) {
		if (!Mission_events[event].born_on_date){
			Mission_events[event].born_on_date = timestamp();
		}

		if (Mission_events[event].result) {
			return EVENT_SATISFIED;
		}

		if (Mission_events[event].formula < 0) {
			return EVENT_FAILED;
		}

		return EVENT_CURRENT;
	}

	return EVENT_UNBORN;
}

void mission_event_set_directive_special(int event)
{
	// bogus
	if((event < 0) || (event >= Num_mission_events)){
		return;
	}

	Mission_events[event].flags |= MEF_DIRECTIVE_SPECIAL;

	// start from a known state
	Mission_events[event].flags &= ~MEF_DIRECTIVE_TEMP_TRUE;
	Mission_directive_special_timestamp = timestamp(DIRECTIVE_SPECIAL_DELAY);
}

void mission_event_unset_directive_special(int event)
{
	// bogus
	if((event < 0) || (event >= Num_mission_events)){
		return;
	}

	Mission_events[event].flags &= ~(MEF_DIRECTIVE_SPECIAL);

	// this event may be marked temporarily true -- if so, then unmark this value!!!
	if ( Mission_events[event].flags & MEF_DIRECTIVE_TEMP_TRUE ){
		Mission_events[event].flags &= ~MEF_DIRECTIVE_TEMP_TRUE;
	}
	Mission_events[event].satisfied_time = 0;
	Mission_directive_special_timestamp = timestamp(-1);
}

// function which evaluates and processes the given event
void mission_process_event( int event )
{
	int store_flags = Mission_events[event].flags;
	int store_formula = Mission_events[event].formula;
	int store_result = Mission_events[event].result;
	int store_count = Mission_events[event].count;

	int result, sindex;
	bool bump_timestamp = false; 

	Directive_count = 0;
	Event_index = event;
	sindex = Mission_events[event].formula;
	result = Mission_events[event].result;

	// if chained, insure that previous event is true and next event is false
	if (Mission_events[event].chain_delay >= 0) {  // this indicates it's chained
		// What everyone expected the chaining behavior to be, as specified in Karajorma's original fix to Mantis #82
		if (Alternate_chaining_behavior) {
			if (event > 0){
				if (!Mission_events[event - 1].result || ((fix) Mission_events[event - 1].satisfied_time + i2f(Mission_events[event].chain_delay) > Missiontime)){
					sindex = -1;  // bypass evaluation
				}
			}
		}
		// Volition's original chaining behavior as used in retail and demonstrated in e.g. btm-01.fsm (or btm-01.fs2 in the Port)
		else {
			if (event > 0){
				if (!Mission_events[event - 1].result || ((fix) Mission_events[event - 1].timestamp + i2f(Mission_events[event].chain_delay) > Missiontime)){
					sindex = -1;  // bypass evaluation
				}
			}

			if ((event < Num_mission_events - 1) && Mission_events[event + 1].result && (Mission_events[event + 1].chain_delay >= 0)){
				sindex = -1;  // bypass evaluation
			}
		}
	}

	if (sindex >= 0) {
		Sexp_useful_number = 1;
		result = eval_sexp(sindex);

		// if the directive count is a special value, deal with that first.  Mark the event as a special
		// event, and unmark it when the directive is true again.
		if ( (Directive_count == DIRECTIVE_WING_ZERO) && !(Mission_events[event].flags & MEF_DIRECTIVE_SPECIAL) ) {			
			// make it special - which basically just means that its true until the next wave arrives
			mission_event_set_directive_special(event);

			Directive_count = 0;
		} else if ( (Mission_events[event].flags & MEF_DIRECTIVE_SPECIAL) && Directive_count > 1 ) {			
			// make it non special
			mission_event_unset_directive_special(event);
		}

		if (Mission_events[event].count || (Directive_count > 1)){
			Mission_events[event].count = Directive_count;
		}

		if (Sexp_useful_number){
			Mission_events[event].flags |= MEF_CURRENT;
		}
	}

	Event_index = 0;
	Mission_events[event].result = result;

	// if the sexpression is known false, then no need to evaluate anymore
	if ((sindex >= 0) && (Sexp_nodes[sindex].value == SEXP_KNOWN_FALSE)) {
		Mission_events[event].timestamp = (int) Missiontime;
		Mission_events[event].satisfied_time = Missiontime;
		// _argv[-1] - repeat_count of -1 would mean repeat indefinitely, so set to 0 instead.
		Mission_events[event].repeat_count = 0;
		Mission_events[event].formula = -1;
		return;
	}

	if (result && !Mission_events[event].satisfied_time) {
		Mission_events[event].satisfied_time = Missiontime;
		if ( Mission_events[event].objective_text ) {
			Mission_directive_sound_timestamp = timestamp(DIRECTIVE_SOUND_DELAY);
		}
	}

	// decrement the trigger count.  When at 0, set the repeat count to 0 so we don't eval this function anymore
	if (result && (Mission_events[event].trigger_count > 0) && (Mission_events[event].flags & MEF_USING_TRIGGER_COUNT) ) {
		Mission_events[event].trigger_count--;
		if (Mission_events[event].trigger_count == 0) {
			 Mission_events[event].repeat_count = 0; 
		}
		else {
			bump_timestamp = true;
		}
	}

	// decrement the repeat count.  When at 0, don't eval this function anymore
	if ( result || timestamp_valid(Mission_events[event].timestamp) ) {
		// _argv[-1] - negative repeat count means repeat indefinitely.
		if ( Mission_events[event].repeat_count > 0 )
			Mission_events[event].repeat_count--;
		if ( Mission_events[event].repeat_count == 0 ) {
			Mission_events[event].timestamp = (int)Missiontime;
			Mission_events[event].formula = -1;

			if(Game_mode & GM_MULTIPLAYER){
				// multiplayer missions (scoring is scaled in the multi_team_maybe_add_score() function)
				multi_team_maybe_add_score(Mission_events[event].score, Mission_events[event].team);
				multi_player_maybe_add_score(Mission_events[event].score, Mission_events[event].team);
			} else {
				// deal with the player's score
				Player->stats.m_score += (int)(Mission_events[event].score * scoring_get_scale_factor());			
			}
		}
		// Set the timestamp for the next check on this event unless we only have a trigger count and no repeat count and 
		// this event didn't trigger this frame. 
		else if (bump_timestamp || (!( Mission_events[event].repeat_count == -1 && Mission_events[event].trigger_count > 0 ))) {
			// set the timestamp to time out 'interval' seconds in the future.  
			Mission_events[event].timestamp = timestamp( Mission_events[event].interval * 1000 );
		}
	}

	// see if anything has changed	
	if(MULTIPLAYER_MASTER && ((store_flags != Mission_events[event].flags) || (store_formula != Mission_events[event].formula) || (store_result != Mission_events[event].result) || (store_count != Mission_events[event].count)) ){
		send_event_update_packet(event);
	}
}

// Maybe play a directive success sound... need to poll since the sound is delayed from when
// the directive is actually satisfied.
void mission_maybe_play_directive_success_sound()
{
	if ( timestamp_elapsed(Mission_directive_sound_timestamp) ) {
		Mission_directive_sound_timestamp=0;
		snd_play( &Snds[SND_DIRECTIVE_COMPLETE] );
	}
}

void mission_eval_goals()
{
	int i, result;

	// before checking whether or not we should evaluate goals, we should run through the events and
	// process any whose timestamp is valid and has expired.  This would catch repeating events only
	for (i=0; i<Num_mission_events; i++) {
		if (Mission_events[i].formula != -1) {
			if ( !timestamp_valid(Mission_events[i].timestamp) || !timestamp_elapsed(Mission_events[i].timestamp) ){
				continue;
			}

			// if we get here, then the timestamp on the event has popped -- we should reevaluate
			mission_process_event(i);
		}
	}
	
	if ( !timestamp_elapsed(Mission_goal_timestamp) ){
		return;
	}

	// first evaluate the players goals
	for (i=0; i<Num_goals; i++) {
		// don't evaluate invalid goals
		if (Mission_goals[i].type & INVALID_GOAL){
			continue;
		}

		if (Mission_goals[i].satisfied == GOAL_INCOMPLETE) {
			result = eval_sexp(Mission_goals[i].formula);
			if ( Sexp_nodes[Mission_goals[i].formula].value == SEXP_KNOWN_FALSE ) {
				mission_goal_status_change( i, GOAL_FAILED );

			} else if (result) {
				mission_goal_status_change(i, GOAL_COMPLETE );
			} // end if result

		}	// end if goals[i].satsified != GOAL_COMPLETE
	} // end for

	// now evaluate any mission events
	for (i=0; i<Num_mission_events; i++) {
		if ( Mission_events[i].formula != -1 ) {
			// only evaluate this event if the timestamp is not valid.  We do this since
			// we will evaluate repeatable events at the top of the file so we can get
			// the exact interval that the designer asked for.
			if ( !timestamp_valid( Mission_events[i].timestamp) ){
				mission_process_event( i );
			}
		}
	}

	// send and remaining sexp data to the clients
	if (MULTIPLAYER_MASTER) {
		multi_sexp_flush_packet();
	}

	if (The_mission.game_type & MISSION_TYPE_TRAINING){
		Mission_goal_timestamp = timestamp(GOAL_TIMESTAMP_TRAINING);
	} else {
		Mission_goal_timestamp = timestamp(GOAL_TIMESTAMP);
	}

	if ( !hud_disabled() && hud_gauge_active(HUD_DIRECTIVES_VIEW) ) {
		mission_maybe_play_directive_success_sound();
	}

   // update goal status if playing on a multiplayer standalone server
	if (Game_mode & GM_STANDALONE_SERVER){
		std_multi_update_goals();
	}
}

//	evaluate_primary_goals() will determine if the primary goals for a mission are complete
//
//	returns 1 - all primary goals are all complete or imcomplete (or there are no primary goals at all)
// returns 0 - not all primary goals are complete
int mission_evaluate_primary_goals()
{
	int i, primary_goals_complete = PRIMARY_GOALS_COMPLETE;

	for (i=0; i<Num_goals; i++) {

		if ( Mission_goals[i].type & INVALID_GOAL ) {
			continue;
		}

		if ( (Mission_goals[i].type & GOAL_TYPE_MASK) == PRIMARY_GOAL ) {
			if ( Mission_goals[i].satisfied == GOAL_INCOMPLETE ) {
				return PRIMARY_GOALS_INCOMPLETE;
			} else if ( Mission_goals[i].satisfied == GOAL_FAILED ) {
				primary_goals_complete = PRIMARY_GOALS_FAILED;
			}
		}
	}	// end for

	return primary_goals_complete;
}

// return 1 if all primary/secondary goals are complete... otherwise return 0
int mission_goals_met()
{
	int i, all_goals_met = 1;

	for (i=0; i<Num_goals; i++) {

		if ( Mission_goals[i].type & INVALID_GOAL ) {
			continue;
		}

		if ( ((Mission_goals[i].type & GOAL_TYPE_MASK) == PRIMARY_GOAL) || ((Mission_goals[i].type & GOAL_TYPE_MASK) == SECONDARY_GOAL) ) {
			if ( Mission_goals[i].satisfied == GOAL_INCOMPLETE ) {
				all_goals_met = 0;
				break;
			} else if ( Mission_goals[i].satisfied == GOAL_FAILED ) {
				all_goals_met = 0;
				break;
			}
		}
	}	// end for

	return all_goals_met;
}

// function used to actually change the status (valid/invalid) of a goal.  Called externally
// with multiplayer code
void mission_goal_validation_change( int goal_num, int valid )
{
	// only incomplete goals can have their status changed
	if ( Mission_goals[goal_num].satisfied != GOAL_INCOMPLETE ){
		return;
	}

	// if in multiplayer, then send a packet
	if ( MULTIPLAYER_MASTER ){
		send_mission_goal_info_packet( goal_num, -1, valid );
	}

	// change the valid status
	if ( valid ){
		Mission_goals[goal_num].type &= ~INVALID_GOAL;
	} else {
		Mission_goals[goal_num].type |= INVALID_GOAL;
	}
}

	// the following function marks a goal invalid.  It can only mark the goal invalid if the goal
// is not complete.  The name passed into this funciton should match the name field in the goal
// structure
void mission_goal_mark_invalid( char *name )
{
	int i;

	for (i=0; i<Num_goals; i++) {
		if ( !stricmp(Mission_goals[i].name, name) ) {
			mission_goal_validation_change( i, 0 );
			return;
		}
	}
}

// the next function marks a goal as valid.  A goal may always be marked valid.
void mission_goal_mark_valid( char *name )
{
	int i;

	for (i=0; i<Num_goals; i++) {
		if ( !stricmp(Mission_goals[i].name, name) ) {
			mission_goal_validation_change( i, 1 );
			return;
		}
	}
}

// function to fail all mission goals.  Don't fail events here since this funciton is currently
// called in mission when something bad happens to the player (like he makes too many shots on friendlies).
// Events can still happen.  Things will just be really bad for the player.
void mission_goal_fail_all()
{
	int i;

	for (i=0; i<Num_goals; i++) {
		Mission_goals[i].satisfied = GOAL_FAILED;
		mission_log_add_entry( LOG_GOAL_FAILED, Mission_goals[i].name, NULL, i );
	}
}

// function to mark all incomplete goals as failed.  Happens at the end of a mission
// mark the events which are not currently satisfied as failed as well.
void mission_goal_fail_incomplete()
{
	int i;

	for (i = 0; i < Num_goals; i++ ) {
		if ( Mission_goals[i].satisfied == GOAL_INCOMPLETE ) {
			Mission_goals[i].satisfied = GOAL_FAILED;
			mission_log_add_entry( LOG_GOAL_FAILED, Mission_goals[i].name, NULL, i );
		}
	}

	// now for the events.  Must set the formula to -1 and the result to 0 to be a failed
	// event.
	for ( i = 0; i < Num_mission_events; i++ ) {
		if ( Mission_events[i].formula != -1 ) {
			Mission_events[i].formula = -1;
			Mission_events[i].result = 0;
		}
	}
}

// small function used to mark all objectives as true.  Used as a debug function and as a way
// to skip past training misisons
void mission_goal_mark_objectives_complete()
{
	int i;

	for (i = 0; i < Num_goals; i++ ) {
		Mission_goals[i].satisfied = GOAL_COMPLETE;
	}
}

// small function used to mark all events as completed.  Used in the skipping of missions.
void mission_goal_mark_events_complete()
{
	int i;

	for (i = 0; i < Num_mission_events; i++ ) {
		Mission_events[i].result = 1;
		Mission_events[i].formula = -1;
	}
}

// some debug console functions to help list and change the status of mission goals
DCF(show_mission_goals,"List and change the status of mission goals")
{
	int i, type;

	if (Dc_command)
		Dc_status = 1;

	if (Dc_help) {
		dc_printf("Usage: show_mission_goals\n\nList all mission goals and their current status.\n");
		Dc_status = 0;
	}

	if (Dc_status) {
		for (i=0; i<Num_goals; i++) {
			type = Mission_goals[i].type & GOAL_TYPE_MASK;
			dc_printf("%2d. %32s(%10s) -- ", i, Mission_goals[i].name, Goal_type_text(type));
			if ( Mission_goals[i].satisfied == GOAL_COMPLETE )
				dc_printf("satisfied.\n");
			else if ( Mission_goals[i].satisfied == GOAL_INCOMPLETE )
				dc_printf("not satisfied\n");
			else if ( Mission_goals[i].satisfied == GOAL_FAILED )
				dc_printf("failed\n");
			else
				dc_printf("\t[unknown goal status].\n");
		}
	}
}

//XSTR:OFF
DCF(change_mission_goal, "Change the mission goal")
{
	int num;

	if ( Dc_command ) {
		dc_get_arg(ARG_INT);
		if ( Dc_arg_int >= Num_goals ) {
			dc_printf ("First parameter must be a valid goal number.\n");
			return;
		}

		num = Dc_arg_int;
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_STRING);
		if ( Dc_arg_type & ARG_TRUE )
			Mission_goals[num].satisfied = GOAL_COMPLETE;
		else if ( Dc_arg_type & ARG_FALSE )
			Mission_goals[num].satisfied = GOAL_FAILED;
		else if ( Dc_arg_type & ARG_NONE )
			Mission_goals[num].satisfied = GOAL_INCOMPLETE;
		else if ( Dc_arg_type & ARG_STRING) {
			if ( !stricmp(Dc_arg, "satisfied") )
				Mission_goals[num].satisfied = GOAL_COMPLETE;
			else if ( !stricmp( Dc_arg, "failed") )
				Mission_goals[num].satisfied = GOAL_FAILED;
			else if ( !stricmp( Dc_arg, "unknown") )
				Mission_goals[num].satisfied = GOAL_INCOMPLETE;
			else
				dc_printf("Unknown status %s.  Use 'satisfied', 'failed', or 'unknown'\n", Dc_arg);
		}
	}

	if ( Dc_help ) {
		dc_printf("Usage: change_mission_goal <goal_num> <status>\n");
		dc_printf("<goal_num> --  Integer number of goal to change.  See show_mission_goals\n");
		dc_printf("<status>   --  [bool] where a true value makes the goal satisfied,\n");
		dc_printf("               a false value makes the goal failed.\n");
		dc_printf("The <status> field may also be one of 'satisfied', 'failed', or 'unknown'\n");
		dc_printf("\nExamples:\n\n'change_mission_goal 1 true' makes goal 1 successful.\n");
		dc_printf("'change_mission_goal 2' marks goal 2 not complete\n");
		dc_printf("'change_mission_goal 0 satisfied' marks goal 0 as satisfied\n");
		Dc_status = 0;
	}
}
//XSTR:ON

// debug functions to mark all primary/secondary/bonus goals as true
#ifndef DEBUG

void mission_goal_mark_all_true(int type)
{
	int i;

	for (i = 0; i < Num_goals; i++ ) {
		if ( (Mission_goals[i].type & GOAL_TYPE_MASK) == type )
			Mission_goals[i].satisfied = GOAL_COMPLETE;
	}

	HUD_sourced_printf(HUD_SOURCE_HIDDEN, NOX("All %s goals marked true"), Goal_type_text(type) );
}

#endif

void goal_screen_button_pressed(int num)
{
	switch (num) {
	case GOAL_SCREEN_BUTTON_SCROLL_UP:
		goal_screen_scroll_up();
		break;

	case GOAL_SCREEN_BUTTON_SCROLL_DOWN:
		goal_screen_scroll_down();
		break;

	case GOAL_SCREEN_BUTTON_RETURN:
		mission_goal_exit();
		break;
	}
}

void goal_screen_scroll_up()
{
	if (Scroll_offset) {
		Scroll_offset--;
		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

void goal_screen_scroll_down()
{
	int max_lines;

	max_lines = Goal_screen_text_h / gr_get_font_height();
	if (Scroll_offset + max_lines < Goal_text.m_num_lines) {
		Scroll_offset++;
		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

// Return the number of resolved goals in num_resolved, return the total
// number of valid goals in total
void mission_goal_fetch_num_resolved(int desired_type, int *num_resolved, int *total, int team)
{
	int i,type;

	*num_resolved=0;
	*total=0;

	for (i=0; i<Num_goals; i++) {
		// if we're checking for team
		if((team >= 0) && (Mission_goals[i].team != team)){
			continue;
		}

		if (Mission_goals[i].type & INVALID_GOAL) {
			continue;
		}

		type = Mission_goals[i].type & GOAL_TYPE_MASK;
		if ( type != desired_type ) {
			continue;
		}

		*total = *total + 1;

		if (Mission_goals[i].satisfied != GOAL_INCOMPLETE) {
			*num_resolved = *num_resolved + 1;
		}
	}
}

// Return whether there are any incomplete goals of the specified type
int mission_goals_incomplete(int desired_type, int team)
{
	int i, type;

	for (i=0; i<Num_goals; i++) {		
		// if we're checking for team
		if((team >= 0) && (Mission_goals[i].team != team)){
			continue;
		}

		if (Mission_goals[i].type & INVALID_GOAL) {
			continue;
		}

		type = Mission_goals[i].type & GOAL_TYPE_MASK;
		if ( type != desired_type ) {
			continue;
		}

		if (Mission_goals[i].satisfied == GOAL_INCOMPLETE) {
			return 1;
		}
	}

	return 0;
}

void mission_goal_exit()
{
	snd_play( &Snds_iface[SND_USER_SELECT] );
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}
