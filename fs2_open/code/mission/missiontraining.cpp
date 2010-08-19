/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "mission/missiontraining.h"
#include "parse/parselo.h"
#include "sound/sound.h"
#include "sound/audiostr.h"
#include "mission/missionmessage.h"
#include "mission/missiongoals.h"
#include "mission/missionparse.h"
#include "io/timer.h"
#include "hud/hudmessage.h"
#include "hud/hud.h"
#include "cfile/cfile.h"
#include "playerman/player.h"
#include "popup/popup.h"
#include "gamesequence/gamesequence.h"
#include "weapon/emp.h"
#include "globalincs/alphacolors.h"
#include "ship/ship.h"
#include "parse/sexp.h"
#include "network/multi.h"



#define MAX_TRAINING_MESSAGE_LINES		10
//#define TRAINING_MESSAGE_WINDOW_X			174
//#define TRAINING_MESSAGE_WINDOW_Y			40
#define TRAINING_MESSAGE_WINDOW_WIDTH	266
#define TRAINING_LINE_WIDTH			250  // width in pixels of actual text
#define TRAINING_TIMING					150  // milliseconds per character to display messages
#define TRAINING_TIMING_BASE			1000  // Minimum milliseconds to display any message
//#define TRAINING_OBJ_WND_X				0		// offset of left edge of window
//#define TRAINING_OBJ_WND_Y				180	// offset of top edge of window
//#define TRAINING_OBJ_WND_Y				187	// offset of top edge of window
#define TRAINING_OBJ_WND_WIDTH		170	// number of pixels wide window is.
#define TRAINING_OBJ_LINE_WIDTH		150	// number of pixels wide text can be
#define TRAINING_OBJ_LINES				50		// number of lines to track in objective list
#define TRAINING_OBJ_DISPLAY_LINES	5		// only display this many lines on screen max
#define MAX_TRAINING_MESSAGE_MODS			20
#define TRAINING_MESSAGE_QUEUE_MAX			40

#define TRAINING_OBJ_STATUS_UNKNOWN		(1 << 28)	// directive status is unknown
#define TRAINING_OBJ_STATUS_KNOWN		(1 << 29)	// directive status is known (satisfied or failed)
#define TRAINING_OBJ_LINES_KEY			(1 << 30)	// flag indicating line describes the key associated with objective
#define TRAINING_OBJ_LINES_EVENT_STATUS_MASK (TRAINING_OBJ_STATUS_KNOWN | TRAINING_OBJ_STATUS_UNKNOWN)

#define TRAINING_OBJ_LINES_MASK(n)	(Training_obj_lines[n] & 0xffff)

#define TMMOD_NORMAL	0
#define TMMOD_BOLD	1

typedef struct {
	char *pos;
	int mode;  // what function to perform at given position (TMMOD_*)
} training_message_mods;  // training message modifiers

typedef struct {
	int num;
	int timestamp;
	int length;
	char *special_message;
} training_message_queue;

char Training_buf[TRAINING_MESSAGE_LENGTH];
char *Training_lines[MAX_TRAINING_MESSAGE_LINES];  // Training message split into lines
char Training_voice_filename[NAME_LENGTH];
int Max_directives = TRAINING_OBJ_DISPLAY_LINES;
int Training_message_timestamp;
int Training_line_sizes[MAX_TRAINING_MESSAGE_LINES];
int Training_message_method = 1;
int Training_num_lines = 0;
int Training_voice = -1;
int Training_voice_type;
int Training_voice_handle;
int Training_flag = 0;
int Training_failure = 0;
int Training_message_queue_count = 0;
int Training_bind_warning = -1;  // Missiontime at which we last gave warning
int Training_message_visible;
training_message_queue Training_message_queue[TRAINING_MESSAGE_QUEUE_MAX];

// coordinates for training messages
int Training_message_window_coords[GR_NUM_RESOLUTIONS][2] = {
	{ 174, 40 },
	{ 379, 125 }
};

// coordinates for "directives" window on hud
int Training_obj_window_coords[GR_NUM_RESOLUTIONS][2] = {
	{ 0, 187 },
	{ 0, 287 }
};


// training objectives global vars.
int Training_obj_num_lines;
int Training_obj_lines[TRAINING_OBJ_LINES];
training_message_mods Training_message_mods[MAX_TRAINING_MESSAGE_MODS];

// local module prototypes
void training_process_message(char *message);
void message_translate_tokens(char *buf, char *text);


#define NUM_DIRECTIVE_GAUGES			3
static hud_frames Directive_gauge[NUM_DIRECTIVE_GAUGES];
static int Directive_frames_loaded = 0;

static char *Directive_fnames[3] = 
{
//XSTR:OFF
	"directives1",
	"directives2",
	"directives3"
//XSTR:ON
};

#define DIRECTIVE_H						9
#define DIRECTIVE_X						5
#define NUM_DIRECTIVE_COORDS			3
#define DIRECTIVE_COORDS_TOP			0
#define DIRECTIVE_COORDS_MIDDLE		1
#define DIRECTIVE_COORDS_TITLE		2
static int Directive_coords[GR_NUM_RESOLUTIONS][NUM_DIRECTIVE_COORDS][2] =
{
	{
		// GR_640
		{5,178},
		{5,190},
		{7,180}
	},
	{
		// GR_1024
		{5,278},
		{5,290},
		{7,280}
	}
};

// displays (renders) the training objectives list
void training_obj_display()
{
	char buf[256], *second_line;
	int i, t, x, y, z, height, end, offset, bx, by, y_count;
	color *c;

	if (!Training_obj_num_lines){
		return;
	}

	if ( !hud_gauge_active(HUD_DIRECTIVES_VIEW) ) {
		// Always draw the directives display if this is a training mission
		if ( !(The_mission.game_type & MISSION_TYPE_TRAINING) ) {
			return;
		}
	}

	// don't ever display directives display in multiplayer missions
	// if ( Game_mode & GM_MULTIPLAYER ){
	// 	return;
	// }

	height = gr_get_font_height();

	offset = 0;
	end = Training_obj_num_lines;
	if (end > Max_directives) {
		end = Max_directives;
		offset = Training_obj_num_lines - end;
	}

	// draw top of objective display
	// hud_set_default_color();
	hud_set_gauge_color(HUD_DIRECTIVES_VIEW);

	GR_AABITMAP(Directive_gauge[0].first_frame, Directive_coords[gr_screen.res][DIRECTIVE_COORDS_TOP][0]+fl2i(HUD_offset_x), Directive_coords[gr_screen.res][DIRECTIVE_COORDS_TOP][1]+fl2i(HUD_offset_y));
	// gr_set_bitmap(Directive_gauge[0].first_frame);
	// gr_aabitmap(Directive_coords[DIRECTIVE_COORDS_TOP][0]+fl2i(HUD_offset_x), Directive_coords[DIRECTIVE_COORDS_TOP][1]+fl2i(HUD_offset_y));

	// print out title
	emp_hud_printf(Directive_coords[gr_screen.res][DIRECTIVE_COORDS_TITLE][0]+fl2i(HUD_offset_x), Directive_coords[gr_screen.res][DIRECTIVE_COORDS_TITLE][1]+fl2i(HUD_offset_y), EG_OBJ_TITLE, XSTR( "directives", 422));
	// gr_printf(Directive_coords[DIRECTIVE_COORDS_TITLE][0]+fl2i(HUD_offset_x), Directive_coords[DIRECTIVE_COORDS_TITLE][1]+fl2i(HUD_offset_y), XSTR( "directives", 422));
	
	bx = DIRECTIVE_X+fl2i(HUD_offset_x);
	by = Directive_coords[gr_screen.res][DIRECTIVE_COORDS_MIDDLE][1]+fl2i(HUD_offset_y);

	y_count = 0;
	for (i=0; i<end; i++) {
		x = DIRECTIVE_X + 3 + fl2i(HUD_offset_x);
		y = Training_obj_window_coords[gr_screen.res][1] + fl2i(HUD_offset_y) + y_count * height + height / 2 + 1;
		z = TRAINING_OBJ_LINES_MASK(i + offset);

		c = &Color_normal;
		if (Training_obj_lines[i + offset] & TRAINING_OBJ_LINES_KEY) {
			message_translate_tokens(buf, Mission_events[z].objective_key_text);  // remap keys
//			gr_set_color_fast(&Color_normal);
			c = &Color_bright_green;
		} else {
			strcpy_s(buf, Mission_events[z].objective_text);
			if (Mission_events[z].count){
				sprintf(buf + strlen(buf), NOX(" [%d]"), Mission_events[z].count);
			}

			// if this is a multiplayer tvt game, and this is event is not for my team, don't display it
			if(MULTI_TEAM && (Net_player != NULL)){
				if((Mission_events[z].team != -1) && (Net_player->p_info.team != Mission_events[z].team)){
					continue;
				}
			}

			switch (mission_get_event_status(z)) {
			case EVENT_CURRENT:
//				gr_set_color_fast(&Color_bright_white);
				c = &Color_bright_white;
				break;

			case EVENT_FAILED:
//				gr_set_color_fast(&Color_bright_red);
				c = &Color_bright_red;
				break;

			case EVENT_SATISFIED:
//				gr_set_color_fast(&Color_bright_blue);
				t = Mission_events[z].satisfied_time;
				if (t + i2f(2) > Missiontime) {
					if (Missiontime % fl2f(.4f) < fl2f(.2f)){
						c = &Color_bright_blue;
					} else {
						c = &Color_bright_white;
					}
				} else {
					c = &Color_bright_blue;
				}
				break;
			}
		}

		// maybe split the directives line
		second_line = split_str_once(buf, 167);
		Assert( second_line != buf );

		// blit the background frames
		// hud_set_default_color();
		hud_set_gauge_color(HUD_DIRECTIVES_VIEW);

		GR_AABITMAP(Directive_gauge[1].first_frame, bx, by);
		// gr_set_bitmap(Directive_gauge[1].first_frame);
		// gr_aabitmap(bx, by);
		
		by += DIRECTIVE_H;

		if ( second_line ) {
			GR_AABITMAP(Directive_gauge[1].first_frame, bx, by);
			// gr_set_bitmap(Directive_gauge[1].first_frame);
			// gr_aabitmap(bx, by);
			
			by += DIRECTIVE_H;
		}

		// blit the text
		gr_set_color_fast(c);
		
		emp_hud_string(x, y, EG_OBJ1 + i, buf);
		// gr_printf(x, y, buf);
		
		y_count++;

		if ( second_line ) {
			y = Training_obj_window_coords[gr_screen.res][1] + fl2i(HUD_offset_y) + y_count * height + height / 2 + 1;
			
			emp_hud_string(x+12, y, EG_OBJ1 + i + 1, second_line);
			// gr_printf(x+12, y, second_line);
			
			y_count++;
		}
	}

	// draw the bottom of objective display
	// hud_set_default_color();
	hud_set_gauge_color(HUD_DIRECTIVES_VIEW);

	GR_AABITMAP(Directive_gauge[2].first_frame, bx, by);
	// gr_set_bitmap(Directive_gauge[2].first_frame);
	// gr_aabitmap(bx, by);
}

// mission initializations (called once before a new mission is started)
void training_mission_init()
{
	int i;

	Assert(!Training_num_lines);
	Training_obj_num_lines = 0;
	Training_message_queue_count = 0;
	Training_failure = 0;
	if (Max_directives > TRAINING_OBJ_LINES) {
		Max_directives = TRAINING_OBJ_LINES;
	}
	for (i=0; i<TRAINING_OBJ_LINES; i++)
		Training_obj_lines[i] = -1;

	// Goober5000
	for (i = 0; i < TRAINING_MESSAGE_QUEUE_MAX; i++)
		Training_message_queue[i].special_message = NULL;

	if ( !Directive_frames_loaded ) {
		for ( i = 0; i < NUM_DIRECTIVE_GAUGES; i++ ) {
			Directive_gauge[i].first_frame = bm_load_animation(Directive_fnames[i], &Directive_gauge[i].num_frames);
			if ( Directive_gauge[i].first_frame < 0 ) {
				Warning(LOCATION,"Cannot load hud ani: %s\n", Directive_fnames[i]);
			}
		}

		Directive_frames_loaded = 1;
	}

	// only clear player flags if this is actually a training mission
	if ( The_mission.game_type & MISSION_TYPE_TRAINING ) {
		Player->flags &= ~(PLAYER_FLAGS_MATCH_TARGET | PLAYER_FLAGS_MSG_MODE | PLAYER_FLAGS_AUTO_TARGETING | PLAYER_FLAGS_AUTO_MATCH_SPEED | PLAYER_FLAGS_LINK_PRIMARY | PLAYER_FLAGS_LINK_SECONDARY );
	}
}

void training_mission_page_in()
{
	int i;
	for ( i = 0; i < NUM_DIRECTIVE_GAUGES; i++ ) {
		bm_page_in_aabitmap( Directive_gauge[i].first_frame, Directive_gauge[i].num_frames );
	}
}


int comp_training_lines_by_born_on_date(const void *m1, const void *m2)
{
	int *e1, *e2;
	e1 = (int*) m1;
	e2 = (int*) m2;
	
	Assert(Mission_events[*e1 & 0xffff].born_on_date != 0);
	Assert(Mission_events[*e2 & 0xffff].born_on_date != 0);

	return (Mission_events[*e1 & 0xffff].born_on_date - Mission_events[*e2 & 0xffff].born_on_date);
}


// now sort list of events
// sort on EVENT_CURRENT and born on date, for other events (EVENT_SATISFIED, EVENT_FAILED) sort on born on date
#define MIN_SATISFIED_TIME		5
#define MIN_FAILED_TIME			7
void sort_training_objectives()
{
	int i, event_status, offset;

	// start by sorting on born on date
	insertion_sort(Training_obj_lines, Training_obj_num_lines, sizeof(int), comp_training_lines_by_born_on_date);

	// get the index of the first directive that will be displayed
	// if less than 0, display all lines
	offset = Training_obj_num_lines - Max_directives;

	if (offset <= 0) {
		return;
	}

	// go through lines 0 to offset-1 and check if there are any CURRENT or	RECENTLY_KNOWN events that should be shown
	int num_offset_events = 0;
	for (i=0; i<offset; i++) {
		event_status = mission_get_event_status(TRAINING_OBJ_LINES_MASK(i));
		
		// if this is a multiplayer tvt game, and this is event is for another team, don't touch it
		if(MULTI_TEAM && (Net_player != NULL)){
			if((Mission_events[TRAINING_OBJ_LINES_MASK(i)].team != -1) &&  (Net_player->p_info.team != Mission_events[TRAINING_OBJ_LINES_MASK(i)].team)){
				continue ;
			}
		}

		if (event_status == EVENT_CURRENT)  {
			Training_obj_lines[i] |= TRAINING_OBJ_STATUS_UNKNOWN;
			num_offset_events++;
		} else if (event_status ==	EVENT_SATISFIED) {
			if (f2i(Missiontime - Mission_events[TRAINING_OBJ_LINES_MASK(i)].satisfied_time) < MIN_SATISFIED_TIME) {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_UNKNOWN;
				num_offset_events++;
			} else {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_KNOWN;
			}
		} else if (event_status ==	EVENT_FAILED) {
			if (f2i(Missiontime - Mission_events[TRAINING_OBJ_LINES_MASK(i)].satisfied_time) < MIN_FAILED_TIME) {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_UNKNOWN;
				num_offset_events++;
			} else {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_KNOWN;
			}
		}
	}

	// if there are no directives which should be moved, we're done
	if (num_offset_events == 0) {
		return;
	}

	// go through lines offset to Training_obj_num_lines to check which should be shown, since some will need to be bumped
	for (i=offset; i<Training_obj_num_lines; i++) {
		event_status = mission_get_event_status(TRAINING_OBJ_LINES_MASK(i));

		// if this is a multiplayer tvt game, and this is event is for another team, it can be bumped
		if(MULTI_TEAM && (Net_player != NULL)){
			if((Mission_events[TRAINING_OBJ_LINES_MASK(i)].team != -1) &&  (Net_player->p_info.team != Mission_events[TRAINING_OBJ_LINES_MASK(i)].team)){
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_KNOWN;
				continue ;
			}
		}

		if (event_status == EVENT_CURRENT)  {
			Training_obj_lines[i] |= TRAINING_OBJ_STATUS_UNKNOWN;
		} else if (event_status ==	EVENT_SATISFIED) {
			if (f2i(Missiontime - Mission_events[TRAINING_OBJ_LINES_MASK(i)].satisfied_time) < MIN_SATISFIED_TIME) {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_UNKNOWN;
			} else {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_KNOWN;
			}
		} else if (event_status ==	EVENT_FAILED) {
			if (f2i(Missiontime - Mission_events[TRAINING_OBJ_LINES_MASK(i)].satisfied_time) < MIN_FAILED_TIME) {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_UNKNOWN;
			} else {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_UNKNOWN;
			}
		}
	}


	int slot_idx, unkn_vis, last_directive;
	// go through list and bump as needed
	for (i=0; i<num_offset_events; i++) {

		// find most recent directive that would not be shown
		for (unkn_vis=offset-1; unkn_vis>=0; unkn_vis--) {
			if (Training_obj_lines[unkn_vis] & TRAINING_OBJ_STATUS_UNKNOWN) {
				break;
			}
		}

		// find first slot that can be bumped
		// look at the last (N-4 to N) positions
		for (slot_idx=0; slot_idx<Max_directives; slot_idx++) {
			if ( Training_obj_lines[i+offset] & TRAINING_OBJ_STATUS_KNOWN ) {
				break;
			}
		}

		// shift and replace (mark old one as STATUS_KNOWN)
		// store the directive that won't be shown
		last_directive = Training_obj_lines[Training_obj_num_lines-1];

		for (int j=slot_idx; j>0; j--) {
			Training_obj_lines[j+offset-1] = Training_obj_lines[j+offset-2];
		}
		Training_obj_lines[offset] = Training_obj_lines[unkn_vis];
		Training_obj_lines[unkn_vis] = last_directive;
		Training_obj_lines[unkn_vis] &= ~TRAINING_OBJ_LINES_EVENT_STATUS_MASK;
		Training_obj_lines[unkn_vis] |= TRAINING_OBJ_STATUS_KNOWN;
	}

	// remove event status
	for (i=0; i<Training_obj_num_lines; i++) {
		Training_obj_lines[i] &= ~TRAINING_OBJ_LINES_EVENT_STATUS_MASK;
	}
}

// called at same rate as goals/events are evaluated.  Maintains the objectives listing, adding,
// removing and updating items
void training_check_objectives()
{
	int i, event_idx, event_status;

	Training_obj_num_lines = 0;
	for (event_idx=0; event_idx<Num_mission_events; event_idx++) {
		event_status = mission_get_event_status(event_idx);
		if ( (event_status != EVENT_UNBORN) && Mission_events[event_idx].objective_text && (timestamp() > Mission_events[event_idx].born_on_date + 3000) ) {
			if (!Training_failure || !strnicmp(Mission_events[event_idx].name, XSTR( "Training failed", 423), 15)) {

				// check for the actual objective
				for (i=0; i<Training_obj_num_lines; i++) {
					if (Training_obj_lines[i] == event_idx) {
						break;
					}
				}

				// not in objective list, need to add it
				if (i == Training_obj_num_lines) {
					if (Training_obj_num_lines == TRAINING_OBJ_LINES) {

						// objective list full, remove topmost objective
						for (i=1; i<TRAINING_OBJ_LINES; i++) {
							Training_obj_lines[i - 1] = Training_obj_lines[i];
						}
						Training_obj_num_lines--;
					}
					// add the new directive
					Training_obj_lines[Training_obj_num_lines++] = event_idx;
				}

				// now check for the associated keypress text
				for (i=0; i<Training_obj_num_lines; i++) {
					if (Training_obj_lines[i] == (event_idx | TRAINING_OBJ_LINES_KEY)) {
						break;
					}
				}

				// if there is a keypress message with directive, process that too.
				if (Mission_events[event_idx].objective_key_text) {
					if (event_status == EVENT_CURRENT) {

						// not in objective list, need to add it
						if (i == Training_obj_num_lines) {
							if (Training_obj_num_lines == TRAINING_OBJ_LINES) {

								// objective list full, remove topmost objective
								for (i=1; i<Training_obj_num_lines; i++) {
									Training_obj_lines[i - 1] = Training_obj_lines[i];
								}
								Training_obj_num_lines--;
							}
							// mark objective as having key text
							Training_obj_lines[Training_obj_num_lines++] = event_idx | TRAINING_OBJ_LINES_KEY;
						}

					} else {
						// message with key press text is no longer valid, so remove it
						if (i < Training_obj_num_lines) {
							for (; i<Training_obj_num_lines - 1; i++) {
								Training_obj_lines[i] = Training_obj_lines[i + 1];
							}
							Training_obj_num_lines--;
						}
					}
				}
			}
		}
	}

	// now sort list of events
	// sort on EVENT_CURRENT and born on date, for other events (EVENT_SATISFIED, EVENT_FAILED) sort on born on date
	sort_training_objectives();
}

// called to do cleanup when leaving a mission
void training_mission_shutdown()
{
	int i;

	if (Training_voice >= 0) {
		if (Training_voice_type) {
			audiostream_close_file(Training_voice_handle, 0);

		} else {
			snd_stop(Training_voice_handle);
		}
	}

	// Goober5000
	for (i = 0; i < TRAINING_MESSAGE_QUEUE_MAX; i++)
	{
		if (Training_message_queue[i].special_message != NULL)
		{
			vm_free(Training_message_queue[i].special_message);
			Training_message_queue[i].special_message = NULL;
		}
	}

	Training_voice = -1;
	Training_num_lines = Training_obj_num_lines = 0;

	*Training_buf = 0;
}

// translates special tokens.  Handles one token only.
char *translate_message_token(char *str)
{
	if (!stricmp(str, NOX("wp"))) {
		sprintf(str, "%d", Training_context_goal_waypoint + 1);
		return str;
	}

	return NULL;
}

// translates all special tokens in a message, producing the new finalized message to be displayed
void message_translate_tokens(char *buf, char *text)
{
	char temp[40], *toke1, *toke2, *ptr, *orig_buf;
	int r;

	orig_buf = buf;
	*buf = 0;
	toke1 = strchr(text, '$');
	toke2 = strchr(text, '#');
	while (toke1 || toke2) {  // is either token types present?
		if (!toke2 || (toke1 && (toke1 < toke2))) {  // found $ before #
			strncpy(buf, text, toke1 - text + 1);  // copy text up to token
			buf += toke1 - text + 1;
			text = toke1 + 1;  // advance pointers past processed data

			toke2 = strchr(text, '$');
			if (!toke2 || ((toke2 - text) == 0))  // No second one, or possibly a double?
				break;

			// make sure we aren't going to have any type of out-of-bounds issues
			if ( ((toke2 - text) < 0) || ((toke2 - text) >= (ptr_s)sizeof(temp)) ) {
				Int3();
			} else {
				strncpy(temp, text, toke2 - text);  // isolate token into seperate buffer
				temp[toke2 - text] = 0;  // null terminate string
				ptr = translate_key(temp);  // try and translate key
				if (ptr) {  // was key translated properly?
					if (!stricmp(ptr, NOX("none")) && (Training_bind_warning != Missiontime)) {
						if ( The_mission.game_type & MISSION_TYPE_TRAINING ) {
							r = popup(PF_TITLE_BIG | PF_TITLE_RED, 2, XSTR( "&Bind Control", 424), XSTR( "&Abort mission", 425),
								XSTR( "Warning\nYou have no control bound to the action \"%s\".  You must do so before you can continue with your training.", 426),
								XSTR(Control_config[Failed_key_index].text, CONTROL_CONFIG_XSTR + Failed_key_index));

							if (r) {  // do they want to abort the mission?
								gameseq_post_event(GS_EVENT_END_GAME);
								return;
							}

							gameseq_post_event(GS_EVENT_CONTROL_CONFIG);  // goto control config screen to bind the control
						}
					}

					buf--;  // erase the $
					strcpy(buf, ptr);  // put translated key in place of token
					buf += strlen(buf);
					text = toke2 + 1;
				}
			}
		} else {
			strncpy(buf, text, toke2 - text + 1);  // copy text up to token
			buf += toke2 - text + 1;
			text = toke2 + 1;  // advance pointers past processed data

			toke1 = strchr(text, '#');
			if (!toke1 || ((toke1 - text) == 0))  // No second one, or possibly a double?
				break;

			// make sure we aren't going to have any type of out-of-bounds issues
			if ( ((toke1 - text) < 0) || ((toke1 - text) >= (ptr_s)sizeof(temp)) ) {
				Int3();
			} else {
				strncpy(temp, text, toke1 - text);  // isolate token into seperate buffer
				temp[toke1 - text] = 0;  // null terminate string
				ptr = translate_message_token(temp);  // try and translate key
				if (ptr) {  // was key translated properly?
					buf--;  // erase the #
					strcpy(buf, ptr);  // put translated key in place of token
					buf += strlen(buf);
					text = toke1 + 1;
				}
			}
		}

		toke1 = strchr(text, '$');
		toke2 = strchr(text, '#');
	}

	strcpy(buf, text);
	return;
}

// plays the voice file associated with a training message.  Automatically streams the file
// from disk if it's over 100k, otherwise plays it as a normal file in memory.  Returns -1
// if it didn't play, otherwise index of voice
int message_play_training_voice(int index)
{
	int len;
	CFILE *fp;

	if (index < 0) {
		if (Training_voice >= 0) {
			if (Training_voice_type) {
				audiostream_close_file(Training_voice_handle, 0);

			} else {
				snd_stop(Training_voice_handle);
			}
		}

		Training_voice = -1;
		return -1;
	}

	if (Message_waves[index].num < 0) {
		fp = cfopen(Message_waves[index].name, "rb");
		if (!fp)
			return -1;

		len = cfilelength(fp);
		cfclose(fp);
		if (len > 100000) {
			if ((Training_voice < 0) || !Training_voice_type || (Training_voice != index)) {
				if (Training_voice >= 0) {
					if (Training_voice_type) {
						if (Training_voice == index)
							audiostream_stop(Training_voice_handle, 1, 0);
						else
							audiostream_close_file(Training_voice_handle, 0);

					} else {
						snd_stop(Training_voice_handle);
					}
				}

				if (strnicmp(Message_waves[index].name, NOX("none.wav"), 4)) {
					Training_voice_handle = audiostream_open(Message_waves[index].name, ASF_VOICE);
					if (Training_voice_handle < 0) {
						nprintf(("Warning", "Unable to load voice file %s\n", Message_waves[index].name));
					//	Warning(LOCATION, "Unable to load voice file %s\n", Message_waves[index].name);
					}
				}
			}  // Training_voice should be valid and loaded now

			Training_voice_type = 1;
			if (Training_voice_handle >= 0)
				audiostream_play(Training_voice_handle, (Master_voice_volume * aav_voice_volume), 0);

			Training_voice = index;
			return Training_voice;

		} else {
			game_snd tmp_gs;
			memset(&tmp_gs, 0, sizeof(game_snd));
			strcpy_s(tmp_gs.filename, Message_waves[index].name);
			Message_waves[index].num = snd_load(&tmp_gs, 0);
			if (Message_waves[index].num < 0) {
				nprintf(("Warning", "Cannot load message wave: %s.  Will not play\n", Message_waves[index].name));
				return -1;
			}
		}
	}

	if (Training_voice >= 0) {
		if (Training_voice_type) {
			audiostream_close_file(Training_voice_handle, 0);

		} else {
			snd_stop(Training_voice_handle);
		}
	}

	Training_voice = index;
	if (Message_waves[index].num >= 0)
		Training_voice_handle = snd_play_raw(Message_waves[index].num, 0.0f);
	else
		Training_voice_handle = -1;

	Training_voice_type = 0;
	return Training_voice;
}

// one time initializations done when we want to display a new training mission.  This does
// all the processing and setup required to actually display it, including starting the
// voice file playing
void message_training_setup(int m, int length, char *special_message)
{
	if ((m < 0) || !Messages[m].message[0]) {  // remove current message from the screen
		Training_num_lines = 0;
		return;
	}

	// translate tokens in message to the real things
	if (special_message == NULL)
		message_translate_tokens(Training_buf, Messages[m].message);
	else
		message_translate_tokens(Training_buf, special_message);

	HUD_add_to_scrollback(Training_buf, HUD_SOURCE_TRAINING);

	// moved from message_training_display() because we got rid of an extra buffer and we have to determine
	// the number of lines earlier to avoid inadvertant modification of Training_buf.  - taylor
	training_process_message(Training_buf);
	Training_num_lines = split_str(Training_buf, TRAINING_LINE_WIDTH, Training_line_sizes, Training_lines, MAX_TRAINING_MESSAGE_LINES);

	Assert( Training_num_lines >= 0 );

	if (message_play_training_voice(Messages[m].wave_info.index) < 0) {
		if (length > 0)
			Training_message_timestamp = timestamp(length * 1000);
		else
			Training_message_timestamp = timestamp(TRAINING_TIMING_BASE + strlen(Messages[m].message) * TRAINING_TIMING);  // no voice file playing

	} else
		Training_message_timestamp = 0;
}

// adds simple text to the directives display
/*id message_training_add_simple( char *text )
{
	int i;

	training_process_message(text);
	HUD_add_to_scrollback(Training_buf, HUD_SOURCE_TRAINING);
	Training_num_lines = split_str(Training_buf, TRAINING_LINE_WIDTH, Training_line_sizes, Training_lines, MAX_TRAINING_MESSAGE_LINES);
	Assert(Training_num_lines > 0);
	for (i=0; i<Training_num_lines; i++)
		Training_lines[i][Training_line_sizes[i]] = 0;

	Training_message_timestamp = timestamp(5000);
} */

// add a message to the queue to be sent later.
void message_training_queue(char *text, int timestamp, int length)
{
	int m;
	char temp_buf[TRAINING_MESSAGE_LENGTH];

	Assert(Training_message_queue_count < TRAINING_MESSAGE_QUEUE_MAX);
	if (Training_message_queue_count < TRAINING_MESSAGE_QUEUE_MAX) {
		if (!stricmp(text, NOX("none"))) {
			m = -1;
		} else {
			for (m=0; m<Num_messages; m++)
				if (!stricmp(text, Messages[m].name))
					break;

			Assert(m < Num_messages);
			if (m >= Num_messages)
				return;
		}

		Training_message_queue[Training_message_queue_count].num = m;
		Training_message_queue[Training_message_queue_count].timestamp = timestamp;
		Training_message_queue[Training_message_queue_count].length = length;

		// Goober5000 - this shouldn't happen, but let's be safe
		if (Training_message_queue[Training_message_queue_count].special_message != NULL)
		{
			Int3();
			vm_free(Training_message_queue[Training_message_queue_count].special_message);
			Training_message_queue[Training_message_queue_count].special_message = NULL;
		}

		// Goober5000 - replace variables if necessary
		strcpy_s(temp_buf, Messages[m].message);
		if (sexp_replace_variable_names_with_values(temp_buf, MESSAGE_LENGTH))
			Training_message_queue[Training_message_queue_count].special_message = vm_strdup(temp_buf);

		Training_message_queue_count++;
	}
}

// Goober5000 - removes current message from the queue
void message_training_remove_from_queue(int idx)
{
	// we're overwriting all messages with the next message, but to
	// avoid memory leaks, we should free the special message entry
	if (Training_message_queue[idx].special_message != NULL)
	{
		vm_free(Training_message_queue[idx].special_message);
		Training_message_queue[idx].special_message = NULL;
	}

	// replace current message with the one above it, etc.
	for (int j=idx+1; j<Training_message_queue_count; j++)
		Training_message_queue[j - 1] = Training_message_queue[j];

	// delete the topmost message
	Training_message_queue_count--;
	Training_message_queue[Training_message_queue_count].length = -1;
	Training_message_queue[Training_message_queue_count].num = -1;
	Training_message_queue[Training_message_queue_count].timestamp = -1;
	Training_message_queue[Training_message_queue_count].special_message = NULL;	// not a memory leak because we copied the pointer
}

// check the training message queue to see if we should play a new message yet or not.
// Goober5000: removed stipulation of instructor being present
void message_training_queue_check()
{
	int i, iship_num;

	// get the instructor's ship.
	iship_num = ship_name_lookup(NOX("instructor"));
//	if ( iship_num == -1 )	// commented out by Goober5000
//		return;

	// if the instructor is dying or departing, do nothing
	if ( iship_num != -1 )	// added by Goober5000
		if (Ships[iship_num].flags & (SF_DYING | SF_DEPARTING))
			return;

	if (Training_failure)
		return;

	for (i=0; i<Training_message_queue_count; i++) {
		if (timestamp_elapsed(Training_message_queue[i].timestamp)) {
			message_training_setup(Training_message_queue[i].num, Training_message_queue[i].length, Training_message_queue[i].special_message);

			// remove this message from the queue now.
			message_training_remove_from_queue(i);
			i--;
		}
	}
}

// displays (renders) the training message to the screen
void message_training_display()
{
	char *str, buf[256];
	int i, z, x, y, height, mode, count;

	Training_message_visible = 0;
	message_training_queue_check();
	
	if ( !hud_disabled_except_messages() ) {
		training_obj_display();
	}
	

	if (Training_failure){
		return;
	}

	if (timestamp_elapsed(Training_message_timestamp) || !strlen(Training_buf)){
		return;
	}

	// the code that preps the training message and counts the number of lines
	// has been moved to message_training_setup()

	if (Training_num_lines <= 0){
		return;
	}

	for (i=0; i<Training_num_lines; i++) {
		Training_lines[i][Training_line_sizes[i]] = 0;
		drop_leading_white_space(Training_lines[i]);
	}

	height = gr_get_font_height();
	gr_set_shader(&Training_msg_glass);
	gr_shade(Training_message_window_coords[gr_screen.res][0], Training_message_window_coords[gr_screen.res][1], TRAINING_MESSAGE_WINDOW_WIDTH, Training_num_lines * height + height);

	gr_set_color_fast(&Color_bright_blue);
	mode = count = 0;
	Training_message_visible = 1;
	for (i=0; i<Training_num_lines; i++) {  // loop through all lines of message
		str = Training_lines[i];
		z = 0;
		x = Training_message_window_coords[gr_screen.res][0] + (TRAINING_MESSAGE_WINDOW_WIDTH - TRAINING_LINE_WIDTH) / 2;
		y = Training_message_window_coords[gr_screen.res][1] + i * height + height / 2 + 1;

		while (*str) {  // loop through each character of each line
			if ((count < MAX_TRAINING_MESSAGE_MODS) && (str == Training_message_mods[count].pos)) {
				buf[z] = 0;
				gr_printf(x, y, buf);
				gr_get_string_size(&z, NULL, buf);
				x += z;
				z = 0;

				mode = Training_message_mods[count++].mode;
				switch (mode) {
					case TMMOD_NORMAL:
						gr_set_color_fast(&Color_bright_blue);
						break;

					case TMMOD_BOLD:
						gr_set_color_fast(&Color_white);
						break;
				}
			}

			buf[z++] = *str++;
		}

		if (z) {
			buf[z] = 0;
			gr_printf(x, y, "%s", buf);
		}
	}

	Training_message_method = 0;
//	if (Training_message_method) {
//		char *message = "Press a key to continue";

//		gr_get_string_size(&i, NULL, message);
//		gr_printf(TRAINING_MESSAGE_WINDOW_X + TRAINING_MESSAGE_WINDOW_WIDTH / 2 - i / 2, TRAINING_MESSAGE_WINDOW_Y + (Training_num_lines + 2) * height, message);
//	}

	if ((Training_voice >= 0) && (Training_num_lines > 0) && !(Training_message_timestamp)) {
		if (Training_voice_type)
			z = audiostream_is_playing(Training_voice_handle);
		else
			z = snd_is_playing(Training_voice_handle);

		if (!z)
			Training_message_timestamp = timestamp(2000);  // 2 second delay
 	}
}

// processes a new training message to get hilighting information and store it in internal structures.
void training_process_message(char *message)
{
	int count;
	char *src, *dest, buf[TRAINING_MESSAGE_LENGTH];

	message_translate_tokens(buf, message);
	count = 0;
	src = buf;
	dest = Training_buf;
	while (*src) {
		if (!strnicmp(src, NOX("<b>"), 3)) {
			Assert(count < MAX_TRAINING_MESSAGE_MODS);
			src += 3;
			Training_message_mods[count].pos = dest;
			Training_message_mods[count].mode = TMMOD_BOLD;
			count++;
		}

		if (!strnicmp(src, NOX("</b>"), 4)) {
			Assert(count < MAX_TRAINING_MESSAGE_MODS);
			src += 4;
			Training_message_mods[count].pos = dest;
			Training_message_mods[count].mode = TMMOD_NORMAL;
			count++;
		}

		*dest++ = *src++;
	}

	*dest = 0;
	if (count < MAX_TRAINING_MESSAGE_MODS)
		Training_message_mods[count].pos = NULL;
}

void training_fail()
{
	Training_failure = 1;
	//	JasonH: Add code here to suspend training and display a directive to warp out.
	//	Suspend training.
	//	Give directive to warp out.
	//	Also ensure that a special failure debriefing is given.  Must mention firing at instructor.
	//	Ask Sandeep to write it (or you can).
}
