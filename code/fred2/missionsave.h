/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __MISSION_SAVE_CPP__
#define __MISSION_SAVE_CPP__

#include <stdio.h>
#include "mission/missionparse.h"
#include "parse/parselo.h"
#include "ai/ailocal.h"
#include "ai/ai.h"
#include "cfile/cfile.h"
#include "ship/ship.h"
#include "object/waypoint.h"

#include <string>

#define BACKUP_DEPTH	9

class CFred_mission_save {
private:
	char *raw_ptr;
	SCP_vector<SCP_string> fso_ver_comment;
	int err;
	CFILE *fp;

	int save_mission_info();
	int save_plot_info();
	int save_variables();
//	int save_briefing_info();
	int save_cutscenes();
	int save_fiction();
	int save_cmd_brief();
	int save_cmd_briefs();
	int save_briefing();
	int save_debriefing();
	int save_players();
	int save_objects();
	int save_common_object_data(object *objp, ship *shipp);
	int save_wings();
	int save_goals();
	int save_waypoints();
	int save_waypoint_list(waypoint_list *w);
	int save_vector(vec3d &v);
	int save_matrix(matrix &m);
	int save_messages();
	int save_events();
	int save_asteroid_fields();
	int save_music();
	void save_campaign_sexp(int node, int link);
	void save_single_dock_instance(ship *shipp, dock_instance *dock_ptr);

	void convert_special_tags_to_retail(char *text, int max_len);
	void convert_special_tags_to_retail(SCP_string &text);
	void convert_special_tags_to_retail();

public:
	void save_turret_info(ship_subsys *ptr, int ship);
	int save_bitmaps();
	int save_reinforcements();
	void save_ai_goals(ai_goal *goalp, int ship);
	int fout(char *format, ...);
	int fout_version(char *format, ...);
	int fout_ext(char *pre_str, char *format, ...);
	void parse_comments(int = 1);
	CFred_mission_save() : err(0), raw_ptr(Mission_text_raw) { }
	int save_mission_file(char *pathname);
	int autosave_mission_file(char *pathname);
	int save_campaign_file(char *pathname);		

	void fso_comment_push(char *ver);
	void fso_comment_pop(bool pop_all = false);

	// Goober5000
	void bypass_comment(char *comment);
};

#endif
