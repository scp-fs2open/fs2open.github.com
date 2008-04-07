/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/MissionSave.h $
 * $Revision: 1.3.2.2 $
 * $Date: 2007-11-21 07:27:46 $
 * $Author: Goober5000 $
 *
 * Mission saving in Fred.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3.2.1  2006/07/13 06:11:48  Goober5000
 * * better formatting for substitute music options
 * * better handling of all special FSO comment tags
 * --Goober5000
 *
 * Revision 1.3  2006/04/20 06:32:01  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 1.2  2006/02/19 00:49:41  Goober5000
 * fixed saving of special tags
 * --Goober5000
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.8  2005/10/30 06:23:23  Goober5000
 * multiple docking support for FRED
 * --Goober5000
 *
 * Revision 1.7  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.6  2005/04/13 20:02:08  chhogg
 * Changed vector to vec3d for Fred.
 *
 * Revision 1.5  2005/03/29 03:43:11  phreak
 * ai directory fixes as well as fixes for the new jump node code
 *
 * Revision 1.4  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.3  2004/03/15 12:14:46  randomtiger
 * Fixed a whole heap of problems with Fred introduced by changes to global vars.
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:00  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 4     1/19/99 3:57p Andsager
 * Round 2 of variables
 * 
 * 3     10/29/98 6:49p Dave
 * Finished up Fred support for externalizing mission and campaign files.
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 21    9/10/98 1:17p Dave
 * Put in code to flag missions and campaigns as being MD or not in Fred
 * and FreeSpace. Put in multiplayer support for filtering out MD
 * missions. Put in multiplayer popups for warning of non-valid missions.
 * 
 * 20    4/14/98 11:55a Allender
 * add end-of-campaign sexpression to allow for mission replay at the end
 * of campaigns
 * 
 * 19    3/05/98 3:59p Hoffoss
 * Added a bunch of new command brief stuff, and asteroid initialization
 * to Fred.
 * 
 * 18    9/30/97 5:56p Hoffoss
 * Added music selection combo boxes to Fred.
 * 
 * 17    8/25/97 5:56p Hoffoss
 * Added multiple asteroid field support, loading and saving of asteroid
 * fields, and ship score field to Fred.
 * 
 * 16    8/17/97 10:22p Hoffoss
 * Fixed several bugs in Fred with Undo feature.  In the process, recoded
 * a lot of CFile.cpp.
 * 
 * 15    6/17/97 3:01p Lawrance
 * allow FRED to save new briefing format
 * 
 * 14    6/09/97 4:57p Hoffoss
 * Added autosave and undo to Fred.
 * 
 * 13    6/05/97 6:10p Hoffoss
 * Added features: Autosaving, object hiding.  Also fixed some minor bugs.
 * 
 * 12    5/13/97 10:52a Hoffoss
 * Added campaign saving code.
 * 
 * 11    4/21/97 5:02p Hoffoss
 * Player/player status editing supported, and both saved and loaded from
 * Mission files.
 * 
 * 10    4/16/97 2:05p Hoffoss
 * Mission saving and loading of turret info now implemented.
 * 
 * 9     3/10/97 6:43p Hoffoss
 * Standardized docking goal usage by fred to use names instead of
 * indexes.
 * 
 * 8     2/05/97 2:57p Hoffoss
 * Added support for wing goals (initial orders) in Fred.
 * 
 * 7     1/30/97 2:24p Hoffoss
 * Added remaining mission file structures and implemented load/save of
 * them.
 *
 * $NoKeywords: $
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
#include <vector>

#define BACKUP_DEPTH	9

class CFred_mission_save {
private:
	char *raw_ptr;
	std::vector<std::string> fso_ver_comment;
	int err;
	CFILE *fp;

	int save_mission_info();
	int save_plot_info();
	int save_variables();
//	int save_briefing_info();
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
	int save_waypoint_list(waypoint_list &w);
	int save_vector(vec3d &v);
	int save_matrix(matrix &m);
	int save_messages();
	int save_events();
	int save_asteroid_fields();
	int save_music();
	void save_campaign_sexp(int node, int link);
	void save_single_dock_instance(ship *shipp, dock_instance *dock_ptr);

	void convert_special_tags_to_retail(char *text, int max_len);
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
