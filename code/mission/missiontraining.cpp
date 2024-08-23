/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "cfile/cfile.h"
#include "gamesequence/gamesequence.h"
#include "globalincs/alphacolors.h"
#include "globalincs/utility.h"
#include "hud/hudmessage.h"
#include "io/timer.h"
#include "mission/missiongoals.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"
#include "mission/missiontraining.h"
#include "mod_table/mod_table.h"
#include "network/multi.h"
#include "parse/parselo.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"
#include "playerman/player.h"
#include "popup/popup.h"
#include "ship/ship.h"
#include "sound/audiostr.h"
#include "sound/fsspeech.h"
#include "sound/sound.h"
#include "weapon/emp.h"



#define MAX_TRAINING_MESSAGE_LINES		10
#define TRAINING_MESSAGE_WINDOW_WIDTH	266
#define TRAINING_LINE_WIDTH			250  // width in pixels of actual text
#define TRAINING_TIMING					150  // milliseconds per character to display messages
#define TRAINING_TIMING_BASE			1000  // Minimum milliseconds to display any message
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
	size_t pos;
	int mode;  // what function to perform at given position (TMMOD_*)
} training_message_mods;  // training message modifiers

typedef struct {
	int num;
	TIMESTAMP timestamp;
	int length;
	char *special_message;
} training_message_queue;

SCP_string Training_buf;
const char *Training_lines[MAX_TRAINING_MESSAGE_LINES];  // Training message split into lines
int Training_line_lengths[MAX_TRAINING_MESSAGE_LINES];

char Training_voice_filename[NAME_LENGTH];
int Max_directives = TRAINING_OBJ_DISPLAY_LINES;
TIMESTAMP Training_message_timestamp;
int Training_message_method = 1;
int Training_num_lines = 0;
int Training_voice = -1;
int Training_voice_type;
int Training_voice_soundstream;
sound_handle Training_voice_snd_handle;
int Training_flag = 0;
int Training_failure = 0;
int Training_message_queue_count = 0;
int Training_bind_warning = -1;  // Missiontime at which we last gave warning
int Training_message_visible;
training_message_queue Training_message_queue[TRAINING_MESSAGE_QUEUE_MAX];

int	Training_context = 0;
int	Training_context_speed_set;
int	Training_context_speed_min;
int	Training_context_speed_max;
TIMESTAMP	Training_context_speed_timestamp;
waypoint_list *Training_context_path;
int Training_context_goal_waypoint;
int Training_context_at_waypoint;
float	Training_context_distance;

int Players_target = UNINITIALIZED;
int Players_mlocked = UNINITIALIZED; // for is-missile-locked - Sesquipedalian
ship_subsys *Players_targeted_subsys;
TIMESTAMP Players_target_timestamp;
TIMESTAMP Players_mlocked_timestamp;


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
int Training_obj_num_lines = 0;
int Training_obj_num_display_lines = 0;
int Training_obj_lines[TRAINING_OBJ_LINES];
training_message_mods Training_message_mods[MAX_TRAINING_MESSAGE_MODS];

// local module prototypes
void training_process_message();


HudGaugeDirectives::HudGaugeDirectives():
HudGauge(HUD_OBJECT_DIRECTIVES, HUD_DIRECTIVES_VIEW, false, true, (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP), 255, 255, 255)
{
}

void HudGaugeDirectives::initHeaderOffsets(int x, int y)
{
	header_offsets[0] = x;
	header_offsets[1] = y;
}

void HudGaugeDirectives::initMiddleFrameOffsetY(int y)
{
	middle_frame_offset_y = y;
}

void HudGaugeDirectives::initTextStartOffsets(int x, int y)
{
	text_start_offsets[0] = x;
	text_start_offsets[1] = y;
}

void HudGaugeDirectives::initTextHeight(int h)
{
	text_h = h;
}

void HudGaugeDirectives::initMaxLineWidth(int w)
{
	max_line_width = w;
}

void HudGaugeDirectives::initKeyLineXOffset(int offset)
{
	key_line_x_offset = offset;
}

void HudGaugeDirectives::initBottomBgOffset(int offset)
{
	bottom_bg_offset = offset;
}

void HudGaugeDirectives::initBitmaps(char *fname_top, char *fname_middle, char *fname_bottom)
{
	directives_top.first_frame = bm_load_animation(fname_top, &directives_top.num_frames);
	if ( directives_top.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname_top);
	}

	directives_middle.first_frame = bm_load_animation(fname_middle, &directives_middle.num_frames);
	if ( directives_middle.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname_middle);
	}

	directives_bottom.first_frame = bm_load_animation(fname_bottom, &directives_bottom.num_frames);
	if ( directives_bottom.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: %s\n", fname_bottom);
	}
}

bool HudGaugeDirectives::canRender() const
{
	if (sexp_override) {
		return false;
	}

	if(hud_disabled_except_messages()) {
		return false;
	}

	if (hud_disabled() && !hud_disabled_except_messages()) {
		return false;
	}

	// force the directives list to display in training missions even if this gauge isn't active.
	if(!active && !(The_mission.game_type & MISSION_TYPE_TRAINING))
		return false;
	
	if ( !(Game_detail_flags & DETAIL_FLAG_HUD) ) {
		return false;
	}

	if ((Viewer_mode & disabled_views)) {
		return false;
	}

	if(pop_up) {
		if(!popUpActive()) {
			return false;
		}
	}

	if (gauge_config == HUD_ETS_GAUGE) {
		if (Ships[Player_obj->instance].flags[Ship::Ship_Flags::No_ets]) {
			return false;
		}
	}

	return true;
}

void HudGaugeDirectives::pageIn()
{
	bm_page_in_aabitmap(directives_top.first_frame, directives_top.num_frames);
	bm_page_in_aabitmap(directives_middle.first_frame, directives_middle.num_frames);
	bm_page_in_aabitmap(directives_bottom.first_frame, directives_bottom.num_frames);
}

void HudGaugeDirectives::render(float  /*frametime*/)
{
	char buf[256], *second_line;
	int i, x, y, z, end, offset, bx, by;
	TIMESTAMP t;
	color *c;

	Training_obj_num_display_lines = 0;

	if (!Training_obj_num_lines){
		return;
	}

	offset = 0;
	end = Training_obj_num_lines;
	if (end > Max_directives) {
		end = Max_directives;
		offset = Training_obj_num_lines - end;
	}

	// draw top of objective display
	setGaugeColor();

	if (directives_top.first_frame >= 0)
		renderBitmap(directives_top.first_frame, position[0], position[1]);

	// print out title
	renderPrintf(position[0] + header_offsets[0], position[1] + header_offsets[1], EG_OBJ_TITLE, "%s", XSTR( "directives", 422));

	bx = position[0];
	by = position[1] + middle_frame_offset_y;

	for (i=0; i<end; i++) {
		x = position[0] + text_start_offsets[0];
		y = position[1] + text_start_offsets[1] + Training_obj_num_display_lines * text_h;
		z = TRAINING_OBJ_LINES_MASK(i + offset);

		int line_x_offset = 0;

		c = &Color_normal;
		if (Training_obj_lines[i + offset] & TRAINING_OBJ_LINES_KEY) {
			SCP_string temp_buf = message_translate_tokens(Mission_events[z].objective_key_text.c_str());  // remap keys
			strcpy_s(buf, temp_buf.c_str());
			c = &Color_bright_green;
			line_x_offset = key_line_x_offset;
		} else {
			strcpy_s(buf, Mission_events[z].objective_text.c_str());
			if (Mission_events[z].count){
				sprintf(buf + strlen(buf), NOX(" [%d]"), Mission_events[z].count);
			}

			// if this is a multiplayer tvt game, and this is event is not for my team, don't display it
			if((MULTI_TEAM) && (Net_player != NULL)){
				if((Mission_events[z].team != -1) && (Net_player->p_info.team != Mission_events[z].team)){
					continue;
				}
			}

			switch (mission_get_event_status(z)) {
			case EventStatus::CURRENT:
				c = &Color_bright_white;
				break;

			case EventStatus::FAILED:
				c = &Color_bright_red;
				break;

			case EventStatus::SATISFIED:
				t = Mission_events[z].satisfied_time;
				Assertion(t.isValid(), "Since event %s was satisfied, satisfied_time must be valid here", Mission_events[z].name.c_str());
				if (timestamp_since(t) < 2 * MILLISECONDS_PER_SECOND) {
					if (Missiontime % fl2f(.4f) < fl2f(.2f)){
						c = &Color_bright_blue;
					} else {
						c = &Color_bright_white;
					}
				} else {
					c = &Color_bright_blue;
				}
				break;

			default:
				// stick with Color_normal
				break;
			}
		}

		// maybe split the directives line
		second_line = split_str_once(buf, max_line_width);

		// if we are unable to split the line, just print it once
		if (second_line == buf) {
			second_line = nullptr;
		}

		// blit the background frames
		setGaugeColor();

		if (directives_middle.first_frame >= 0)
			renderBitmap(directives_middle.first_frame, bx, by);
		
		by += text_h;

		if ( second_line ) {

			if (directives_middle.first_frame >= 0)
				renderBitmap(directives_middle.first_frame, bx, by);
			
			by += text_h;
		}

		// blit the text
		gr_set_color_fast(c);
		
		renderString(x + line_x_offset, y, EG_OBJ1 + i, buf);
		
		Training_obj_num_display_lines++;

		if ( second_line ) {
			y = position[1] + text_start_offsets[1] + Training_obj_num_display_lines * text_h;
			
			renderString(x+12, y, EG_OBJ1 + i + 1, second_line);
			
			Training_obj_num_display_lines++;
		}
	}

	// draw the bottom of objective display
	setGaugeColor();

	if (directives_bottom.first_frame >= 0)
		renderBitmap(directives_bottom.first_frame, bx, by + bottom_bg_offset);
}

/**
 * Mission initializations (called once before a new mission is started)
 */
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

	// only clear player flags if this is actually a training mission
	if ( The_mission.game_type & MISSION_TYPE_TRAINING ) {
		Player->flags &= ~(PLAYER_FLAGS_MATCH_TARGET | PLAYER_FLAGS_MSG_MODE | PLAYER_FLAGS_AUTO_TARGETING | PLAYER_FLAGS_AUTO_MATCH_SPEED | PLAYER_FLAGS_LINK_PRIMARY | PLAYER_FLAGS_LINK_SECONDARY );
	}

	Training_context = 0;
	Training_context_speed_set = 0;
	Training_context_speed_timestamp = TIMESTAMP::invalid();
	Training_context_path = nullptr;

	Players_target = UNINITIALIZED;
	Players_mlocked = UNINITIALIZED;
	Players_targeted_subsys = nullptr;
	Players_target_timestamp = TIMESTAMP::invalid();
	Players_mlocked_timestamp = TIMESTAMP::invalid();
}

int comp_training_lines_by_born_on_date(const int *e1, const int *e2)
{
	return timestamp_compare(Mission_events[*e1 & 0xffff].born_on_date, Mission_events[*e2 & 0xffff].born_on_date);
}


/**
 * Sort list of training events
 *
 * Sort on EVENT_CURRENT and born on date, for other events (EVENT_SATISFIED, EVENT_FAILED) sort on born on date
 */
#define MIN_SATISFIED_TIME		5 * MILLISECONDS_PER_SECOND
#define MIN_FAILED_TIME			7 * MILLISECONDS_PER_SECOND
void sort_training_objectives()
{
	int i, offset;

	// start by sorting on born on date
	insertion_sort(Training_obj_lines, Training_obj_num_lines, comp_training_lines_by_born_on_date);

	// get the index of the first directive that will be displayed
	// if less than 0, display all lines
	offset = Training_obj_num_lines - Max_directives;

	if (offset <= 0) {
		return;
	}

	// go through lines 0 to offset-1 and check if there are any CURRENT or	RECENTLY_KNOWN events that should be shown
	int num_offset_events = 0;
	for (i=0; i<offset; i++) {
		int event_num = TRAINING_OBJ_LINES_MASK(i);
		auto event_status = mission_get_event_status(event_num);
		
		// if this is a multiplayer tvt game, and this is event is for another team, don't touch it
		if(MULTI_TEAM && (Net_player != NULL)){
			if((Mission_events[event_num].team != -1) &&  (Net_player->p_info.team != Mission_events[event_num].team)){
				continue ;
			}
		}

		if (event_status == EventStatus::CURRENT)  {
			Training_obj_lines[i] |= TRAINING_OBJ_STATUS_UNKNOWN;
			num_offset_events++;
		} else if (event_status == EventStatus::SATISFIED) {
			Assertion(Mission_events[event_num].satisfied_time.isValid(), "Since event %s was satisfied, satisfied_time must be valid here", Mission_events[event_num].name.c_str());
			if (timestamp_since(Mission_events[event_num].satisfied_time) < MIN_SATISFIED_TIME) {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_UNKNOWN;
				num_offset_events++;
			} else {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_KNOWN;
			}
		} else if (event_status == EventStatus::FAILED) {
			Assertion(Mission_events[event_num].satisfied_time.isValid(), "Since event %s failed, satisfied_time must be valid here", Mission_events[event_num].name.c_str());
			if (timestamp_since(Mission_events[event_num].satisfied_time) < MIN_FAILED_TIME) {
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
		int event_num = TRAINING_OBJ_LINES_MASK(i);
		auto event_status = mission_get_event_status(event_num);

		// if this is a multiplayer tvt game, and this is event is for another team, it can be bumped
		if(MULTI_TEAM && (Net_player != NULL)){
			if((Mission_events[event_num].team != -1) &&  (Net_player->p_info.team != Mission_events[event_num].team)){
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_KNOWN;
				continue ;
			}
		}

		if (event_status == EventStatus::CURRENT)  {
			Training_obj_lines[i] |= TRAINING_OBJ_STATUS_UNKNOWN;
		} else if (event_status == EventStatus::SATISFIED) {
			Assertion(Mission_events[event_num].satisfied_time.isValid(), "Since event %s was satisfied, satisfied_time must be valid here", Mission_events[event_num].name.c_str());
			if (timestamp_since(Mission_events[event_num].satisfied_time) < MIN_SATISFIED_TIME) {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_UNKNOWN;
			} else {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_KNOWN;
			}
		} else if (event_status == EventStatus::FAILED) {
			Assertion(Mission_events[event_num].satisfied_time.isValid(), "Since event %s failed, satisfied_time must be valid here", Mission_events[event_num].name.c_str());
			if (timestamp_since(Mission_events[event_num].satisfied_time) < MIN_FAILED_TIME) {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_UNKNOWN;
			} else {
				Training_obj_lines[i] |= TRAINING_OBJ_STATUS_KNOWN;
			}
		}
	}

	// go through list and bump as needed
	for (i=0; i<num_offset_events; i++) {
		int slot_idx, unkn_vis, slot_directive;

		// find most recent directive that would not be shown
		for (unkn_vis=offset-1; unkn_vis>=0; unkn_vis--) {
			if (Training_obj_lines[unkn_vis] & TRAINING_OBJ_STATUS_UNKNOWN) {
				break;
			}
		}

		// find first slot that can be bumped
		// look at the last (N-4 to N) positions
		for (slot_idx=0; slot_idx<Max_directives; slot_idx++) {
			if ( Training_obj_lines[slot_idx+offset] & TRAINING_OBJ_STATUS_KNOWN ) {
				break;
			}
		}

		if (slot_idx == Max_directives){
			// We did not manage to find space for all directives! This is bad, but nothing we can do about it.
			break;
		}

		// Since the directive to be shown here is older than the ones currently on display, remove the directive to be bumped and then shift all others up one until that entry.
		// Store the directive to be bumped for later.
		slot_directive = Training_obj_lines[slot_idx+offset];

		// Shift newer objectives
		for (int j=slot_idx; j>0; j--) {
			Training_obj_lines[j+offset] = Training_obj_lines[j+offset-1];
		}

		//Place directive to be shown in the topmost slot
		Training_obj_lines[offset] = Training_obj_lines[unkn_vis];

		//Restore bumped directive on the non-rendered slot
		Training_obj_lines[unkn_vis] = slot_directive;

		Assertion((Training_obj_lines[unkn_vis] & TRAINING_OBJ_LINES_EVENT_STATUS_MASK) == TRAINING_OBJ_STATUS_KNOWN, "Objective bumped was not known");
	}

	// remove event status
	for (i=0; i<Training_obj_num_lines; i++) {
		Training_obj_lines[i] &= ~TRAINING_OBJ_LINES_EVENT_STATUS_MASK;
	}
}

/**
 * Maintains the objectives listing, adding, removing and updating items
 *
 * Called at same rate as goals/events are evaluated.  
 */
void training_check_objectives()
{
	int i, event_idx;

	Training_obj_num_lines = 0;
	for (event_idx=0; event_idx<(int)Mission_events.size(); event_idx++) {
		auto event_status = mission_get_event_status(event_idx);
		if ( (event_status != EventStatus::UNBORN) && !Mission_events[event_idx].objective_text.empty() && (timestamp_since(Mission_events[event_idx].born_on_date) > Directive_wait_time) ) {
			if (!Training_failure || !strnicmp(Mission_events[event_idx].name.c_str(), XSTR( "Training failed", 423), 15)) {

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
				if (!Mission_events[event_idx].objective_key_text.empty()) {
					if (event_status == EventStatus::CURRENT) {

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

	// Cyborg - Multiplayer clients will not run the other directive functions, so just run this quick check to see
	// if the directive success sound should be played.
	if (MULTIPLAYER_CLIENT) {
		if ( !hud_disabled() && hud_gauge_active(HUD_DIRECTIVES_VIEW) ) {
			mission_maybe_play_directive_success_sound();
		}
	}
}

/**
 * Do cleanup when leaving a mission
 */
void training_mission_shutdown()
{
	int i;

	if (Training_voice >= 0) {
		if (Training_voice_type) {
			audiostream_close_file(Training_voice_soundstream, false);

		} else {
			snd_stop(Training_voice_snd_handle);
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

	Training_buf.clear();
}

/**
 * Translates special tokens.  Handles one token only.
 */
char *translate_message_token(char *str)
{
	if (!stricmp(str, NOX("wp"))) {
		sprintf(str, "%d", Training_context_goal_waypoint + 1);
		return str;
	}

	return NULL;
}

/**
 * Translates all special tokens in a message, producing the new finalized message to be displayed
 */
SCP_string message_translate_tokens(const char *text)
{
	char temp[40];
	const char *ptr, *toke1, *toke2;
	int r;
	SCP_string buf;

	toke1 = strchr(text, '$');
	toke2 = strchr(text, '#');
	while (toke1 || toke2) {  // is either token types present?
		if (!toke2 || (toke1 && (toke1 < toke2))) {  // found $ before #
			buf.append(text, toke1 - text + 1);  // copy text up to token
			text = toke1 + 1;  // advance pointers past processed data

			toke2 = strchr(text, '$');
			if (!toke2 || ((toke2 - text) == 0))  // No second one, or possibly a double?
				break;

			// make sure we aren't going to have any type of out-of-bounds issues
			if ( (toke2 > text) && ((toke2 - text) < (ptr_s)sizeof(temp)) ) {
				strncpy(temp, text, toke2 - text);  // isolate token into seperate buffer
				temp[toke2 - text] = 0;  // null terminate string
				ptr = translate_key(temp);  // try and translate key
				if (ptr != nullptr) {  // was key translated properly?
					if (!stricmp(ptr, NOX("none")) && (Training_bind_warning != Missiontime)) {
						// check if a warning message should be displayed if the key is unbound
						if ( (The_mission.game_type & MISSION_TYPE_TRAINING) || (Always_warn_player_about_unbound_keys && (The_mission.game_type & MISSION_TYPE_SINGLE)) ) {
							r = popup(PF_TITLE_BIG | PF_TITLE_RED, 2, XSTR( "&Bind Control", 424), XSTR( "&Abort mission", 425),
								XSTR( "Warning\nYou have no control bound to the action \"%s\".  You must do so before you can continue with your training.", 426),
								XSTR(Control_config[Failed_key_index].text.c_str(), CONTROL_CONFIG_XSTR + Failed_key_index));

							if (r) {  // do they want to abort the mission?
								gameseq_post_event(GS_EVENT_END_GAME);
								return buf;
							}

							gameseq_post_event(GS_EVENT_CONTROL_CONFIG);  // goto control config screen to bind the control
						}
					}

					buf.pop_back();	// erase the $
					buf += ptr;		// put translated key in place of token
					text = toke2 + 1;
				}
			}
		} else {
			buf.append(text, toke2 - text + 1);  // copy text up to token
			text = toke2 + 1;  // advance pointers past processed data

			toke1 = strchr(text, '#');
			if (!toke1 || ((toke1 - text) == 0))  // No second one, or possibly a double?
				break;

			// make sure we aren't going to have any type of out-of-bounds issues
			if ( (toke1 > text) && ((toke1 - text) < (ptr_s)sizeof(temp)) ) {
				strncpy(temp, text, toke1 - text);  // isolate token into seperate buffer
				temp[toke1 - text] = 0;  // null terminate string
				ptr = translate_message_token(temp);  // try and translate key
				if (ptr) {  // was key translated properly?
					buf.pop_back();	// erase the #
					buf += ptr;		// put translated key in place of token
					text = toke1 + 1;
				}
			}
		}

		toke1 = strchr(text, '$');
		toke2 = strchr(text, '#');
	}

	buf += text;
	return buf;
}

/**
 * Plays the voice file associated with a training message.
 *
 * Automatically streams the file from disk if it's over 100k, otherwise plays it as 
 * a normal file in memory.  Returns -1 if it didn't play, otherwise index of voice
 */
int message_play_training_voice(int index)
{
	int len;
	CFILE *fp;

	if (index < 0) {
		if (Training_voice >= 0) {
			if (Training_voice_type) {
				audiostream_close_file(Training_voice_soundstream, false);

			} else {
				snd_stop(Training_voice_snd_handle);
			}
		}

		Training_voice = -1;
		return -1;
	}

	if (!Message_waves[index].num.isValid()) {
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
							audiostream_stop(Training_voice_soundstream, 1, 0);
						else
							audiostream_close_file(Training_voice_soundstream, false);

					} else {
						snd_stop(Training_voice_snd_handle);
					}
				}

				if (strnicmp(Message_waves[index].name, NOX("none.wav"), 4) != 0) {
					Training_voice_soundstream = audiostream_open(Message_waves[index].name, ASF_VOICE);
					if (Training_voice_soundstream < 0) {
						nprintf(("Warning", "Unable to load voice file %s\n", Message_waves[index].name));
					}
				}
			}  // Training_voice should be valid and loaded now

			Training_voice_type = 1;
			if (Training_voice_soundstream >= 0)
				audiostream_play(Training_voice_soundstream, (Master_voice_volume * aav_voice_volume), 0);

			Training_voice = index;
			return Training_voice;

		} else {
			game_snd_entry tmp_gse;
			strcpy_s(tmp_gse.filename, Message_waves[index].name);
			Message_waves[index].num = snd_load(&tmp_gse, nullptr, 0);
			if (!Message_waves[index].num.isValid()) {
				nprintf(("Warning", "Cannot load message wave: %s.  Will not play\n", Message_waves[index].name));
				return -1;
			}
		}
	}

	if (Training_voice >= 0) {
		if (Training_voice_type) {
			audiostream_close_file(Training_voice_soundstream, false);

		} else {
			snd_stop(Training_voice_snd_handle);
		}
	}

	Training_voice = index;
	if (Message_waves[index].num.isValid())
		Training_voice_snd_handle = snd_play_raw(Message_waves[index].num, 0.0f);
	else
		Training_voice_snd_handle = sound_handle::invalid();

	Training_voice_type = 0;
	return Training_voice;
}

/** 
 * One time initializations done when we want to display a new training mission.
 * 
 * This does all the processing and setup required to actually display it, including 
 * starting the voice file playing
 */
void message_training_setup(int m, int length, char *special_message)
{
	if ((m < 0) || !Messages[m].message[0]) {  // remove current message from the screen
		Training_num_lines = 0;
		return;
	}

	// translate tokens in message to the real things
	Training_buf = message_translate_tokens(special_message ? special_message : Messages[m].message);

	HUD_add_to_scrollback(Training_buf.c_str(), HUD_SOURCE_TRAINING);

	// moved from message_training_display() because we got rid of an extra buffer and we have to determine
	// the number of lines earlier to avoid inadvertant modification of Training_buf.  - taylor
	training_process_message();
	Training_num_lines = split_str(Training_buf.c_str(), TRAINING_LINE_WIDTH, Training_line_lengths, Training_lines, MAX_TRAINING_MESSAGE_LINES);

	Assert(Training_num_lines >= 0);

	if ((message_play_training_voice(Messages[m].wave_info.index) < 0) || (Master_voice_volume <= 0)) {
		if (length > 0)
			Training_message_timestamp = _timestamp(length * MILLISECONDS_PER_SECOND);
		else
			Training_message_timestamp = _timestamp(TRAINING_TIMING_BASE + (int)strlen(Messages[m].message) * TRAINING_TIMING);  // no voice file playing

	} else
		Training_message_timestamp = TIMESTAMP::invalid();
}

/**
 * Add a message to the queue to be sent later
 */
void message_training_queue(const char *text, TIMESTAMP timestamp, int length)
{
	int m;
	SCP_string temp_buf;

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
		// karajorma/jg18 - replace container references if necessary
		temp_buf = Messages[m].message;
		const bool replace_var = sexp_replace_variable_names_with_values(temp_buf);
		const bool replace_con = sexp_container_replace_refs_with_values(temp_buf);
		if (replace_var || replace_con)
			Training_message_queue[Training_message_queue_count].special_message = vm_strdup(temp_buf.c_str());

		Training_message_queue_count++;
	}
}

/**
 * Removes current message from the queue
 */
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
	Training_message_queue[Training_message_queue_count].timestamp = TIMESTAMP::invalid();
	Training_message_queue[Training_message_queue_count].special_message = NULL;	// not a memory leak because we copied the pointer
}

/**
 * Check the training message queue to see if we should play a new message yet or not.
 */
void message_training_queue_check()
{
	// get the instructor's ship.
	auto ship_entry = ship_registry_get(NOX("instructor"));
	if (ship_entry != nullptr)
	{
		if (ship_entry->has_shipp())
		{
			// if the instructor is dying or departing, do nothing
			if (ship_entry->shipp()->is_dying_or_departing())
				return;
		}
		else
		{
			// if the instructor has left the mission, or hasn't arrived yet, do nothing
			return;
		}
	}

	if (Training_failure)
		return;

	if (Dont_preempt_training_voice) {
		// don't pre-empt any voice files that are playing
		if (Training_voice >= 0) {
			auto z = (Training_voice_type) ? audiostream_is_playing(Training_voice_soundstream) : snd_is_playing(Training_voice_snd_handle);
			if (z)
				return;
		}
		if (fsspeech_playing())
			return;
	}

	for (int i=0; i<Training_message_queue_count; i++) {
		if (timestamp_elapsed(Training_message_queue[i].timestamp)) {
			message_training_setup(Training_message_queue[i].num, Training_message_queue[i].length, Training_message_queue[i].special_message);

			// remove this message from the queue now.
			message_training_remove_from_queue(i);
			i--;
		}
	}
}

void message_training_update_frame()
{
	int z;

	if ((Viewer_mode & (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY ))) {
		return;
	}
	
	if ( hud_disabled() && !hud_disabled_except_messages()) {
		return;
	}

	Training_message_visible = 0;
	message_training_queue_check();

	if (Training_failure){
		return;
	}

	if (timestamp_elapsed(Training_message_timestamp) || Training_buf.empty()){
		return;
	}

	// the code that preps the training message and counts the number of lines
	// has been moved to message_training_setup()

	if (Training_num_lines <= 0){
		return;
	}

	Training_message_visible = 1;

	if ((Training_voice >= 0) && (Training_num_lines > 0) && !(Training_message_timestamp.isValid())) {
		if (Training_voice_type)
			z = audiostream_is_playing(Training_voice_soundstream);
		else
			z = snd_is_playing(Training_voice_snd_handle);

		if (!z)
			Training_message_timestamp = _timestamp(2 * MILLISECONDS_PER_SECOND);  // 2 second delay
 	}

	Training_message_method = 0;
}

HudGaugeTrainingMessages::HudGaugeTrainingMessages():
HudGauge(HUD_OBJECT_TRAINING_MESSAGES, HUD_DIRECTIVES_VIEW, false, true, VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY | VM_OTHER_SHIP, 255, 255, 255)
{
}

bool HudGaugeTrainingMessages::canRender() const
{
	if (hud_disabled() && !hud_disabled_except_messages()) {
		return false;
	}
	
	if ( !(Game_detail_flags & DETAIL_FLAG_HUD) ) {
		return false;
	}

	if ((Viewer_mode & disabled_views)) {
		return false;
	}

	if(pop_up) {
		if(!popUpActive()) {
			return false;
		}
	}

	return true;
}

void HudGaugeTrainingMessages::pageIn()
{
}

/**
 * Displays (renders) the training message to the screen
 */
void HudGaugeTrainingMessages::render(float  /*frametime*/)
{
	const char *str;
	char buf[256];
	int i, z, x, y, height, mode, count;

	if (Training_failure){
		return;
	}

	if (timestamp_elapsed(Training_message_timestamp) || Training_buf.empty()){
		return;
	}

	if (Training_num_lines <= 0){
		return;
	}

	gr_set_screen_scale(base_w, base_h);
	height = gr_get_font_height();
	gr_set_shader(&Training_msg_glass);

	int nx = 0;
	int ny = 0;

	if (reticle_follow) {
		nx = HUD_nose_x;
		ny = HUD_nose_y;

		gr_reset_screen_scale();
		gr_resize_screen_pos(&nx, &ny);
		gr_set_screen_scale(base_w, base_h);
		gr_unsize_screen_pos(&nx, &ny);
	}

	gr_shade(position[0] + nx, position[1] + ny, TRAINING_MESSAGE_WINDOW_WIDTH, Training_num_lines * height + height);
	gr_reset_screen_scale();

	gr_set_color_fast(&Color_bright_blue);
	mode = count = 0;
	for (i=0; i<Training_num_lines; i++) {  // loop through all lines of message
		str = Training_lines[i];
		z = 0;
		x = position[0] + (TRAINING_MESSAGE_WINDOW_WIDTH - TRAINING_LINE_WIDTH) / 2;
		y = position[1] + i * height + height / 2 + 1;

		while ((str - Training_lines[i]) < Training_line_lengths[i]) {  // loop through each character of each line
			if ((count < MAX_TRAINING_MESSAGE_MODS) && (static_cast<size_t>(str - Training_lines[i]) == Training_message_mods[count].pos)) {
				buf[z] = 0;
				renderPrintf(x, y, "%s", buf);
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
			renderPrintf(x, y, "%s", buf);
		}
	}
}

/**
 * Processes a new training message to get hilighting information and store it in internal structures.
 */
void training_process_message()
{
	int count = 0;
	SCP_string orig = Training_buf;
	Training_buf.clear();
	auto src = orig.c_str();

	// basically what this is doing is re-copying the training message, marking out the points where it is bolded or un-bolded
	// (it does the re-copying in case it has to skip over the bold tags)
	while (*src) {
		if (!strnicmp(src, NOX("<b>"), 3)) {
			Assert(count < MAX_TRAINING_MESSAGE_MODS);
			src += 3;
			Training_message_mods[count].pos = Training_buf.size();
			Training_message_mods[count].mode = TMMOD_BOLD;
			count++;
		}

		if (!strnicmp(src, NOX("</b>"), 4)) {
			Assert(count < MAX_TRAINING_MESSAGE_MODS);
			src += 4;
			Training_message_mods[count].pos = Training_buf.size();
			Training_message_mods[count].mode = TMMOD_NORMAL;
			count++;
		}

		Training_buf += *src++;
	}

	if (count < MAX_TRAINING_MESSAGE_MODS)
		Training_message_mods[count].pos = 0;
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
