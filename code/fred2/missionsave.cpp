/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Fred2/MissionSave.cpp $
 * $Revision: 1.39 $
 * $Date: 2007-12-30 18:30:28 $
 * $Author: karajorma $
 *
 * Mission saving in Fred.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.38  2007/11/21 07:28:37  Goober5000
 * add Wing Commander Saga's fiction viewer
 *
 * Revision 1.37  2007/09/03 01:02:49  Goober5000
 * fix for 1376
 *
 * Revision 1.36  2007/07/23 15:16:48  Kazan
 * Autopilot upgrades as described, MSVC2005 project fixes
 *
 * Revision 1.35  2007/02/21 01:44:02  Goober5000
 * remove duplicate model texture replacement
 *
 * Revision 1.34  2007/02/20 04:20:10  Goober5000
 * the great big duplicate model removal commit
 *
 * Revision 1.33  2007/02/11 21:26:34  Goober5000
 * massive shield infrastructure commit
 *
 * Revision 1.32  2007/01/15 13:42:59  karajorma
 * Hmmm. Forgot to commit changes to support network variables and setting ammo/weapons to HEAD as well as 3.6.9.
 *
 * Revision 1.31  2007/01/07 21:28:10  Goober5000
 * yet more tweaks to the WCS death scream stuff
 * added a ship flag to force screaming
 *
 * Revision 1.30  2007/01/07 01:00:18  Goober5000
 * convert a mission variable to a mission flag
 *
 * Revision 1.29  2007/01/07 00:24:17  Goober5000
 * move this to the Message location of the mission
 *
 * Revision 1.28  2007/01/07 00:01:28  Goober5000
 * add a feature for specifying the source of Command messages
 *
 * Revision 1.27  2006/10/15 22:01:38  wmcoolmon
 * Fix extra jumpnode settings not saving. They still don't show up in FRED, unfortunately.
 *
 * Revision 1.26  2006/10/09 05:25:18  Goober5000
 * make sexp nodes dynamic
 *
 * Revision 1.25  2006/10/01 06:18:35  Goober5000
 * more accurate message number
 *
 * Revision 1.24  2006/09/28 23:47:24  Goober5000
 * fix for Mantis #1070
 *
 * Revision 1.23  2006/08/19 21:46:05  Goober5000
 * disable duplicate model texture replace
 *
 * Revision 1.22  2006/08/06 18:47:29  Goober5000
 * add the multiple background feature
 * --Goober5000
 *
 * Revision 1.21  2006/07/13 06:11:52  Goober5000
 * * better formatting for substitute music options
 * * better handling of all special FSO comment tags
 * --Goober5000
 *
 * Revision 1.20  2006/07/06 21:00:12  Goober5000
 * remove obsolete (and hackish) flag
 * --Goober5000
 *
 * Revision 1.19  2006/07/06 20:46:39  Goober5000
 * WCS screaming stuff
 * --Goober5000
 *
 * Revision 1.18  2006/06/23 09:17:02  karajorma
 * Make the ambient light sliders actually save their settings.
 *
 * Revision 1.17  2006/06/10 18:34:07  Goober5000
 * fix parsing/handling of debriefing persona indexes
 * --Goober5000
 *
 * Revision 1.16  2006/06/04 01:01:52  Goober5000
 * add fighterbay restriction code
 * --Goober5000
 *
 * Revision 1.15  2006/06/02 09:40:33  karajorma
 * Team Loadout from variable changes. Added alt ship classes
 *
 * Revision 1.14  2006/05/30 01:36:24  Goober5000
 * add AI Profile box to FRED
 * --Goober5000
 *
 * Revision 1.13  2006/05/14 15:52:42  karajorma
 * Fixes for the Invulnerability problems from Mantis - 0000910
 *
 * Revision 1.12  2006/04/20 06:32:01  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 1.11  2006/02/28 07:37:12  Goober5000
 * fix another import bug
 *
 * Revision 1.10  2006/02/26 23:23:30  wmcoolmon
 * Targetable as bomb SEXPs and dialog stuff; made invulnerable an object flag in both FRED and FS2.
 *
 * Revision 1.9  2006/02/20 02:13:07  Goober5000
 * added ai-ignore-new which hopefully should fix the ignore bug
 * --Goober5000
 *
 * Revision 1.8  2006/02/19 00:49:41  Goober5000
 * fixed saving of special tags
 * --Goober5000
 *
 * Revision 1.7  2006/02/12 10:42:25  Goober5000
 * allow specification of debriefing personas
 * --Goober5000
 *
 * Revision 1.6  2006/02/12 05:23:16  Goober5000
 * additional fixes and enhancements for substitute music
 * --Goober5000
 *
 * Revision 1.5  2006/02/12 01:27:47  Goober5000
 * more cool work on importing, music handling, etc.
 * --Goober5000
 *
 * Revision 1.4  2006/02/11 02:58:23  Goober5000
 * yet more various and sundry fixes
 * --Goober5000
 *
 * Revision 1.3  2006/01/30 06:27:59  taylor
 * dynamic starfield bitmaps
 *
 * Revision 1.2  2006/01/26 04:01:58  Goober5000
 * spelling
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.50  2006/01/14 23:49:01  Goober5000
 * second pass; all the errors are fixed now; one more thing to take care of
 * --Goober5000
 *
 * Revision 1.49  2006/01/14 05:24:18  Goober5000
 * first pass at converting FRED to use new IFF stuff
 * --Goober5000
 *
 * Revision 1.48  2005/12/29 08:21:00  wmcoolmon
 * No my widdle FRED, I didn't forget about you ^_^ (codebase commit)
 *
 * Revision 1.47  2005/12/29 00:55:25  phreak
 * Briefing icons are now able to be flipped.  Left becomes right and vice versa.
 *
 * Revision 1.46  2005/11/22 09:07:59  Goober5000
 * fred will now load and save ai profiles
 * --Goober5000
 *
 * Revision 1.45  2005/10/31 09:20:14  Goober5000
 * generalize
 * --Goober5000
 *
 * Revision 1.44  2005/10/30 06:23:23  Goober5000
 * multiple docking support for FRED
 * --Goober5000
 *
 * Revision 1.43  2005/10/29 23:02:33  Goober5000
 * partial FRED commit of changes
 * --Goober5000
 *
 * Revision 1.42  2005/09/09 22:31:12  Goober5000
 * this ought to take care of a FRED bug
 * --Goober5000
 *
 * Revision 1.41  2005/07/25 06:51:33  Goober5000
 * mission flag changes for FRED
 * --Goober5000
 *
 * Revision 1.40  2005/04/13 20:11:06  chhogg
 * Changed vector to vec3d for Fred.
 *
 * Revision 1.38  2005/03/29 03:43:11  phreak
 * ai directory fixes as well as fixes for the new jump node code
 *
 * Revision 1.37  2005/03/27 12:59:13  Goober5000
 * stuff for FRED
 * --Goober5000
 *
 * Revision 1.36  2005/03/06 23:19:27  wmcoolmon
 * Fixed the last of the jump node errors
 *
 * Revision 1.35  2005/01/29 06:28:15  Goober5000
 * bugfixes
 * --Goober5000
 *
 * Revision 1.34  2005/01/29 05:40:27  Goober5000
 * regular docking should work again in FRED now (multiple docking still isn't done yet)
 * --Goober5000
 *
 * Revision 1.33  2004/12/31 18:21:23  taylor
 * fix loadout problem with first bank weaponry and single bank ships
 *
 * Revision 1.32  2004/12/15 19:16:13  Goober5000
 * FRED code for custom wing names
 * --Goober5000
 *
 * Revision 1.31  2004/10/12 22:48:40  Goober5000
 * added toggle-subsystem-scanning ship flag
 * --Goober5000
 *
 * Revision 1.30  2004/10/12 07:45:30  Goober5000
 * added contrail speed threshold
 * --Goober5000
 *
 * Revision 1.29  2004/10/11 22:31:23  Goober5000
 * teh FRED implementation
 * --Goober5000
 *
 * Revision 1.28  2004/09/17 07:56:51  Goober5000
 * bunch of FRED tweaks and fixes
 * --Goober5000
 *
 * Revision 1.27  2004/07/29 00:18:43  Kazan
 * taylor and I fixed fred2 by making code.lib and fred2's defines match up
 *
 * Revision 1.26  2004/05/11 02:17:13  Goober5000
 * fixed some save stuff
 * --Goober5000
 *
 * Revision 1.25  2004/04/29 03:35:18  wmcoolmon
 * Added Hull and Subsys repair ceiling fields to Mission Specs Dialog in FRED
 * Updated mission saving code to store both variables properly
 *
 * Revision 1.24  2004/03/15 12:14:46  randomtiger
 * Fixed a whole heap of problems with Fred introduced by changes to global vars.
 *
 * Revision 1.23  2004/02/13 04:25:37  randomtiger
 * Added messagebox to offer startup choice between htl, non htl and quitting.
 * Tidied up the background dialog box a bit and added some disabled controls for future ambient light feature.
 * Set non htl to use a lower level of model and texture detail to boost poor performance on this path.
 *
 * Revision 1.22  2004/02/05 23:23:19  randomtiger
 * Merging OGL Fred into normal Fred, no conflicts, should be perfect
 *
 * Revision 1.21.2.2  2004/02/05 01:50:46  randomtiger
 * Addition to about dialog and other small changes
 *
 * Revision 1.21.2.1  2004/01/17 22:11:25  randomtiger
 * Large series of changes to make fred run though OGL. Requires code.lib to be up to date to compile.
 * Port still in progress, fred currently unusable, note this has been committed to fred_ogl branch.
 *
 * Revision 1.21  2004/01/14 06:28:39  Goober5000
 * made set-support-ship number align with general FS convention
 * --Goober5000
 *
 * Revision 1.20  2004/01/14 06:03:14  Goober5000
 * added save capability to the hull repair and subsys repair ceiling features
 * by WMCoolmon... but he still has to incorporate the dialog for them
 * --Goober5000
 *
 * Revision 1.19  2003/10/23 23:50:28  phreak
 * added ability for FRED to save info about user-defined skybox
 *
 * Revision 1.18  2003/09/05 10:55:16  Goober5000
 * bah
 * --Goober5000
 *
 * Revision 1.17  2003/09/05 10:04:33  Goober5000
 * persistent variable checkboxes in FRED
 * --Goober5000
 *
 * Revision 1.16  2003/08/26 20:32:41  Goober5000
 * added support for saving in FS2 retail format
 * --Goober5000
 *
 * Revision 1.15  2003/05/10 22:35:25  phreak
 * fixed minor save bug when dealing with custom loading screens
 *
 * Revision 1.14  2003/05/10 00:31:34  phreak
 * custom loading screens are now saved in the .fs2 file
 *
 * Revision 1.13  2003/04/29 01:04:28  Goober5000
 * custom hitpoints for FRED
 * --Goober5000
 *
 * Revision 1.12  2003/03/25 07:03:29  Goober5000
 * added beginning functionality for $Texture Replace implementation in FRED
 * --Goober5000
 *
 * Revision 1.11  2003/03/03 04:15:24  Goober5000
 * FRED portion of tech room fix, plus some neatening up of the dialogs
 * --Goober5000
 *
 * Revision 1.10  2003/01/19 07:02:15  Goober5000
 * fixed a bunch of bugs - "no-subspace-drive" should now work properly for
 * all ships, and all ships who have their departure anchor set to a capital ship
 * should exit to that ship when told to depart
 * --Goober5000
 *
 * Revision 1.9  2003/01/18 10:00:43  Goober5000
 * added "no-subspace-drive" flag for ships
 * --Goober5000
 *
 * Revision 1.8  2003/01/14 04:00:15  Goober5000
 * allowed for up to 256 main halls
 * --Goober5000
 *
 * Revision 1.7  2003/01/06 20:49:16  Goober5000
 * FRED2 support for wing squad logos - look in the wing editor
 * --Goober5000
 *
 * Revision 1.6  2003/01/05 05:41:17  Goober5000
 * fixed newline
 * --Goober5000
 *
 * Revision 1.5  2003/01/03 21:58:07  Goober5000
 * Fixed some minor bugs, and added a primitive-sensors flag, where if a ship
 * has primitive sensors it can't target anything and objects don't appear
 * on radar if they're outside a certain range.  This range can be modified
 * via the sexp primitive-sensors-set-range.
 * --Goober5000
 *
 * Revision 1.4  2002/12/24 05:55:46  Goober5000
 * silly newline fix when saving mission flags
 * --Goober5000
 *
 * Revision 1.3  2002/08/15 04:35:44  penguin
 * Changes to build with fs2_open code.lib
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:00  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 33    8/28/99 7:29p Dave
 * Fixed wingmen persona messaging. Make sure locked turrets don't count
 * towards the # attacking a player.
 * 
 * 32    8/27/99 12:04a Dave
 * Campaign loop screen.
 * 
 * 31    8/26/99 8:52p Dave
 * Gave multiplayer TvT messaging a heavy dose of sanity. Cheat codes.
 * 
 * 30    8/17/99 5:20p Andsager
 * Control campaign editor bug.
 * 
 * 29    8/16/99 3:53p Andsager
 * Add special warp in interface in Fred and saving / reading.
 * 
 * 28    7/02/99 4:30p Dave
 * Much more sophisticated lightning support.
 * 
 * 27    6/10/99 11:06a Andsager
 * Mission designed selection of asteroid types.
 * 
 * 26    6/03/99 6:37p Dave
 * More TNT fun. Made perspective bitmaps more flexible.
 * 
 * 25    5/20/99 6:59p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 24    5/09/99 6:00p Dave
 * Lots of cool new effects. E3 build tweaks.
 * 
 * 23    4/26/99 8:47p Dave
 * Made all pof related nebula stuff customizable through Fred.
 * 
 * 22    4/26/99 12:49p Andsager
 * Add protect object from beam support to Fred
 * 
 * 21    4/16/99 2:34p Andsager
 * Second pass on debris fields
 * 
 * 20    4/15/99 5:00p Andsager
 * Frist pass on Debris field
 * 
 * 19    4/07/99 6:21p Dave
 * Fred and FreeSpace support for multiple background bitmaps and suns.
 * Fixed link errors on all subprojects. Moved encrypt_init() to
 * cfile_init() and lcl_init(), since its safe to call twice.
 * 
 * 18    3/31/99 9:50a Andsager
 * Interface for generalization of asteroid field (debris field)
 * 
 * 17    3/24/99 4:05p Dave
 * Put in support for assigning the player to a specific squadron with a
 * specific logo. Preliminary work for doing pos/orient checksumming in
 * multiplayer to reduce bandwidth.
 * 
 * 16    3/01/99 7:39p Dave
 * Added prioritizing ship respawns. Also fixed respawns in TvT so teams
 * don't mix respawn points.
 * 
 * 15    2/26/99 6:01p Andsager
 * Add sexp has-been-tagged-delay and cap-subsys-cargo-known-delay
 * 
 * 14    2/17/99 2:11p Dave
 * First full run of squad war. All freespace and tracker side stuff
 * works.
 * 
 * 13    2/11/99 2:15p Andsager
 * Add ship explosion modification to FRED
 * 
 * 12    2/07/99 8:51p Andsager
 * Add inner bound to asteroid field.  Inner bound tries to stay astroid
 * free.  Wrap when within and don't throw at ships inside.
 * 
 * 11    2/03/99 12:42p Andsager
 * Add escort priority.  Modify ship_flags_dlg to include field.  Save and
 * Load.  Add escort priority field to ship.
 * 
 * 10    1/25/99 5:03a Dave
 * First run of stealth, AWACS and TAG missile support. New mission type
 * :)
 * 
 * 9     1/19/99 3:57p Andsager
 * Round 2 of variables
 * 
 * 8     1/07/99 1:52p Andsager
 * Initial check in of Sexp_variables
 * 
 * 7     12/17/98 2:43p Andsager
 * Modify fred campaign save file to include optional mission loops
 * 
 * 6     11/14/98 5:37p Dave
 * Put in support for full nebulas.
 * 
 * 5     11/05/98 4:18p Dave
 * Modified mission file format.
 * 
 * 4     10/29/98 9:22p Dave
 * Removed minor bug concering externalization of campaign files.
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
 * 207   9/16/98 10:42a Hoffoss
 * Added sexp node counting to fsm files for end user designers.
 * 
 * 206   9/15/98 7:24p Dave
 * Minor UI changes. Localized bunch of new text.
 * 
 * 205   9/15/98 11:44a Dave
 * Renamed builtin ships and wepaons appropriately in FRED. Put in scoring
 * scale factors. Fixed standalone filtering of MD missions to non-MD
 * hosts.
 * 
 * 204   9/10/98 1:17p Dave
 * Put in code to flag missions and campaigns as being MD or not in Fred
 * and FreeSpace. Put in multiplayer support for filtering out MD
 * missions. Put in multiplayer popups for warning of non-valid missions.
 * 
 * 203   5/21/98 12:58a Hoffoss
 * Fixed warnings optimized build turned up.
 * 
 * 202   5/20/98 1:04p Hoffoss
 * Made credits screen use new artwork and removed rating field usage from
 * Fred (a goal struct member).
 * 
 * 201   5/19/98 1:19p Allender
 * new low level reliable socket reading code.  Make all missions/campaign
 * load/save to data missions folder (i.e. we are rid of the player
 * missions folder)
 * 
 * 200   5/13/98 5:13p Allender
 * red alert support to go back to previous mission
 * 
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "freespace2/freespace.h"
#include "MissionSave.h"
#include "mission/missiongoals.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"
#include "FredRender.h"
#include "ai/aigoals.h"
#include "starfield/starfield.h"
#include "lighting/lighting.h"
#include "globalincs/linklist.h"
#include "weapon/weapon.h"
#include "mission/missioncampaign.h"
#include "CampaignTreeWnd.h"
#include "CampaignTreeView.h"
#include "CampaignEditorDlg.h"
#include "parse/sexp.h"
#include "mission/missionbriefcommon.h"
#include "Management.h"
#include "gamesnd/eventmusic.h"
#include "starfield/nebula.h"
#include "asteroid/asteroid.h"
#include "missionui/missioncmdbrief.h"
#include "jumpnode/jumpnode.h"
#include "MainFrm.h"
#include "localization/fhash.h"
#include "nebula/neb.h"
#include "osapi/osapi.h"
#include "FredView.h"
#include "cfile/cfile.h"
#include "object/objectdock.h"
#include "object/objectshield.h"
#include "iff_defs/iff_defs.h"
#include "missionui/fictionviewer.h"

void CFred_mission_save::convert_special_tags_to_retail(char *text, int max_len)
{
	replace_all(Mp, "$quote", "''", max_len);
	replace_all(Mp, "$semicolon", ",", max_len);
}

// Goober5000 - convert $quote and $semicolon to '' and ,
void CFred_mission_save::convert_special_tags_to_retail()
{
	int i, team, stage;

	if (Format_fs2_open)
		return;

	for (team = 0; team < Num_teams; team++)
	{
		// command briefing
		for (stage = 0; stage < Cmd_briefs[team].num_stages; stage++)
		{
			convert_special_tags_to_retail(Cmd_briefs[team].stage[stage].text, CMD_BRIEF_TEXT_MAX);
		}

		// briefing
		for (stage = 0; stage < Briefings[team].num_stages; stage++)
		{
			convert_special_tags_to_retail(Briefings[team].stages[stage].new_text, MAX_BRIEF_LEN);
		}

		// debriefing
		for (stage = 0; stage < Debriefings[team].num_stages; stage++)
		{
			convert_special_tags_to_retail(Debriefings[team].stages[stage].new_text, MAX_DEBRIEF_LEN);
		}
	}

	for (i = Num_builtin_messages; i < Num_messages; i++)
	{
		convert_special_tags_to_retail(Messages[i].message, MESSAGE_LENGTH);
	}
}

int CFred_mission_save::save_mission_file(char *pathname)
{
	char backup_name[256], savepath[MAX_PATH_LEN], *p;
	CTime t;

	t = CTime::GetCurrentTime();
	strcpy(The_mission.modified, t.Format("%x at %X"));

	strcpy(savepath, "");
	p = strrchr(pathname, '\\');
	if ( p ) {
		*p = '\0';
		strcpy(savepath, pathname);
		*p = '\\';
		strcat(savepath, "\\");
	}
	strcat(savepath, "saving.xxx");

	reset_parse();
	fred_parse_flag = 0;
	fp = cfopen(savepath, "wt", CFILE_NORMAL);
	if (!fp)	{
		nprintf(("Error", "Can't open mission file to save.\n"));
		return -1;
	}	

	// Goober5000
	convert_special_tags_to_retail();

	if (save_mission_info())
		err = -2;
	else if (save_plot_info())
		err = -3;
	else if (save_variables())
		err = -3;
//	else if (save_briefing_info())
//		err = -4;
	else if (save_cmd_briefs())
		err = -4;
	else if (save_briefing())
		err = -4;
	else if (save_debriefing())
		err = -5;
	else if (save_players())
		err = -6;
	else if (save_objects())
		err = -7;
	else if (save_wings())
		err = -8;
	else if (save_events())
		err = -9;
	else if (save_goals())
		err = -10;
	else if (save_waypoints())
		err = -11;
	else if (save_messages())
		err = -12;
	else if (save_reinforcements())
		err = -13;
	else if (save_bitmaps())
		err = -14;
	else if (save_asteroid_fields())
		err = -15;
	else if (save_music())
		err = -16;
	else {
		required_string_fred("#End");
		parse_comments(2);
		token_found = NULL;
		parse_comments();
		fout("\n");
	}

	cfclose(fp);
	if (err) {
		mprintf(("Mission saving error code #%d", err));

	} else {
		strcpy(backup_name, pathname);
		if (backup_name[strlen(backup_name) - 4] == '.')
			backup_name[strlen(backup_name) - 4] = 0;

		strcat(backup_name, ".bak");
		cf_attrib(pathname, 0, FILE_ATTRIBUTE_READONLY, CF_TYPE_MISSIONS);
		cf_delete(backup_name, CF_TYPE_MISSIONS);
		cf_rename(pathname, backup_name, CF_TYPE_MISSIONS);
		cf_rename(savepath, pathname, CF_TYPE_MISSIONS);
	}

	return err;
}

int CFred_mission_save::autosave_mission_file(char *pathname)
{
	char backup_name[256], name2[256];
	int i, len;
	CTime t;
	
	t = CTime::GetCurrentTime();
	strcpy(The_mission.modified, t.Format("%x at %X"));

	len = strlen(pathname);
	strcpy(backup_name, pathname);
	strcpy(name2, pathname);
	sprintf(backup_name + len, ".%.3d", BACKUP_DEPTH);
	cf_delete(backup_name, CF_TYPE_MISSIONS);
	for (i=BACKUP_DEPTH; i>1; i--) {
		sprintf(backup_name + len, ".%.3d", i - 1);
		sprintf(name2 + len, ".%.3d", i);
		cf_rename(backup_name, name2, CF_TYPE_MISSIONS);
	}
	
	strcpy(backup_name + len, ".001");
	reset_parse();
	fred_parse_flag = 0;
	fp = cfopen(backup_name, "wt", CFILE_NORMAL, CF_TYPE_MISSIONS);
	if (!fp)	{
		nprintf(("Error", "Can't open mission file to save.\n"));
		return -1;
	}

	// Goober5000
	convert_special_tags_to_retail();

	if (save_mission_info())
		err = -2;
	else if (save_plot_info())
		err = -3;
	else if (save_variables())
		err = -3;
//	else if (save_briefing_info())
//		err = -4;
	else if (save_cmd_briefs())
		err = -4;
	else if (save_briefing())
		err = -4;
	else if (save_debriefing())
		err = -5;
	else if (save_players())
		err = -6;
	else if (save_objects())
		err = -7;
	else if (save_wings())
		err = -8;
	else if (save_events())
		err = -9;
	else if (save_goals())
		err = -10;
	else if (save_waypoints())
		err = -11;
	else if (save_messages())
		err = -12;
	else if (save_reinforcements())
		err = -13;
	else if (save_bitmaps())
		err = -14;
	else if (save_asteroid_fields())
		err = -15;
	else if (save_music())
		err = -16;
	else {
		required_string_fred("#End");
		parse_comments(2);
		token_found = NULL;
		parse_comments();
		fout("\n");
	}

	cfclose(fp);
	if (err)
		mprintf(("Mission saving error code #%d", err));

	return err;
}

int CFred_mission_save::save_mission_info()
{
	required_string_fred("#Mission Info");
	parse_comments(0);

	required_string_fred("$Version:");
	parse_comments(2);
	fout(" %.2f", FRED_MISSION_VERSION);

	// XSTR
	required_string_fred("$Name:");
	parse_comments();
	fout(" ");
	fout_ext("%s", The_mission.name);

	required_string_fred("$Author:");
	parse_comments();
	fout(" %s", The_mission.author);

	required_string_fred("$Created:");
	parse_comments();
	fout(" %s", The_mission.created);

	required_string_fred("$Modified:");
	parse_comments();
	fout(" %s", The_mission.modified);

	required_string_fred("$Notes:");
	parse_comments();
	fout("\n%s", The_mission.notes);

	required_string_fred("$End Notes:");
	parse_comments(0);

	// XSTR
	required_string_fred("$Mission Desc:");
	parse_comments(2);
	fout("\n");
	fout_ext("%s", The_mission.mission_desc);
	fout("\n");

	required_string_fred("$end_multi_text");
	parse_comments(0);

#if 0
	if (optional_string_fred("+Game Type:"))
		parse_comments(2);
	else
		fout("\n\n+Game Type:");
	fout("\n%s", Game_types[The_mission.game_type]);	
#endif		

	if ( optional_string_fred("+Game Type Flags:")){
		parse_comments(2);
	} else {
		fout("\n+Game Type Flags:");
	}	

	fout(" %d", The_mission.game_type);

	if (optional_string_fred("+Flags:")){
		parse_comments(2);
	} else {
		fout("\n+Flags:");
	}

	fout(" %d", The_mission.flags);

	// maybe write out Nebula intensity
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		Assert(Neb2_awacs > 0.0f);
		fout("\n+NebAwacs: %f\n", Neb2_awacs);

		// storm name
		fout("\n+Storm: %s\n", Mission_parse_storm_name);
	}

	// Goober5000
	if (Format_fs2_open)
	{
		if (The_mission.contrail_threshold != CONTRAIL_THRESHOLD_DEFAULT)
		{
			fout("\n$Contrail Speed Threshold: %d\n", The_mission.contrail_threshold);
		}
	}

	// For multiplayer missions -- write out the number of player starts and number of respawns
	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		if (optional_string_fred("+Num Players:"))
			parse_comments(2);
		else
			fout("\n+Num Players:");

		fout(" %d", Player_starts);

		if (optional_string_fred("+Num Respawns:"))
			parse_comments(2);
		else
			fout("\n+Num Respawns:");

		fout(" %d", The_mission.num_respawns);
	}

	if (!Format_fs2_open)
	{
		if ( optional_string_fred("+Red Alert:"))
			parse_comments(2);
		else
			fout("\n+Red Alert:");

		fout(" %d", (The_mission.flags & MISSION_FLAG_RED_ALERT) ? 1 : 0);
	}

	if (!Format_fs2_open)
	{
		if ( optional_string_fred("+Scramble:"))
			parse_comments(2);
		else
			fout("\n+Scramble:");

		fout(" %d", (The_mission.flags & MISSION_FLAG_SCRAMBLE) ? 1 : 0);
	}

	if ( optional_string_fred("+Disallow Support:")){
		parse_comments(2);
	} else {
		fout("\n+Disallow Support:");
	}
	// this is compatible with non-SCP variants - Goober5000
	fout(" %d", (The_mission.support_ships.max_support_ships == 0)?1:0 );

	// here be WMCoolmon's hull and subsys repair stuff
	if (Format_fs2_open)
	{
		if ( optional_string_fred("+Hull Repair Ceiling:")) {
			parse_comments(2);
		} else {
			fout("\n+Hull Repair Ceiling:");
		}
		fout(" %f", The_mission.support_ships.max_hull_repair_val);

		if ( optional_string_fred("+Subsystem Repair Ceiling:")) {
			parse_comments(2);
		} else {
			fout("\n+Subsystem Repair Ceiling:");
		}
		fout(" %f", The_mission.support_ships.max_subsys_repair_val);
	}

	if (!Format_fs2_open && (The_mission.flags & MISSION_FLAG_ALL_ATTACK))
	{
		if (optional_string_fred("+All Teams Attack"))
			parse_comments();
		else
			fout("\n+All Teams Attack");
	}

	if (Entry_delay_time) {
		if (optional_string_fred("+Player Entry Delay:"))
			parse_comments(2);
		else
			fout("\n\n+Player Entry Delay:");

		fout("\n%f", f2fl(Entry_delay_time));
	}

	if (optional_string_fred("+Viewer pos:")){
		parse_comments(2);
	} else {
		fout("\n\n+Viewer pos:");
	}

	save_vector(view_pos);

	if (optional_string_fred("+Viewer orient:")){
		parse_comments();
	} else {
		fout("\n+Viewer orient:");
	}

	save_matrix(view_orient);

	// squadron info
	if(!(The_mission.game_type & MISSION_TYPE_MULTI) && (strlen(The_mission.squad_name) > 0)){
		// squad name
		fout("\n+SquadReassignName: %s", The_mission.squad_name);

		// maybe squad logo
		if(strlen(The_mission.squad_filename) > 0){
			fout("\n+SquadReassignLogo: %s", The_mission.squad_filename);
		}
	}

	// Goober5000 - special wing info
	if (Format_fs2_open)
	{
		int i;
		fout("\n");

		// starting wings
		if (strcmp(Starting_wing_names[0], "Alpha") || strcmp(Starting_wing_names[1], "Beta") || strcmp(Starting_wing_names[2], "Gamma"))
		{
			fout("\n$Starting wing names: ( ");

			for (i=0; i<MAX_STARTING_WINGS; i++)
			{
				fout ("\"%s\" ", Starting_wing_names[i]);
			}

			fout (")");
		}

		// squadron wings
		if (strcmp(Squadron_wing_names[0], "Alpha") || strcmp(Squadron_wing_names[1], "Beta") || strcmp(Squadron_wing_names[2], "Gamma") || strcmp(Squadron_wing_names[3], "Delta") || strcmp(Squadron_wing_names[4], "Epsilon"))
		{
			fout("\n$Squadron wing names: ( ");

			for (i=0; i<MAX_SQUADRON_WINGS; i++)
			{
				fout("\"%s\" ", Squadron_wing_names[i]);
			}

			fout(")");
		}

		// tvt wings
		if (strcmp(TVT_wing_names[0], "Alpha") || strcmp(TVT_wing_names[1], "Zeta"))
		{
			fout("\n$Team-versus-team wing names: ( ");

			for (i=0; i<MAX_TVT_WINGS; i++)
			{
				fout("\"%s\" ", TVT_wing_names[i]);
			}

			fout(")");
		}
	}

	// Phreak's loading screen stuff
	if (Format_fs2_open)
	{
		if (strlen(The_mission.loading_screen[GR_640]) > 0)
		{
			fout("\n\n$Load Screen 640:\t%s",The_mission.loading_screen[GR_640]);
		}

		if (strlen(The_mission.loading_screen[GR_1024]) > 0)
		{
			fout("\n$Load Screen 1024:\t%s",The_mission.loading_screen[GR_1024]);
		}
	}

	// Phreak's skybox stuff
	if (strlen(The_mission.skybox_model) > 0)
	{
		char out_str[NAME_LENGTH];
		char *period;

		// kill off any extension, we will add one here
		strcpy(out_str, The_mission.skybox_model);
		period = strchr(out_str, '.');
		if (period != NULL)
			*period = 0;

		fout_and_bypass("\n\n;;FSO 3.6.0;; $Skybox Model: %s.pof", out_str);
	}

	// Goober5000's AI profile stuff
	if (Format_fs2_open)
	{
		int profile_index = (The_mission.ai_profile - Ai_profiles);
		Assert(profile_index >= 0 && profile_index < MAX_AI_PROFILES);

		fout("\n\n$AI Profile: %s", The_mission.ai_profile->profile_name);
	}

	return err;
}

int CFred_mission_save::save_plot_info()
{
	if (!Format_fs2_open)
	{
		if (optional_string_fred("#Plot Info"))
		{
			parse_comments(2);

			// XSTR
			required_string_fred("$Tour:");
			parse_comments(2);
			fout(" ");
			fout_ext("Blah");

			required_string_fred("$Pre-Briefing Cutscene:");
			parse_comments();
			fout(" Blah");

			required_string_fred("$Pre-Mission Cutscene:");
			parse_comments();
			fout(" Blah");

			required_string_fred("$Next Mission Success:");
			parse_comments();
			fout(" Blah");

			required_string_fred("$Next Mission Partial:");
			parse_comments();
			fout(" Blah");

			required_string_fred("$Next Mission Failure:");
			parse_comments();
			fout(" Blah");
		}
		else
		{
			fout("\n\n#Plot Info\n\n");

			fout("$Tour: "); fout_ext("Blah"); fout("\n");
			fout("$Pre-Briefing Cutscene: Blah\n");
			fout("$Pre-Mission Cutscene: Blah\n");
			fout("$Next Mission Success: Blah\n");
			fout("$Next Mission Partial: Blah\n");
			fout("$Next Mission Failure: Blah\n");

			fout("\n");
		}
	}

	return err;
}

int CFred_mission_save::save_fiction()
{
	if (mission_has_fiction())
	{
		if (Format_fs2_open)
		{
			if (optional_string_fred("#Fiction Viewer"))
			{
				required_string_fred("$File:");
				parse_comments();
				fout(" %s", fiction_file());
			}
			else
			{
				fout("\n\n");
				fout("#Fiction Viewer\n\n");
				fout("$File: %s\n\n", fiction_file());
			}
		}
		else
		{
			MessageBox(NULL, "Warning: This mission contains fiction viewer data, but you are saving in the retail mission format.", "Incompatibility with retail mission format", MB_OK);
		}
	}

	return err;
}

int CFred_mission_save::save_cmd_brief()
{
	int stage;

	stage = 0;
	required_string_fred("#Command Briefing");
	parse_comments(2);

	if (The_mission.game_type & MISSION_TYPE_MULTI)
		return err;  // no command briefings allowed in multiplayer missions.

	for (stage=0; stage<Cur_cmd_brief->num_stages; stage++) {
		required_string_fred("$Stage Text:");
		parse_comments(2);

		// XSTR
		fout("\n");
		fout_ext("%s", Cur_cmd_brief->stage[stage].text);

		required_string_fred("$end_multi_text", "$Stage Text:");
		parse_comments();

		required_string_fred("$Ani Filename:");
		parse_comments();
		fout(" %s", Cur_cmd_brief->stage[stage].ani_filename);

		required_string_fred("+Wave Filename:", "$Ani Filename:");
		parse_comments();
		fout(" %s", Cur_cmd_brief->stage[stage].wave_filename);
	}

	return err;
}

int CFred_mission_save::save_cmd_briefs()
{
	int i;

	for (i=0; i<Num_teams; i++) {
		Cur_cmd_brief = &Cmd_briefs[i];
		save_cmd_brief();
	}

	return err;
}

int CFred_mission_save::save_briefing()
{
	int			i,j,k, nb;
	char			out[4096];
	brief_stage	*bs;
	brief_icon	*bi;

	for ( nb = 0; nb < Num_teams; nb++ ) {

		required_string_fred("#Briefing");
		parse_comments(2);

		required_string_fred("$start_briefing");
		parse_comments();

		Assert(Briefings[nb].num_stages <= MAX_BRIEF_STAGES);
		required_string_fred("$num_stages:");
		parse_comments();
		fout(" %d", Briefings[nb].num_stages);

		for (i=0; i<Briefings[nb].num_stages; i++) {
			bs = &Briefings[nb].stages[i];

			required_string_fred("$start_stage");
			parse_comments();

			required_string_fred("$multi_text");
			parse_comments();

			// XSTR
			fout("\n");
			sprintf(out,"%s", bs->new_text);
			fout_ext(out);

			required_string_fred("$end_multi_text", "$start_stage");
			parse_comments();

			if (!drop_white_space(bs->voice)[0]){
				strcpy(bs->voice, "None");
			}

			required_string_fred("$voice:");
			parse_comments();
			fout(" %s", bs->voice);

			required_string_fred("$camera_pos:");
			parse_comments();
			save_vector(bs->camera_pos);
			
			required_string_fred("$camera_orient:");
			parse_comments();
			save_matrix(bs->camera_orient);

			required_string_fred("$camera_time:");
			parse_comments();
			fout(" %d", bs->camera_time);

			required_string_fred("$num_lines:");
			parse_comments();
			fout(" %d", bs->num_lines);

			for (k=0; k<bs->num_lines; k++) {
				required_string_fred("$line_start:");
				parse_comments();
				fout(" %d", bs->lines[k].start_icon);

				required_string_fred("$line_end:");
				parse_comments();
				fout(" %d", bs->lines[k].end_icon);
			}

			required_string_fred("$num_icons:");
			parse_comments();
			Assert(bs->num_icons <= MAX_STAGE_ICONS );
			fout(" %d", bs->num_icons);

			required_string_fred("$Flags:");
			parse_comments();
			fout(" %d", bs->flags);

			required_string_fred("$Formula:");
			parse_comments();
			convert_sexp_to_string(bs->formula, out, SEXP_SAVE_MODE);
			fout(" %s", out);

			for ( j = 0; j < bs->num_icons; j++ ) {
				bi = &bs->icons[j];

				required_string_fred("$start_icon");
				parse_comments();

				required_string_fred("$type:");
				parse_comments();
				fout(" %d", bi->type);

				required_string_fred("$team:");
				parse_comments();
				fout(" %s", Iff_info[bi->team].iff_name);

				required_string_fred("$class:");
				parse_comments();
				if (bi->ship_class < 0)
					bi->ship_class = 0;

				fout(" %s", Ship_info[bi->ship_class].name);

				required_string_fred("$pos:");
				parse_comments();
				save_vector(bi->pos);

				if (drop_white_space(bi->label)[0]) {
					if (optional_string_fred("$label:"))
						parse_comments();
					else
						fout("\n$label:");

					fout(" %s", bi->label);
				}

				if (optional_string_fred("+id:"))
					parse_comments(); 
				else
					fout("\n+id:");

				fout(" %d", bi->id);

				required_string_fred("$hlight:");
				parse_comments();
				fout(" %d", (bi->flags & BI_HIGHLIGHT)?1:0 );

				if (Format_fs2_open)
				{
					required_string_fred("$mirror:");
					parse_comments();
					fout(" %d", (bi->flags & BI_MIRROR_ICON)?1:0 );
				}

				required_string_fred("$multi_text");
				parse_comments();

//				sprintf(out,"\n%s", bi->text);
//				fout(out);

				required_string_fred("$end_multi_text");
				parse_comments();

				required_string_fred("$end_icon");
				parse_comments();
			}

			required_string_fred("$end_stage");
			parse_comments();
		}
		required_string_fred("$end_briefing");
		parse_comments();
	}

	return err;
}

int CFred_mission_save::save_debriefing()
{
	int j, i;
	char out[8192];

	for ( j = 0; j < Num_teams; j++ ) {

		Debriefing = &Debriefings[j];

		required_string_fred("#Debriefing_info");
		parse_comments(2);

		required_string_fred("$Num stages:");
		parse_comments(2);
		fout(" %d", Debriefing->num_stages);

		for (i=0; i<Debriefing->num_stages; i++) {
			required_string_fred("$Formula:");
			parse_comments(2);
			convert_sexp_to_string(Debriefing->stages[i].formula, out, SEXP_SAVE_MODE);
			fout(" %s", out);

			// XSTR
			required_string_fred("$Multi text");
			parse_comments();
			fout("\n   ");
			fout_ext("%s", Debriefing->stages[i].new_text);

			required_string_fred("$end_multi_text");
			parse_comments();

			if (!drop_white_space(Debriefing->stages[i].voice)[0]){
				strcpy(Debriefing->stages[i].voice, "None");
			}

			required_string_fred("$Voice:");
			parse_comments();
			fout(" %s", Debriefing->stages[i].voice);

			// XSTR
			required_string_fred("$Recommendation text:");
			parse_comments();
			fout("\n   ");
			fout_ext("%s", Debriefing->stages[i].new_recommendation_text);

			required_string_fred("$end_multi_text");
			parse_comments();
		}
	}

	return err;
}

int sexp_variable_block_count();
// save variables
int CFred_mission_save::save_variables()
{
	char *type;
	char number[] = "number";
	char string[] = "string";
	char block[] = "block";
	int i;

	// sort sexp_variables
	sexp_variable_sort();

	// get count
	int num_variables = sexp_variable_count();
	int num_block_vars = sexp_variable_block_count();
	int total_variables = num_variables + num_block_vars;

	if (total_variables > 0) {

		// write 'em out
		required_string_fred("#Sexp_variables");
		parse_comments(2);

		required_string_fred("$Variables:");
		parse_comments(2);

		fout ("\n(");
//		parse_comments();

		for (i=0; i<num_variables; i++) {
			if (Sexp_variables[i].type & SEXP_VARIABLE_NUMBER) {
				type = number;
			} else {
				type = string;
			}
			// index "var name" "default" "type"
			fout("\n\t\t%d\t\t\"%s\"\t\t\"%s\"\t\t\"%s\"", i, Sexp_variables[i].variable_name, Sexp_variables[i].text, type);

			// persistent and network variables
			if (Format_fs2_open)
			{
				// Network variable - Karajorma
				if (Sexp_variables[i].type & SEXP_VARIABLE_NETWORK) {
					fout("\t\t\"%s\"", "network-variable");
				}

				// player-persistent - Goober5000
				if (Sexp_variables[i].type & SEXP_VARIABLE_PLAYER_PERSISTENT) {
					fout("\t\t\"%s\"", "player-persistent");
				// campaign-persistent - Goober5000
				} else if (Sexp_variables[i].type & SEXP_VARIABLE_CAMPAIGN_PERSISTENT) {
					fout("\t\t\"%s\"", "campaign-persistent");
				}
			}

//			parse_comments();
		}

		for (i=MAX_SEXP_VARIABLES-num_block_vars; i<MAX_SEXP_VARIABLES; i++) {
			type = block;
			fout("\n\t\t%d\t\t\"%s\"\t\t\"%s\"\t\t\"%s\"", i, Sexp_variables[i].variable_name, Sexp_variables[i].text, type);
		}

		fout("\n)");
	}

	return err;
}


int CFred_mission_save::save_players()
{
	int i, j;
	int used_pool[MAX_WEAPON_TYPES];

	// write out alternate name list
	if(Mission_alt_type_count > 0){
		fout("\n\n#Alternate Types:\n");

		// write them all out
		for(i=0; i<Mission_alt_type_count; i++){
			fout("$Alt: %s\n", Mission_alt_types[i]);
		}

		// end
		fout("\n#end\n");
	}

	required_string_fred("#Players");
	parse_comments(2);
	fout("\t\t;! %d total\n", Player_starts);

	for (i=0; i<Num_teams; i++) {
		required_string_fred("$Starting Shipname:");
		parse_comments();
		Assert(Player_start_shipnum >= 0);
		fout(" %s", Ships[Player_start_shipnum].ship_name);
		
		required_string_fred("$Ship Choices:");
		parse_comments();
		fout(" (\n");

		for (j=0; j<Team_data[i].number_choices; j++)
		{
			//Karajorma - Check to see if a variable name should be written for the class rather than a number
			if (!strcmp(Team_data[i].ship_list_variables[j], ""))
			{
				fout("\t\"%s\"\t", Ship_info[Team_data[i].ship_list[j]].name); 
			}
			else 
			{
				fout("\t@%s\t", Team_data[i].ship_list_variables[j]);
			}

			// Now check if we should write a variable or a number for the amount of ships available
			if (!strcmp(Team_data[i].ship_count_variables[j], ""))
			{
				fout("%d\n", Team_data[i].ship_count[j]);
			}
			else 
			{
				fout("@%s\n", Team_data[i].ship_count_variables[j]);
			}
		}
		fout(")");

		if (optional_string_fred("+Weaponry Pool:", "$Starting Shipname:")){
			parse_comments(2);
		} else {
			fout("\n\n+Weaponry Pool:");
		}

		fout(" (\n");
		generate_weaponry_usage_list(used_pool);
		for (j=0; j<Num_weapon_types; j++){
			if (Team_data[i].weaponry_pool[j] + used_pool[j] > 0){
				fout("\t\"%s\"\t%d\n", Weapon_info[j].name, Team_data[i].weaponry_pool[j] + used_pool[j]);
			}
		}

		fout(")");
	}

	return err;
}

// Goober5000
void CFred_mission_save::save_single_dock_instance(ship *shipp, dock_instance *dock_ptr)
{
	Assert(shipp && dock_ptr);
	Assert(dock_ptr->docked_objp->type == OBJ_SHIP || dock_ptr->docked_objp->type == OBJ_START);

	// get ships and objects
	object *objp = &Objects[shipp->objnum];
	object *other_objp = dock_ptr->docked_objp;
	ship *other_shipp = &Ships[other_objp->instance];

	// write other ship
	if (optional_string_fred("+Docked With:", "$Name:"))
		parse_comments();
	else
		fout("\n+Docked With:");
	fout(" %s", other_shipp->ship_name);


	// Goober5000 - hm, Volition seems to have reversed docker and dockee here

	// write docker (actually dockee) point
	required_string_fred("$Docker Point:", "$Name:");
	parse_comments();
	fout(" %s", model_get_dock_name(Ship_info[other_shipp->ship_info_index].model_num, dock_find_dockpoint_used_by_object(other_objp, objp)));

	// write dockee (actually docker) point
	required_string_fred("$Dockee Point:", "$Name:");
	parse_comments();
	fout(" %s", model_get_dock_name(Ship_info[shipp->ship_info_index].model_num, dock_find_dockpoint_used_by_object(objp, other_objp)));
}

int CFred_mission_save::save_objects()
{
	char out[4096];
	int i, j, k, z, wrote_heading;
	ai_info *aip;
	object *objp;
	ship_info *sip;
	
	required_string_fred("#Objects");
	parse_comments(2);
	fout("\t\t;! %d total\n", ship_get_num_ships() );

	for (i=z=0; i<MAX_SHIPS; i++) {
		if (Ships[i].objnum < 0){
			continue;
		}

		j = Objects[Ships[i].objnum].type;
		if ((j != OBJ_SHIP) && (j != OBJ_START)){
			continue;
		}

		objp = &Objects[Ships[i].objnum];
		sip = &Ship_info[Ships[i].ship_info_index];
		required_string_either_fred("$Name:", "#Wings");
		required_string_fred("$Name:");
		parse_comments(z ? 2 : 1);
		fout(" %s\t\t;! Object #%d\n", Ships[i].ship_name, i);

		required_string_fred("$Class:");
		parse_comments(0);
		fout(" %s", Ship_info[Ships[i].ship_info_index].name);

		// optional alternate type name
		if(strlen(Fred_alt_names[i])){
			fout("\n$Alt: %s\n", Fred_alt_names[i]);
		}
		
		// optional alternate ship classes
		if (Format_fs2_open)
		{
			// Alternate class type 1
			for (int m=0; m < MAX_ALT_CLASS_1; m++)
			{
				if ((Ships[i].alt_class_one[m] > -1) || (Ships[i].alt_class_one_variable[m] > -1))
				{
					if (optional_string_fred("+Alt_Ship_Class_Type_1: ", "$Name:"))
					{
						parse_comments();
					}
					else 
					{
						fout("\n+Alt_Ship_Class_Type_1: ");
					}

					if (Ships[i].alt_class_one[m] > -1)
					{
						fout ("\"%s\"", Ship_info[Ships[i].alt_class_one[m]].name);
					}
					else 
					{
						fout ("@%s", Sexp_variables[Ships[i].alt_class_one_variable[m]].variable_name);
					}
				}
			}

			// Alternate class type 2
			for (int n=0; n < MAX_ALT_CLASS_2; n++)
			{
				if ((Ships[i].alt_class_two[n] > -1) || (Ships[i].alt_class_two_variable[n] > -1))
				{
					if (optional_string_fred("+Alt_Ship_Class_Type_2: ", "$Name:"))
					{
						parse_comments();
					}
					else 
					{
						fout("\n+Alt_Ship_Class_Type_2: ");
					}

					if (Ships[i].alt_class_two[n] > -1)
					{
						fout ("\"%s\"", Ship_info[Ships[i].alt_class_two[n]].name);
					}
					else 
					{
						fout ("@%s", Sexp_variables[Ships[i].alt_class_two_variable[n]].variable_name);
					}
				}
			}
		}

		required_string_fred("$Team:");
		parse_comments();
		fout(" %s", Iff_info[Ships[i].team].iff_name);

		required_string_fred("$Location:");
		parse_comments();
		save_vector(Objects[Ships[i].objnum].pos);

		required_string_fred("$Orientation:");
		parse_comments();
		save_matrix(Objects[Ships[i].objnum].orient);

		if (Format_fs2_retail)
		{
			required_string_fred("$IFF:");
			parse_comments();
			fout(" %s", "IFF 1");
		}

		Assert(Ships[i].ai_index >= 0);
		aip = &Ai_info[Ships[i].ai_index];

		required_string_fred("$AI Behavior:");
		parse_comments();
		fout(" %s", Ai_behavior_names[aip->behavior]);

		if (Ships[i].weapons.ai_class != Ship_info[Ships[i].ship_info_index].ai_class) {
			if (optional_string_fred("+AI Class:", "$Name:"))
				parse_comments();
			else
				fout("\n+AI Class:");

			fout(" %s", Ai_class_names[Ships[i].weapons.ai_class]);
		}

		save_ai_goals(Ai_info[Ships[i].ai_index].goals, i);

		// XSTR
		required_string_fred("$Cargo 1:");
		parse_comments();
		fout(" ");
		fout_ext("%s", Cargo_names[Ships[i].cargo1]);

		save_common_object_data(&Objects[Ships[i].objnum], &Ships[i]);

		if (Ships[i].wingnum >= 0){
			Ships[i].arrival_location = ARRIVE_AT_LOCATION;
		}

		required_string_fred("$Arrival Location:");
		parse_comments();
		fout(" %s", Arrival_location_names[Ships[i].arrival_location]);

		if (Ships[i].arrival_location != ARRIVE_AT_LOCATION)
		{
			if (optional_string_fred("+Arrival Distance:", "$Name:")){
				parse_comments();
			} else {
				fout("\n+Arrival Distance:");
			}

			fout(" %d", Ships[i].arrival_distance);
			if (optional_string_fred("$Arrival Anchor:", "$Name:")){
				parse_comments();
			} else {
				fout("\n$Arrival Anchor:");
			}

			z = Ships[i].arrival_anchor;
			if (z & SPECIAL_ARRIVAL_ANCHOR_FLAG)
			{
				// get name
				char tmp[NAME_LENGTH + 15];
				stuff_special_arrival_anchor_name(tmp, z, Format_fs2_retail);

				// save it
				fout(" %s", tmp);
			}
			else if (z >= 0)
			{
				fout(" %s", Ships[z].ship_name);
			}
			else
			{
				fout(" <error>");
			}
		}

		// Goober5000
		if (Format_fs2_open)
		{
			if ((Ships[i].arrival_location == ARRIVE_FROM_DOCK_BAY) && (Ships[i].arrival_path_mask > 0))
			{
				int j, anchor_shipnum;
				polymodel *pm;

				anchor_shipnum = Ships[i].arrival_anchor;
				Assert(anchor_shipnum >= 0 && anchor_shipnum < MAX_SHIPS);

				fout("\n+Arrival Paths: ( ");

				pm = model_get(Ship_info[Ships[anchor_shipnum].ship_info_index].model_num);
				for (j = 0; j < pm->ship_bay->num_paths; j++)
				{
					if (Ships[i].arrival_path_mask & (1 << j))
					{
						fout("\"%s\" ", pm->paths[pm->ship_bay->path_indexes[j]].name);
					}
				}

				fout(")");
			}
		}

		if (Ships[i].arrival_delay)
		{
			if (optional_string_fred("+Arrival Delay:", "$Name:"))
				parse_comments();
			else
				fout("\n+Arrival Delay:");

			fout(" %d", Ships[i].arrival_delay);
		}

		required_string_fred("$Arrival Cue:");
		parse_comments();
		convert_sexp_to_string(Ships[i].arrival_cue, out, SEXP_SAVE_MODE);
		fout(" %s", out);

		required_string_fred("$Departure Location:");
		parse_comments();
		fout(" %s", Departure_location_names[Ships[i].departure_location]);

		if ( Ships[i].departure_location != DEPART_AT_LOCATION )
		{
			required_string_fred("$Departure Anchor:");
			parse_comments();
			
			if ( Ships[i].departure_anchor >= 0 )
				fout(" %s", Ships[Ships[i].departure_anchor].ship_name );
			else
				fout(" <error>");
		}

		// Goober5000
		if (Format_fs2_open)
		{
			if ((Ships[i].departure_location == DEPART_AT_DOCK_BAY) && (Ships[i].departure_path_mask > 0))
			{
				int j, anchor_shipnum;
				polymodel *pm;

				anchor_shipnum = Ships[i].departure_anchor;
				Assert(anchor_shipnum >= 0 && anchor_shipnum < MAX_SHIPS);

				fout("\n+Departure Paths: ( ");

				pm = model_get(Ship_info[Ships[anchor_shipnum].ship_info_index].model_num);
				for (j = 0; j < pm->ship_bay->num_paths; j++)
				{
					if (Ships[i].departure_path_mask & (1 << j))
					{
						fout("\"%s\" ", pm->paths[pm->ship_bay->path_indexes[j]].name);
					}
				}

				fout(")");
			}
		}

		if (Ships[i].departure_delay)
		{
			if (optional_string_fred("+Departure delay:", "$Name:"))
				parse_comments();
			else
				fout("\n+Departure delay:");

			fout(" %d", Ships[i].departure_delay);
		}

		required_string_fred("$Departure Cue:");
		parse_comments();
		convert_sexp_to_string(Ships[i].departure_cue, out, SEXP_SAVE_MODE);
		fout(" %s", out);

		required_string_fred("$Determination:");
		parse_comments();
		fout(" %d", Ships[i].determination);

		if (optional_string_fred("+Flags:", "$Name:")) {
			parse_comments();
			fout (" (");
		} else
			fout("\n+Flags: (");

		if (Ships[i].flags & SF_CARGO_REVEALED)
			fout(" \"cargo-known\"");
		if (Ships[i].flags & SF_IGNORE_COUNT)
			fout(" \"ignore-count\"");
		if (objp->flags & OF_PROTECTED)
			fout(" \"protect-ship\"");
		if (Ships[i].flags & SF_REINFORCEMENT)
			fout(" \"reinforcement\"");
		if (objp->flags & OF_NO_SHIELDS)
			fout(" \"no-shields\"");
		if (Ships[i].flags & SF_ESCORT)
			fout(" \"escort\"");
		if (objp->type == OBJ_START)
			fout(" \"player-start\"");
		if (Ships[i].flags & SF_NO_ARRIVAL_MUSIC)
			fout(" \"no-arrival-music\"");
		if (Ships[i].flags & SF_NO_ARRIVAL_WARP)
			fout(" \"no-arrival-warp\"");
		if (Ships[i].flags & SF_NO_DEPARTURE_WARP)
			fout(" \"no-departure-warp\"");
		if (Ships[i].flags & SF_LOCKED)
			fout(" \"locked\"");
		if (Objects[Ships[i].objnum].flags & OF_INVULNERABLE)
			fout(" \"invulnerable\"");
		if (Ships[i].flags & SF_HIDDEN_FROM_SENSORS)
			fout(" \"hidden-from-sensors\"");
		if (Ships[i].flags & SF_SCANNABLE)
			fout(" \"scannable\"");
		if (Ai_info[Ships[i].ai_index].ai_flags & AIF_KAMIKAZE)
			fout(" \"kamikaze\"");
		if (Ai_info[Ships[i].ai_index].ai_flags & AIF_NO_DYNAMIC)
			fout(" \"no-dynamic\"");
		if (Ships[i].flags & SF_RED_ALERT_STORE_STATUS)
			fout(" \"red-alert-carry\"");
		if (objp->flags & OF_BEAM_PROTECTED)
			fout(" \"beam-protect-ship\"");
		if (Ships[i].ship_guardian_threshold != 0)
			fout(" \"guardian\"");
		if (objp->flags & OF_SPECIAL_WARP)
			fout(" \"special-warp\"");
		if (Ships[i].flags & SF_VAPORIZE)
			fout(" \"vaporize\"");
		if (Ships[i].flags2 & SF2_STEALTH)
			fout(" \"stealth\"");
		if (Ships[i].flags2 & SF2_FRIENDLY_STEALTH_INVIS)
			fout(" \"friendly-stealth-invisible\"");
		if (Ships[i].flags2 & SF2_DONT_COLLIDE_INVIS)
			fout(" \"don't-collide-invisible\"");
		fout(" )");

		// flags2 added by Goober5000 --------------------------------
		if (Format_fs2_open)
		{
			if (optional_string_fred("+Flags2:", "$Name:")) {
				parse_comments();
				fout (" (");
			} else
				fout("\n+Flags2: (");

			if (Ships[i].flags2 & SF2_PRIMITIVE_SENSORS)
				fout(" \"primitive-sensors\"");
			if (Ships[i].flags2 & SF2_NO_SUBSPACE_DRIVE)
				fout(" \"no-subspace-drive\"");
			if (Ships[i].flags2 & SF2_NAVPOINT_CARRY)
				fout(" \"nav-carry-status\"");
			if (Ships[i].flags2 & SF2_AFFECTED_BY_GRAVITY)
				fout(" \"affected-by-gravity\"");
			if (Ships[i].flags2 & SF2_TOGGLE_SUBSYSTEM_SCANNING)
				fout(" \"toggle-subsystem-scanning\"");
			if (Objects[i].flags & OF_TARGETABLE_AS_BOMB)
				fout(" \"targetable-as-bomb\"");
			if (Ships[i].flags2 & SF2_NO_BUILTIN_MESSAGES)
				fout(" \"no-builtin-messages\"");
			if (Ships[i].flags2 & SF2_PRIMARIES_LOCKED)
				fout(" \"primaries-locked\"");
			if (Ships[i].flags2 & SF2_SECONDARIES_LOCKED)
				fout(" \"secondaries-locked\"");
			if (Ships[i].flags2 & SF2_NO_DEATH_SCREAM)
				fout(" \"no-death-scream\"");
			if (Ships[i].flags2 & SF2_ALWAYS_DEATH_SCREAM)
				fout(" \"always-death-scream\"");
			if (Ships[i].flags2 & SF2_NAVPOINT_NEEDSLINK)
				fout(" \"nav-needslink\"");
			if (Ships[i].flags2 & SF2_USE_ALT_NAME_AS_CALLSIGN)
				fout(" \"use-alt-name-as-callsign\"");
			if (Ships[i].flags2 & SF2_SET_CLASS_DYNAMICALLY)
				fout(" \"set-class-dynamically\"");
			if (Ships[i].flags2 & SF2_TEAM_LOADOUT_STORE_STATUS)
				fout(" \"team-loadout-store\"");
			fout(" )");
		}
		// -----------------------------------------------------------

		fout("\n+Respawn priority: %d", Ships[i].respawn_priority);	// HA!  Newline added by Goober5000

		if (Ships[i].flags & SF_ESCORT) {
			if (optional_string_fred("+Escort priority:", "$Name:")) {
				parse_comments();
			} else {
				fout("\n+Escort priority:");
			}

			fout(" %d", Ships[i].escort_priority);
		}

		if (Ships[i].special_exp_index != -1) {
			if (optional_string_fred("+Special Exp index:", "$Name:")) {
				parse_comments();
			} else {
				fout("\n+Special Exp index:");
			}

			fout(" %d", Ships[i].special_exp_index);
		}

		// Goober5000 ------------------------------------------------
		if (Format_fs2_open)
		{
			if (Ships[i].special_hitpoint_index != -1) {
				if (optional_string_fred("+Special Hitpoint index:", "$Name:")) {
					parse_comments();
				} else {
					fout("\n+Special Hitpoint index:");
				}

				fout(" %d", Ships[i].special_hitpoint_index);
			}
		}
		// -----------------------------------------------------------

		if ( Ai_info[Ships[i].ai_index].ai_flags & AIF_KAMIKAZE ) {
			if ( optional_string_fred("+Kamikaze Damage:", "$Name:")){
				parse_comments();
			} else {
				fout("\n+Kamikaze Damage:");
			}

			fout(" %d", (int)(Ai_info[Ships[i].ai_index].kamikaze_damage) );
		}

		if (Ships[i].hotkey != -1) {
			if (optional_string_fred("+Hotkey:", "$Name:")){
				parse_comments();
			} else {
				fout("\n+Hotkey:");
			}

			fout(" %d", Ships[i].hotkey);
		}

		// mwa -- new code to save off information about initially docked ships.
		// Goober5000 - newer code to save off information about initially docked ships. ;)
		if (object_is_docked(&Objects[Ships[i].objnum]))
		{
			// possible incompatibility
			if (!Format_fs2_open && !dock_check_docked_one_on_one(&Objects[Ships[i].objnum]))
			{
				static bool warned = false;
				if (!warned)
				{
					CString text = "You are saving in the retail mission format, but \"";
					text += Ships[i].ship_name;
					text += "\" is docked to more than one ship.  If you wish to run this mission in retail, ";
					text += "you should remove the additional ships and save the mission again.";
					MessageBox(NULL, text, "Incompatibility with retail mission format", MB_OK);

					warned = true;	// to avoid zillions of boxes
				}
			}

			// save one-on-one groups as if they were retail
			if (dock_check_docked_one_on_one(&Objects[Ships[i].objnum]))
			{
				// retail format only saved information for non-leaders
				if (!(Ships[i].flags & SF_DOCK_LEADER))
				{
					save_single_dock_instance(&Ships[i], Objects[Ships[i].objnum].dock_list);
				}
			}
			// multiply docked
			else
			{
				// save all instances for all ships
				for (dock_instance *dock_ptr = Objects[Ships[i].objnum].dock_list; dock_ptr != NULL; dock_ptr = dock_ptr->next)
				{
					save_single_dock_instance(&Ships[i], dock_ptr);
				}
			}
		}

		// check the ship flag about killing off the ship before a missino starts.  Write out the appropriate
		// variable if necessary
		if ( Ships[i].flags & SF_KILL_BEFORE_MISSION ) {
			if ( optional_string_fred("+Destroy At:", "$Name:"))
				parse_comments();
			else
				fout ("\n+Destroy At: ");

			fout(" %d", Ships[i].final_death_time );
		}

		// possibly write out the orders that this ship will accept.  We'll only do it if the orders
		// are not the default set of orders
		if ( Ships[i].orders_accepted != ship_get_default_orders_accepted( &Ship_info[Ships[i].ship_info_index]) ) {
			if ( optional_string_fred("+Orders Accepted:", "$Name:") )
				parse_comments();
			else
				fout("\n+Orders Accepted:");

			fout(" %d\t\t;! note that this is a bitfield!!!", Ships[i].orders_accepted);
		}

		if (Ships[i].group >= 0) {
			if (optional_string_fred("+Group:", "$Name:"))
				parse_comments();
			else
				fout("\n+Group:");

			fout(" %d", Ships[i].group);
		}

		// Only bother with the ships score if it is not the default value with FSO missions
		// Always write it out for retail missions though
		if (!Format_fs2_open || Ship_info[Ships[i].ship_info_index].score != Ships[i].score ) {
			if (optional_string_fred("+Score:", "$Name:"))
				parse_comments();
			else
				fout("\n+Score:");

			fout(" %d", Ships[i].score);
		}

		// deal with the persona for this ship as well.
		if ( Ships[i].persona_index != -1 ) {
			if (optional_string_fred("+Persona Index:", "$Name:"))
				parse_comments();
			else
				fout("\n+Persona Index:");

			fout(" %d", Ships[i].persona_index);
		}

		// Goober5000 - deal with texture replacement ----------------
		k = 0;
		wrote_heading = 0;
		while (k < Fred_num_texture_replacements)
		{
			if (!stricmp(Ships[i].ship_name, Fred_texture_replacements[k].ship_name))
			{
				// see about writing the title
				if (!wrote_heading)
				{
					fout_and_bypass("\n;;FSO 3.6.8;; $Texture Replace:");
					wrote_heading = 1;
				}

				// write out this entry
				fout_and_bypass("\n;;FSO 3.6.8;; +old: %s", Fred_texture_replacements[k].old_texture);
				fout_and_bypass("\n;;FSO 3.6.8;; +new: %s", Fred_texture_replacements[k].new_texture);
			}

			k++;	// increment down the list of texture replacements
		}
		// end of texture replacement -------------------------------

		z++;
	}

	return err;
}

int CFred_mission_save::save_common_object_data(object *objp, ship *shipp)
{
	int j, z;
	ship_subsys *ptr = NULL;
	ship_info *sip   = NULL;
	ship_weapon *wp  = NULL;
	float temp_max_hull_strength;

	sip = &Ship_info[shipp->ship_info_index];

	if ((int) objp->phys_info.speed) {
		if (optional_string_fred("+Initial Velocity:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Initial Velocity:");

		fout(" %d", (int) objp->phys_info.speed);
	}

	// Goober5000
	if (Format_fs2_open && (shipp->special_hitpoint_index != -1))
	{
		temp_max_hull_strength = (float) atoi(Sexp_variables[shipp->special_hitpoint_index+HULL_STRENGTH].text);
	}
	else
	{
		temp_max_hull_strength = sip->max_hull_strength;
	}

	if ((int) objp->hull_strength != temp_max_hull_strength) {
		if (optional_string_fred("+Initial Hull:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Initial Hull:");

		fout(" %d", (int) objp->hull_strength);
	}

	int shield_strength = (int) shield_get_strength(objp);
	if (shield_strength != 100) {
		if (optional_string_fred("+Initial Shields:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Initial Shields:");

		fout(" %d", shield_strength);
	}

	// save normal ship weapons info
	required_string_fred("+Subsystem:", "$Name:");
	parse_comments();
	fout(" Pilot");

	wp = &shipp->weapons;
	z = 0;
	j = wp->num_primary_banks;
	while (j-- && (j >= 0))
		if (wp->primary_bank_weapons[j] != sip->primary_bank_weapons[j])
			z = 1;

	if (z) {
		if (optional_string_fred("+Primary Banks:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Primary Banks:");

		fout(" ( ");
		for (j=0; j<wp->num_primary_banks; j++)
			fout("\"%s\" ", Weapon_info[wp->primary_bank_weapons[j]].name);

		fout(")");
	}

	z = 0;
	j = wp->num_secondary_banks;
	while (j-- && (j >= 0))
		if (wp->secondary_bank_weapons[j] != sip->secondary_bank_weapons[j])
			z = 1;

	if (z) {
		if (optional_string_fred("+Secondary Banks:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Secondary Banks:");

		fout(" ( ");
		for (j=0; j<wp->num_secondary_banks; j++)
			fout("\"%s\" ", Weapon_info[wp->secondary_bank_weapons[j]].name);

		fout(")");
	}

	z = 0;
	j = wp->num_secondary_banks;
	while (j-- && (j >= 0))
		if (wp->secondary_bank_ammo[j] != 100)
			z = 1;

	if (z) {
		if (optional_string_fred("+Sbank Ammo:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Sbank Ammo:");

		fout(" ( ");
		for (j=0; j<wp->num_secondary_banks; j++)
			fout("%d ", wp->secondary_bank_ammo[j]);

		fout(")");
	}

	ptr = GET_FIRST(&shipp->subsys_list);
	Assert(ptr);

	while (ptr != END_OF_LIST(&shipp->subsys_list) && ptr) {
		// Crashing here!
		if ( (ptr->current_hits) || (ptr->system_info && ptr->system_info->type == SUBSYSTEM_TURRET) || (ptr->subsys_cargo_name != -1)) {
			if (optional_string_fred("+Subsystem:", "$Name:"))
				parse_comments();
			else
				fout("\n+Subsystem:");

			fout(" %s", ptr->system_info->subobj_name);
		}

		if (ptr->current_hits) {
			if (optional_string_fred("$Damage:", "$Name:", "+Subsystem:"))
				parse_comments();
			else
				fout("\n$Damage:");

			fout(" %d", (int) ptr->current_hits);
		}

		if (ptr->subsys_cargo_name != -1) {
			if (optional_string_fred("+Cargo Name:", "$Name:", "+Subsystem:"))
				parse_comments();
			else
				fout("\n+Cargo Name:");

			fout_ext("%s", Cargo_names[ptr->subsys_cargo_name]);
		}

		if (ptr->system_info->type == SUBSYSTEM_TURRET)
			save_turret_info(ptr, shipp - Ships);

		ptr = GET_NEXT(ptr);
	}

/*	for (j=0; j<shipp->status_count; j++) {
		required_string_fred("$Status Description:");
		parse_comments(-1);
		fout(" %s", Status_desc_names[shipp->status_type[j]]);

		required_string_fred("$Status:");
		parse_comments(-1);
		fout(" %s", Status_type_names[shipp->status[j]]);

		required_string_fred("$Target:");
		parse_comments(-1);
		fout(" %s", Status_target_names[shipp->target[j]]);
	}*/

	return err;
}

int CFred_mission_save::save_wings()
{
	char out[4096];
	int i, j, z, ship, count = 0;

	fred_parse_flag = 0;
	required_string_fred("#Wings");
	parse_comments(2);
	fout("\t\t;! %d total", Num_wings);

	for (i=0; i<MAX_WINGS; i++) {
		if (!Wings[i].wave_count)
			continue;

		count++;
		required_string_either_fred("$Name:", "#Events");
		required_string_fred("$Name:");
		parse_comments(2);
		fout(" %s", Wings[i].name);

		// squad logo - Goober5000
		if (Format_fs2_open)
		{
			if (strlen(Wings[i].wing_squad_filename) > 0)
			{
				if (optional_string_fred("+Squad Logo:", "$Name:"))
					parse_comments();
				else
					fout("\n+Squad Logo:");

				fout(" %s", Wings[i].wing_squad_filename);
			}
		}

		required_string_fred("$Waves:");
		parse_comments();
		fout(" %d", Wings[i].num_waves);

		required_string_fred("$Wave Threshold:");
		parse_comments();
		fout(" %d", Wings[i].threshold);

		required_string_fred("$Special Ship:");
		parse_comments();
		fout(" %d\t\t;! %s\n", Wings[i].special_ship,
			Ships[Wings[i].ship_index[Wings[i].special_ship]].ship_name);

		required_string_fred("$Arrival Location:");
		parse_comments();
		fout(" %s", Arrival_location_names[Wings[i].arrival_location]);

		if (Wings[i].arrival_location != ARRIVE_AT_LOCATION)
		{
			if (optional_string_fred("+Arrival Distance:", "$Name:"))
				parse_comments();
			else
				fout("\n+Arrival Distance:");

			fout(" %d", Wings[i].arrival_distance);
			if (optional_string_fred("$Arrival Anchor:", "$Name:"))
				parse_comments();
			else
				fout("\n$Arrival Anchor:");

			z = Wings[i].arrival_anchor;
			if (z & SPECIAL_ARRIVAL_ANCHOR_FLAG)
			{
				// get name
				char tmp[NAME_LENGTH + 15];
				stuff_special_arrival_anchor_name(tmp, z, Format_fs2_retail);

				// save it
				fout(" %s", tmp);
			}
			else if (z >= 0)
			{
				fout(" %s", Ships[z].ship_name);
			}
			else
			{
				fout(" <error>");
			}
		}

		// Goober5000
		if (Format_fs2_open)
		{
			if ((Wings[i].arrival_location == ARRIVE_FROM_DOCK_BAY) && (Wings[i].arrival_path_mask > 0))
			{
				int j, anchor_shipnum;
				polymodel *pm;

				anchor_shipnum = Wings[i].arrival_anchor;
				Assert(anchor_shipnum >= 0 && anchor_shipnum < MAX_SHIPS);

				fout("\n+Arrival Paths: ( ");

				pm = model_get(Ship_info[Ships[anchor_shipnum].ship_info_index].model_num);
				for (j = 0; j < pm->ship_bay->num_paths; j++)
				{
					if (Wings[i].arrival_path_mask & (1 << j))
					{
						fout("\"%s\" ", pm->paths[pm->ship_bay->path_indexes[j]].name);
					}
				}

				fout(")");
			}
		}

		if (Wings[i].arrival_delay)
		{
			if (optional_string_fred("+Arrival delay:", "$Name:"))
				parse_comments();
			else
				fout("\n+Arrival delay:");

			fout(" %d", Wings[i].arrival_delay);
		}

		required_string_fred("$Arrival Cue:");
		parse_comments();
		convert_sexp_to_string(Wings[i].arrival_cue, out, SEXP_SAVE_MODE);
		fout(" %s", out);

		required_string_fred("$Departure Location:");
		parse_comments();
		fout(" %s", Departure_location_names[Wings[i].departure_location]);

		if ( Wings[i].departure_location != DEPART_AT_LOCATION )
		{
			required_string_fred("$Departure Anchor:");
			parse_comments();

			if ( Wings[i].departure_anchor >= 0 )
				fout(" %s", Ships[Wings[i].departure_anchor].ship_name );
			else
				fout(" <error>");
		}

		// Goober5000
		if (Format_fs2_open)
		{
			if ((Wings[i].departure_location == DEPART_AT_DOCK_BAY) && (Wings[i].departure_path_mask > 0))
			{
				int j, anchor_shipnum;
				polymodel *pm;

				anchor_shipnum = Wings[i].departure_anchor;
				Assert(anchor_shipnum >= 0 && anchor_shipnum < MAX_SHIPS);

				fout("\n+Departure Paths: ( ");

				pm = model_get(Ship_info[Ships[anchor_shipnum].ship_info_index].model_num);
				for (j = 0; j < pm->ship_bay->num_paths; j++)
				{
					if (Wings[i].departure_path_mask & (1 << j))
					{
						fout("\"%s\" ", pm->paths[pm->ship_bay->path_indexes[j]].name);
					}
				}

				fout(")");
			}
		}

		if (Wings[i].departure_delay)
		{
			if (optional_string_fred("+Departure delay:", "$Name:"))
				parse_comments();
			else
				fout("\n+Departure delay:");

			fout(" %d", Wings[i].departure_delay);
		}

		required_string_fred("$Departure Cue:");
		parse_comments();
		convert_sexp_to_string(Wings[i].departure_cue, out, SEXP_SAVE_MODE);
		fout(" %s", out);

		required_string_fred("$Ships:");
		parse_comments();
		fout(" (\t\t;! %d total\n", Wings[i].wave_count);

		for (j=0; j<Wings[i].wave_count; j++) {
			ship = Wings[i].ship_index[j];
//			if (Objects[Ships[ship].objnum].type == OBJ_START)
//				fout("\t\"Player 1\"\n");
//			else
				fout("\t\"%s\"\n", Ships[Wings[i].ship_index[j]].ship_name);
		}

		fout(")");
		save_ai_goals(Wings[i].ai_goals, -1);

		if (Wings[i].hotkey != -1) {
			if (optional_string_fred("+Hotkey:", "$Name:"))
				parse_comments();
			else
				fout("\n+Hotkey:");

			fout(" %d", Wings[i].hotkey);
		}

		if ( optional_string_fred("+Flags:", "$Name:")) {
			parse_comments();
			fout( "(" );
		} else 
			fout("\n+Flags: (");

		if ( Wings[i].flags & WF_IGNORE_COUNT )
			fout(" \"ignore-count\"");
		if ( Wings[i].flags & WF_REINFORCEMENT )
			fout(" \"reinforcement\"");
		if ( Wings[i].flags & WF_NO_ARRIVAL_MUSIC )
			fout(" \"no-arrival-music\"");
		if ( Wings[i].flags & WF_NO_ARRIVAL_MESSAGE )
			fout(" \"no-arrival-message\"");
		if ( Wings[i].flags & WF_NO_ARRIVAL_WARP )
			fout(" \"no-arrival-warp\"");
		if ( Wings[i].flags & WF_NO_DEPARTURE_WARP )
			fout(" \"no-departure-warp\"");
		if ( Wings[i].flags & WF_NO_DYNAMIC )
			fout( " \"no-dynamic\"" );

		fout(" )");

		if (Wings[i].wave_delay_min) {
			if (optional_string_fred("+Wave Delay Min:", "$Name:"))
				parse_comments();
			else
				fout("\n+Wave Delay Min:");

			fout(" %d", Wings[i].wave_delay_min);
		}

		if (Wings[i].wave_delay_max) {
			if (optional_string_fred("+Wave Delay Max:", "$Name:"))
				parse_comments();
			else
				fout("\n+Wave Delay Max:");

			fout(" %d", Wings[i].wave_delay_max);
		}
	}

	Assert(count == Num_wings);
	return err;
}

int CFred_mission_save::save_goals()
{
	char out[4096];
	int i;

	fred_parse_flag = 0;
	required_string_fred("#Goals");
	parse_comments(2);
	fout("\t\t;! %d total\n", Num_goals);

	for (i=0; i<Num_goals; i++) {
		int type;

		required_string_either_fred("$Type:", "#Waypoints");
		required_string_fred("$Type:");
		parse_comments(i ? 2 : 1);

		type = Mission_goals[i].type & GOAL_TYPE_MASK;
		fout(" %s", Goal_type_names[type]);

		if (*Mission_goals[i].name) {
			if (optional_string_fred("+Name:", "$Type:"))
				parse_comments();
			else
				fout("\n+Name:");

			fout(" %s", Mission_goals[i].name);
		}

		// XSTR
		required_string_fred("$MessageNew:");
		parse_comments();
		fout(" ");
		fout_ext("%s", Mission_goals[i].message);
		fout("\n");
		required_string_fred("$end_multi_text");
		parse_comments(0);

		required_string_fred("$Formula:");
		parse_comments();
		convert_sexp_to_string(Mission_goals[i].formula, out, SEXP_SAVE_MODE);
		fout(" %s", out);

		if ( Mission_goals[i].type & INVALID_GOAL ) {
			if (optional_string_fred("+Invalid", "$Type:"))
				parse_comments();
			else
				fout("\n+Invalid");
		}

		if ( Mission_goals[i].flags & MGF_NO_MUSIC ) {
			if (optional_string_fred("+No music", "$Type:"))
				parse_comments();
			else
				fout("\n+No music");
		}

		if ( Mission_goals[i].score != 0 ) {
			if ( optional_string_fred("+Score:", "$Type:"))
				parse_comments();
			else
				fout("\n+Score:");
			fout(" %d", Mission_goals[i].score );
		}

		if ( The_mission.game_type & MISSION_TYPE_MULTI_TEAMS ) {
			if ( optional_string_fred("+Team:", "$Type:"))
				parse_comments();
			else
				fout("\n+Team:");
			fout(" %d", Mission_goals[i].team );
		}
	}

	return err;
}

int CFred_mission_save::save_waypoints()
{
	int i;
	//object *ptr;

	fred_parse_flag = 0;
	required_string_fred("#Waypoints");
	parse_comments(2);
	fout("\t\t;! %d lists total\n", Num_waypoint_lists);

	for ( jump_node *jnp = (jump_node *)Jump_nodes.get_first(); !Jump_nodes.is_end(jnp); jnp = (jump_node *)jnp->get_next() )
	{
		required_string_fred("$Jump Node:", "$Jump Node Name:");
		parse_comments(2);
		save_vector(jnp->get_obj()->pos);

		required_string_fred("$Jump Node Name:", "$Jump Node:");
		parse_comments();
		fout(" %s", jnp->get_name_ptr());
		
		if(jnp->is_special_model())
		{
			if ( optional_string_fred("+Model File:", "$Jump Node:"))
				parse_comments();
			else
				fout("\n+Model File:");

			int model = jnp->get_modelnum();
			polymodel *pm = model_get(model);
			fout(" %s", pm->filename );
		}

		if(jnp->is_colored())
		{
			if ( optional_string_fred("+Alphacolor:", "$Jump Node:"))
				parse_comments();
			else
				fout("\n+Alphacolor:");

			color jn_color = jnp->get_color();
			fout(" %u %u %u %u", jn_color.red, jn_color.green, jn_color.blue, jn_color.alpha );
		}

		int hidden_is_there = optional_string_fred("+Hidden:", "$Jump Node:");
		if(hidden_is_there)
			parse_comments();

		if(hidden_is_there || jnp->is_hidden())
		{
			if(!hidden_is_there)
				fout("\n+Hidden:");

			if(jnp->is_hidden())
				fout(" %s", "true");
			else
				fout(" %s", "false");
		}
	}

	for (i=0; i<Num_waypoint_lists; i++)
	{
		required_string_either_fred("$Name:", "#Messages");
		required_string_fred("$Name:");
		parse_comments(i ? 2 : 1);
		fout(" %s", Waypoint_lists[i].name);

		required_string_fred("$List:");
		parse_comments();
		fout(" (\t\t;! %d points in list\n", Waypoint_lists[i].count);

		save_waypoint_list(Waypoint_lists[i]);
		fout(")");
	}

	return err;
}

int CFred_mission_save::save_waypoint_list(waypoint_list &w)
{
	int i;

	for (i=0; i<w.count; i++)
		fout("\t( %f, %f, %f )\n", w.waypoints[i].xyz.x, w.waypoints[i].xyz.y, w.waypoints[i].xyz.z);

	return 0;
}

int CFred_mission_save::save_messages()
{
	int i;

	fred_parse_flag = 0;
	required_string_fred("#Messages");
	parse_comments(2);
	fout("\t\t;! %d total\n", Num_messages-Num_builtin_messages);

	// Goober5000 - special Command info
	if (Format_fs2_open)
	{
		if (stricmp(The_mission.command_sender, DEFAULT_COMMAND))
			fout("\n$Command Sender: %s", The_mission.command_sender);

		if (The_mission.command_persona != Default_command_persona)
			fout("\n$Command Persona: %s", Personas[The_mission.command_persona].name);
	}

	for (i=Num_builtin_messages; i<Num_messages; i++) {
		required_string_either_fred("$Name:", "#Reinforcements");
		required_string_fred("$Name:");
		parse_comments(2);
		fout(" %s", Messages[i].name);

		// team
		required_string_fred("$Team:");
		parse_comments(1);
		if((Messages[i].multi_team < 0) || (Messages[i].multi_team >= 2)){
			fout(" %d", -1);
		} else {
			fout(" %d", Messages[i].multi_team);
		}

		// XSTR
		required_string_fred("$MessageNew:");
		parse_comments();
		fout(" ");
		fout_ext("%s", Messages[i].message);
		fout("\n");
		required_string_fred("$end_multi_text");
		parse_comments(0);

		if ( Messages[i].persona_index != -1 ) {
			if ( optional_string_fred("+Persona:", "$Name:"))
				parse_comments();
			else
				fout("\n+Persona:");

			fout(" %s", Personas[Messages[i].persona_index].name );
		}

		if (Messages[i].avi_info.name) {
			if (optional_string_fred("+AVI Name:", "$Name:"))
				parse_comments();
			else
				fout("\n+AVI Name:");

			fout(" %s", Messages[i].avi_info.name);
		}

		if (Messages[i].wave_info.name) {
			if (optional_string_fred("+Wave Name:", "$Name:"))
				parse_comments();
			else
				fout("\n+Wave Name:");

			fout(" %s", Messages[i].wave_info.name);
		}
	}

	return err;
}

int CFred_mission_save::save_vector(vec3d &v)
{
	fout(" %f, %f, %f", v.xyz.x, v.xyz.y, v.xyz.z);
	return 0;
}

int CFred_mission_save::save_matrix(matrix &m)
{
	fout("\n\t%f, %f, %f,\n", m.vec.rvec.xyz.x, m.vec.rvec.xyz.y, m.vec.rvec.xyz.z);
	fout("\t%f, %f, %f,\n",   m.vec.uvec.xyz.x, m.vec.uvec.xyz.y, m.vec.uvec.xyz.z);
	fout("\t%f, %f, %f",      m.vec.fvec.xyz.x, m.vec.fvec.xyz.y, m.vec.fvec.xyz.z);
	return 0;
}

// Goober5000 - move past the comment without copying it to the output file
// (used for special FSO comment tags)
void CFred_mission_save::bypass_comment(char *comment)
{
	char *ch = strstr(raw_ptr, comment);
	if (ch != NULL)
	{
		char *writep = ch;
		char *readp = strchr(writep, '\n');

		// copy all characters past it
		while (*readp != '\0')
		{
			*writep = *readp;

			writep++;
			readp++;
		}

		*writep = '\0';
	}
}

// saves comments from previous campaign/mission file
void CFred_mission_save::parse_comments(int newlines)
{
	char *comment_start = NULL;
	int state = 0, same_line = 0, first_comment = 1, tab = 0, flag = 0;

	if (newlines < 0) {
		newlines = -newlines;
		tab = 1;
	}

	if (newlines)
		same_line = 1;

	if (fred_parse_flag || !Token_found_flag || !token_found || (token_found && (*Mission_text_raw == EOF_CHAR))) {
		while (newlines-- > 0)
			fout("\n");

		if (tab)
			fout("\t");

		if (token_found)
			fout("%s", token_found);

		return;
	}

	while (*raw_ptr != EOF_CHAR) {
		if (!state) {
			if (token_found && (*raw_ptr == *token_found))
				if (!strnicmp(raw_ptr, token_found, strlen(token_found))) {
					same_line = newlines - 1 + same_line;
					while (same_line-- > 0)
						fout("\n");
					
					if (tab)
						fout("\t");

					fout("%s", token_found);
					return;
				}

			if ((*raw_ptr == '/') && (raw_ptr[1] == '*')) {
				comment_start = raw_ptr;
				state = 1;
			}

			if ((*raw_ptr == ';') && (raw_ptr[1] != '!')) {
				comment_start = raw_ptr;
				state = 2;
			}

			if ((*raw_ptr == '/') && (raw_ptr[1] == '/')) {
				comment_start = raw_ptr;
				state = 2;
			}

			if (*raw_ptr == '\n')
				flag = 1;

			if (state && flag)
				fout("\n");

		} else {
			if ((*raw_ptr == '\n') && (state == 2)) {
				if (first_comment && !flag)
					fout("\t\t");

				*raw_ptr = 0;
				fout("%s\n", comment_start);
				*raw_ptr = '\n';
				state = first_comment = same_line = flag = 0;
			}

			if ((*raw_ptr == '*') && (raw_ptr[1] == '/') && (state == 1)) {
				if (first_comment && !flag)
					fout("\t\t");

				state = raw_ptr[2];
				raw_ptr[2] = 0;
				fout("%s", comment_start);
				raw_ptr[2] = (char)state;
				state = first_comment = flag = 0;
			}
		}

		raw_ptr++;
	}

	return;
}

// Goober5000
int CFred_mission_save::fout_and_bypass(char *format, ...)
{
	char str[16384];
	va_list args;
	int len;
	char *ch;

	if (err)
		return err;

	// only mess with stuff if we have a special tag
	ch = strstr(format, ";;FSO");
	if (ch != NULL)
	{
		// we might have put something in front of the special tag
		if (ch != format)
		{
			// grab those characters
			len = ch - format;
			strncpy(str, format, len);
			str[len] = '\0';
			format += len;

			// output them
			cfputs(str, fp);
		}

		// assume the tag ends with a colon
		ch = strstr(format, ":");
		if (ch != NULL)
		{
			// grab the tag
			len = ch - format + 1;
			strncpy(str, format, len);
			str[len] = '\0';

			// bypass it
			bypass_comment(str);
		}
	}
	
	// now do a fout as usual
	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);
	Assert(strlen(str) < 16384);

	cfputs(str, fp);
	return 0;
}

int CFred_mission_save::fout(char *format, ...)
{
	char str[16384];
	va_list args;
	
	if (err){
		return err;
	}

	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);
	Assert(strlen(str) < 16384);

	cfputs(str, fp);
	return 0;
}

int CFred_mission_save::fout_ext(char *format, ...)
{
	char str[16384];
	char str_out[16384] = "";
	va_list args;
	int str_id;
	
	if (err){
		return err;
	}

	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);
	Assert(strlen(str) < 16384);

	// lookup the string in the hash table
	str_id = fhash_string_exists(str);
	// doesn't exist, so assign it an ID of -1 and stick it in the table
	if(str_id <= -2){
		sprintf(str_out, " XSTR(\"%s\", -1)", str);

		// add the string to the table		
		fhash_add_str(str, -1);
	}
	// _does_ exist, so just write it out as it is
	else {
		sprintf(str_out, " XSTR(\"%s\", %d)", str, str_id);
	}

	cfputs(str_out, fp);
	return 0;
}

void CFred_mission_save::save_ai_goals(ai_goal *goalp, int ship)
{
	char *str = NULL, buf[80];
	int i, valid, flag = 1;

	for (i=0; i<MAX_AI_GOALS; i++) {
		if (goalp[i].ai_mode == AI_GOAL_NONE)
			continue;

		if (flag) {
			if (optional_string_fred("$AI Goals:", "$Name:"))
				parse_comments();
			else
				fout("\n$AI Goals:");

			fout(" ( goals ");
			flag = 0;
		}

		if (goalp[i].ai_mode == AI_GOAL_CHASE_ANY) {
			fout("( ai-chase-any %d ) ", goalp[i].priority);

		} else if (goalp[i].ai_mode == AI_GOAL_UNDOCK) {
			fout("( ai-undock %d ) ", goalp[i].priority);

		} else if (goalp[i].ai_mode == AI_GOAL_KEEP_SAFE_DISTANCE) {
			fout("( ai-keep-safe-distance %d ) ", goalp[i].priority);
		
		} else if (goalp[i].ai_mode == AI_GOAL_PLAY_DEAD) {
			fout("( ai-play-dead %d ) ", goalp[i].priority);

		} else if (goalp[i].ai_mode == AI_GOAL_WARP) {
			fout("( ai-warp-out %d ) ", goalp[i].priority);
		
		} else {
			valid = 1;
			if (!goalp[i].ship_name) {
				Warning(LOCATION, "Ai goal has no target where one is required");

			} else {
				sprintf(buf, "\"%s\"", goalp[i].ship_name);
				switch (goalp[i].ai_mode) {
					case AI_GOAL_WAYPOINTS:
						str = "ai-waypoints";
						break;

					case AI_GOAL_WAYPOINTS_ONCE:
						str = "ai-waypoints-once";
						break;

					case AI_GOAL_DESTROY_SUBSYSTEM:
						if (goalp[i].docker.index == -1 || !goalp[i].docker.index) {
							valid = 0;
							Warning(LOCATION, "AI destroy subsystem goal invalid subsystem name\n");

						} else {
							sprintf(buf, "\"%s\" \"%s\"", goalp[i].ship_name, goalp[i].docker.name);
							str = "ai-destroy-subsystem";
						}

						break;

					case AI_GOAL_DOCK:
						if (ship < 0) {
							valid = 0;
							Warning(LOCATION, "Wings aren't allowed to have a docking goal\n");
							
						} else if (goalp[i].docker.index == -1 || !goalp[i].docker.index) {
							valid = 0;
							Warning(LOCATION, "AI dock goal for \"%s\" has invalid docker point "
								"(docking with \"%s\")\n", Ships[ship].ship_name, goalp[i].ship_name);

						} else if (goalp[i].dockee.index == -1 || !goalp[i].dockee.index) {
							valid = 0;
							Warning(LOCATION, "AI dock goal for \"%s\" has invalid dockee point "
								"(docking with \"%s\")\n", Ships[ship].ship_name, goalp[i].ship_name);

						} else {
							sprintf(buf, "\"%s\" \"%s\" \"%s\"", goalp[i].ship_name,
								goalp[i].docker.name, goalp[i].dockee.name);

							str = "ai-dock";
						}
						break;

					case AI_GOAL_CHASE:
						str = "ai-chase";
						break;

					case AI_GOAL_CHASE_WING:
						str = "ai-chase-wing";
						break;

					case AI_GOAL_GUARD:
						str = "ai-guard";
						break;

					case AI_GOAL_GUARD_WING:
						str = "ai-guard-wing";
						break;

					case AI_GOAL_DISABLE_SHIP:
						str = "ai-disable-ship";
						break;

					case AI_GOAL_DISARM_SHIP:
						str = "ai-disarm-ship";
						break;

					case AI_GOAL_IGNORE:
						str = "ai-ignore";
						break;

					case AI_GOAL_IGNORE_NEW:
						str = "ai-ignore-new";
						break;

					case AI_GOAL_EVADE_SHIP:
						str = "ai-evade-ship";
						break;

					case AI_GOAL_STAY_NEAR_SHIP:
						str = "ai-stay-near-ship";
						break;

					case AI_GOAL_STAY_STILL:
						str = "ai-stay-still";
						break;

					default:
						Assert(0);
				}

				if (valid)
					fout("( %s %s %d ) ", str, buf, goalp[i].priority);
			}
		}
	}

	if (!flag)
		fout(")");
}

int CFred_mission_save::save_events()
{
	char out[4096];
	int i;

	fred_parse_flag = 0;
	required_string_fred("#Events");
	parse_comments(2);
	fout("\t\t;! %d total\n", Num_mission_events);

	for (i=0; i<Num_mission_events; i++) {
		required_string_either_fred("$Formula:", "#Goals");
		required_string_fred("$Formula:");
		parse_comments(i ? 2 : 1);
		convert_sexp_to_string(Mission_events[i].formula, out, SEXP_SAVE_MODE);
		fout(" %s", out);

		if (*Mission_events[i].name) {
			if (optional_string_fred("+Name:", "$Formula:")){
				parse_comments();
			} else {
				fout("\n+Name:");
			}

			fout(" %s", Mission_events[i].name);
		}

		if ( optional_string_fred("+Repeat Count:", "$Formula:")){
			parse_comments();
		} else {
			fout("\n+Repeat Count:");
		}

		fout(" %d", Mission_events[i].repeat_count);

		if ( optional_string_fred("+Interval:", "$Formula:")){
			parse_comments();
		} else {
			fout("\n+Interval:");
		}

		fout(" %d", Mission_events[i].interval);

		if ( Mission_events[i].score != 0 ) {
			if ( optional_string_fred("+Score:", "$Formula:")){
				parse_comments();
			} else {
				fout("\n+Score:");
			}
			fout(" %d", Mission_events[i].score);
		}

		if ( Mission_events[i].chain_delay >= 0 ) {
			if ( optional_string_fred("+Chained:", "$Formula:")){
				parse_comments();
			} else {
				fout("\n+Chained:");
			}

			fout(" %d", Mission_events[i].chain_delay);
		}

		//XSTR
		if (Mission_events[i].objective_text) {
			if (optional_string_fred("+Objective:", "$Formula:")){
				parse_comments();
			} else {
				fout("\n+Objective:");
			}

			fout(" ");
			fout_ext("%s", Mission_events[i].objective_text);
		}

		//XSTR
		if (Mission_events[i].objective_key_text) {
			if (optional_string_fred("+Objective key:", "$Formula:")){
				parse_comments();
			} else {
				fout("\n+Objective key:");
			}

			fout(" ");
			fout_ext("%s", Mission_events[i].objective_key_text);
		}

		// save team
		if (Mission_events[i].team >= 0){
			if (optional_string_fred("+Team:")){
				parse_comments();
			} else {
				fout("\n+Team:");
			} 
			fout(" ");
			fout("%d", Mission_events[i].team);
		}
	}

	return err;
}

int CFred_mission_save::save_reinforcements()
{
	int i, j, type;

	fred_parse_flag = 0;
	required_string_fred("#Reinforcements");
	parse_comments(2);
	fout("\t\t;! %d total\n", Num_reinforcements);

	for (i=0; i<Num_reinforcements; i++) {
		required_string_either_fred("$Name:", "#Background bitmaps");
		required_string_fred("$Name:");
		parse_comments(i ? 2 : 1);
		fout(" %s", Reinforcements[i].name);

		type = TYPE_ATTACK_PROTECT;
		for (j=0; j<MAX_SHIPS; j++)
			if ((Ships[j].objnum != -1) && !stricmp(Ships[j].ship_name, Reinforcements[i].name)) {
				if (Ship_info[Ships[j].ship_info_index].flags & SIF_SUPPORT)
					type = TYPE_REPAIR_REARM;
				break;
			}

		required_string_fred("$Type:");
		parse_comments();
		fout(" %s", Reinforcement_type_names[type]);

		required_string_fred("$Num times:");
		parse_comments();
		fout(" %d", Reinforcements[i].uses);

		if ( optional_string_fred("+Arrival Delay:", "$Name:"))
			parse_comments();
		else
			fout("\n+Arrival Delay:");
		fout(" %d", Reinforcements[i].arrival_delay );

		if (optional_string_fred("+No Messages:", "$Name:"))
			parse_comments();
		else
			fout("\n+No Messages:");
		fout(" (");
		for (j = 0; j < MAX_REINFORCEMENT_MESSAGES; j++) {
			if ( strlen(Reinforcements[i].no_messages[j]) )
				fout(" \"%s\"", Reinforcements[i].no_messages[j]);
		}
		fout(" )");

		if (optional_string_fred("+Yes Messages:", "$Name:"))
			parse_comments();
		else
			fout("\n+Yes Messages:");
		fout(" (");
		for (j = 0; j < MAX_REINFORCEMENT_MESSAGES; j++) {
			if ( strlen(Reinforcements[i].yes_messages[j]) )
				fout(" \"%s\"", Reinforcements[i].yes_messages[j]);
		}
		fout(" )");

	}

	return err;
}

int CFred_mission_save::save_bitmaps()
{	
	int i;
	uint j;

	fred_parse_flag = 0;
	required_string_fred("#Background bitmaps");
	parse_comments(2);
	fout("\t\t;! %d total\n", stars_get_num_bitmaps());

	required_string_fred("$Num stars:");
	parse_comments();
	fout(" %d", Num_stars);

	required_string_fred("$Ambient light level:");
	parse_comments();
	fout(" %d", The_mission.ambient_light_level);

	// neb2 stuff
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		required_string_fred("+Neb2:");
		parse_comments();
		fout(" %s\n", Neb2_texture_name);

		required_string_fred("+Neb2Flags:");
		parse_comments();
		fout(" %d\n", Neb2_poof_flags);
	}
	// neb 1 stuff
	else {
		if (Nebula_index >= 0) {
			if (optional_string_fred("+Nebula:")){
				parse_comments();
			} else {
				fout("\n+Nebula:");
			}		
			fout(" %s", Nebula_filenames[Nebula_index]);		

			required_string_fred("+Color:");
			parse_comments();
			fout(" %s", Nebula_colors[Mission_palette]);

			required_string_fred("+Pitch:");
			parse_comments();
			fout(" %d", Nebula_pitch);

			required_string_fred("+Bank:");
			parse_comments();
			fout(" %d", Nebula_bank);

			required_string_fred("+Heading:");
			parse_comments();
			fout(" %d", Nebula_heading);
		}
	}

	// Goober5000 - save all but the lowest priority using the special comment tag
	for (i = 0; i < Num_backgrounds; i++)
	{
		bool tag = (i < Num_backgrounds - 1);
		background_t *background = &Backgrounds[i];

		fout_and_bypass("\n\n;;FSO 3.6.9;; $Bitmap List:");

		// save suns by filename
		for (j = 0; j < background->suns.size(); j++)
		{
			starfield_list_entry *sle = &background->suns[j];

			if (tag)
			{
				// filename
				fout_and_bypass("\n;;FSO 3.6.9;; $Sun: %s", sle->filename);

				// angles
				fout_and_bypass("\n;;FSO 3.6.9;; +Angles: %f %f %f", sle->ang.p, sle->ang.b, sle->ang.h);

				// scale
				fout_and_bypass("\n;;FSO 3.6.9;; +Scale: %f", sle->scale_x);
			}
			else
			{
				// filename
				required_string_fred("$Sun:");
				parse_comments();
				fout(" %s", sle->filename);

				// angles
		 		required_string_fred("+Angles:");
 				parse_comments();
				fout(" %f %f %f", sle->ang.p, sle->ang.b, sle->ang.h);

				// scale
		 		required_string_fred("+Scale:");
 				parse_comments();
				fout(" %f", sle->scale_x);
			}
		}

		// save background bitmaps by filename
		for (j = 0; j < background->bitmaps.size(); j++)
		{
			starfield_list_entry *sle = &background->bitmaps[j];

			if (tag)
			{
				// filename
				fout_and_bypass("\n;;FSO 3.6.9;; $Starbitmap: %s", sle->filename);

				// angles
				fout_and_bypass("\n;;FSO 3.6.9;; +Angles: %f %f %f", sle->ang.p, sle->ang.b, sle->ang.h);

				// scale
				fout_and_bypass("\n;;FSO 3.6.9;; +ScaleX: %f", sle->scale_x);
				fout_and_bypass("\n;;FSO 3.6.9;; +ScaleY: %f", sle->scale_y);

				// div
				fout_and_bypass("\n;;FSO 3.6.9;; +DivX: %d", sle->div_x);
				fout_and_bypass("\n;;FSO 3.6.9;; +DivY: %d", sle->div_y);
			}
			else
			{
				// filename
				required_string_fred("$Starbitmap:");
		 		parse_comments();
				fout(" %s", sle->filename);

				// angles
		 		required_string_fred("+Angles:");
 				parse_comments();
				fout(" %f %f %f", sle->ang.p, sle->ang.b, sle->ang.h);

				// scale
		 		required_string_fred("+ScaleX:");
 				parse_comments();
				fout(" %f", sle->scale_x);
 				required_string_fred("+ScaleY:");
 				parse_comments();
				fout(" %f", sle->scale_y);

				// div
		 		required_string_fred("+DivX:");
		 		parse_comments();
				fout(" %d", sle->div_x);
		 		required_string_fred("+DivY:");
 				parse_comments();
				fout(" %d", sle->div_y);
			}
		}
 	}

	// taylor's environment map thingy
	if (strlen(The_mission.envmap_name) > 0)
		fout_and_bypass("\n\n;;FSO 3.6.9;; $Environment Map: %s", The_mission.envmap_name);

	return err;
}

int CFred_mission_save::save_asteroid_fields()
{
	int i, idx;

	fred_parse_flag = 0;
	required_string_fred("#Asteroid Fields");
	parse_comments(2);

	for (i=0; i<1 /*MAX_ASTEROID_FIELDS*/; i++) {
		if (!Asteroid_field.num_initial_asteroids)
			continue;

		required_string_fred("$Density:");
		parse_comments(2);
		fout(" %d", Asteroid_field.num_initial_asteroids);

		// field type
		if (optional_string_fred("+Field Type:")){
			parse_comments();
		} else {
			fout("\n+Field Type:");
		}
		fout(" %d", Asteroid_field.field_type);

		// debris type
		if (optional_string_fred("+Debris Genre:")){
			parse_comments();
		} else {
			fout("\n+Debris Genre:");
		}
		fout(" %d", Asteroid_field.debris_genre);

		// field_debris_type (only if ship genre)
		if (Asteroid_field.debris_genre == DG_SHIP) {
			for (int idx=0; idx<3; idx++) {
				if (Asteroid_field.field_debris_type[idx] != -1) {
					if (optional_string_fred("+Field Debris Type:")){
						parse_comments();
					} else {
						fout("\n+Field Debris Type:");
					}
					fout(" %d", Asteroid_field.field_debris_type[idx]);
				}
			}
		} else {
			// asteroid subtypes stored in field_debris_type as -1 or 1
			for (idx=0; idx<3; idx++) {
				if (Asteroid_field.field_debris_type[idx] != -1) {
					if (optional_string_fred("+Field Debris Type:")){
						parse_comments();
					} else {
						fout("\n+Field Debris Type:");
					}
					fout(" %d", idx);
				}
			}
		}


		required_string_fred("$Average Speed:");
		parse_comments();
		fout(" %f", vm_vec_mag(&Asteroid_field.vel));

		required_string_fred("$Minimum:");
		parse_comments();
		save_vector(Asteroid_field.min_bound);

		required_string_fred("$Maximum:");
		parse_comments();
		save_vector(Asteroid_field.max_bound);

		if (Asteroid_field.has_inner_bound == 1) {
			if (optional_string_fred("+Inner Bound:")){
				parse_comments();
			} else {
				fout("\n+Inner Bound:");
			}

			required_string_fred("$Minimum:");
			parse_comments();
			save_vector(Asteroid_field.inner_min_bound);

			required_string_fred("$Maximum:");
			parse_comments();
			save_vector(Asteroid_field.inner_max_bound);
		}
	}

	return err;
}

int CFred_mission_save::save_music()
{
	required_string_fred("#Music");
	parse_comments(2);

	required_string_fred("$Event Music:");
	parse_comments(2);
	if (Current_soundtrack_num < 0)
		fout(" None");
	else
		fout(" %s", Soundtracks[Current_soundtrack_num].name);

	// Goober5000 - save using the special comment prefix
	if (stricmp(The_mission.substitute_event_music_name, "None"))
		fout_and_bypass("\n;;FSO 3.6.9;; $Substitute Event Music: %s", The_mission.substitute_event_music_name);

	required_string_fred("$Briefing Music:");
	parse_comments();
	if (Mission_music[SCORE_BRIEFING] < 0)
		fout(" None");
	else
		fout(" %s", Spooled_music[Mission_music[SCORE_BRIEFING]].name);

	// Goober5000 - save using the special comment prefix
	if (stricmp(The_mission.substitute_briefing_music_name, "None"))
		fout_and_bypass("\n;;FSO 3.6.9;; $Substitute Briefing Music: %s", The_mission.substitute_briefing_music_name);

	// avoid keeping the old one around
	bypass_comment(";;FSO 3.6.8;; $Substitute Music:");

	return err;
}

void CFred_mission_save::save_turret_info(ship_subsys *ptr, int ship)
{
	int i, z;
	ship_weapon *wp = &ptr->weapons;

	if (wp->ai_class != Ship_info[Ships[ship].ship_info_index].ai_class) {
		if (optional_string_fred("+AI Class:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+AI Class:");

		fout(" %s", Ai_class_names[wp->ai_class]);
	}

	z = 0;
	i = wp->num_primary_banks;
	while (i--)
		if (wp->primary_bank_weapons[i] != ptr->system_info->primary_banks[i])
			z = 1;

	if (z) {
		if (optional_string_fred("+Primary Banks:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Primary Banks:");

		fout(" ( ");
		for (i=0; i<wp->num_primary_banks; i++)
			fout("\"%s\" ", Weapon_info[wp->primary_bank_weapons[i]].name);

		fout(")");
	}

	z = 0;
	i = wp->num_secondary_banks;
	while (i--)
		if (wp->secondary_bank_weapons[i] != ptr->system_info->secondary_banks[i])
			z = 1;

	if (z) {
		if (optional_string_fred("+Secondary Banks:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Secondary Banks:");

		fout(" ( ");
		for (i=0; i<wp->num_secondary_banks; i++)
			fout("\"%s\" ", Weapon_info[wp->secondary_bank_weapons[i]].name);

		fout(")");
	}

	z = 0;
	i = wp->num_secondary_banks;
	while (i--)
		if (wp->secondary_bank_ammo[i] != 100)
			z = 1;

	if (z) {
		if (optional_string_fred("+Sbank Ammo:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Sbank Ammo:");

		fout(" ( ");
		for (i=0; i<wp->num_secondary_banks; i++)
			fout("%d ", wp->secondary_bank_ammo[i]);

		fout(")");
	}
}

int CFred_mission_save::save_campaign_file(char *pathname)
{
	int i, j, m, flag;

	Campaign_tree_formp->save_tree();  // flush all changes so they get saved.
	Campaign_tree_viewp->sort_elements();
	reset_parse();
	fred_parse_flag = 0;

	pathname = cf_add_ext(pathname, FS_CAMPAIGN_FILE_EXT);
	fp = cfopen(pathname, "wt", CFILE_NORMAL, CF_TYPE_MISSIONS);
	if (!fp)	{
		nprintf(("Error", "Can't open campaign file to save.\n"));
		return -1;
	}

	required_string_fred("$Name:");
	parse_comments(0);
	fout(" %s", Campaign.name);

	Assert((Campaign.type >= 0) && (Campaign.type < MAX_CAMPAIGN_TYPES));
	required_string_fred("$Type:");
	parse_comments();
	fout(" %s", campaign_types[Campaign.type]);

	// XSTR
	if (Campaign.desc) {
		required_string_fred("+Description:");
		parse_comments();
		fout("\n");
		fout_ext("%s", Campaign.desc);
		fout("\n$end_multi_text");
	}

	if ( Campaign.type != CAMPAIGN_TYPE_SINGLE ) {
		required_string_fred("+Num Players:");
		parse_comments();
		fout(" %d", Campaign.num_players);
	}	

	// campaign flags - Goober5000
	if (Format_fs2_open)
	{
		optional_string_fred("$Flags:");
		parse_comments();
		fout(" %d\n", Campaign.flags);
	}

	// write out the ships and weapons which the player can start the campaign with
	optional_string_fred("+Starting Ships: (");
	parse_comments(2);
	for (i = 0; i < MAX_SHIP_CLASSES; i++ ) {
		if ( Campaign.ships_allowed[i] )
			fout(" \"%s\" ", Ship_info[i].name );
	}
	fout( ")\n" );

	optional_string_fred("+Starting Weapons: (");
	parse_comments();
	for (i = 0; i < MAX_WEAPON_TYPES; i++ ) {
		if ( Campaign.weapons_allowed[i] )
			fout(" \"%s\" ", Weapon_info[i].name );
	}
	fout( ")\n" );

	fred_parse_flag = 0;
	for (i=0; i<Campaign.num_missions; i++) {
		m = Sorted[i];
		required_string_either_fred("$Mission:", "#End");
		required_string_fred("$Mission:");
		parse_comments(2);
		fout(" %s", Campaign.missions[m].name);

		if ( strlen(Campaign.missions[i].briefing_cutscene) ) {
			if (optional_string_fred("+Briefing Cutscene:", "$Mission"))
				parse_comments();
			else
				fout("\n+Briefing Cutscene:");

			fout( " %s", Campaign.missions[i].briefing_cutscene );
		}

		required_string_fred("+Flags:", "$Mission:");
		parse_comments();

		// Goober5000
		if (Format_fs2_open)
		{
			// don't save Bastion flag
			fout(" %d", Campaign.missions[m].flags & ~CMISSION_FLAG_BASTION);

			// new main hall stuff
			if (optional_string_fred("+Main Hall:", "$Mission:"))
				parse_comments();
			else
				fout("\n+Main Hall:");

			fout(" %d", Campaign.missions[m].main_hall);
		}
		else
		{
			// save Bastion flag properly
			fout(" %d", Campaign.missions[m].flags | ((Campaign.missions[m].main_hall > 0) ? CMISSION_FLAG_BASTION : 0));
		}

		// Goober5000
		// unfortunately, retail FRED doesn't preserve comments placed here... however since campaigns are
		// very seldom edited, this shouldn't be too big of a problem
		if (Campaign.missions[m].debrief_persona_index >= 0 && Campaign.missions[m].debrief_persona_index <= 0xff)
		{
			fout_and_bypass("\n;;FSO 3.6.8;; +Debriefing Persona Index: %d", Campaign.missions[m].debrief_persona_index);
		}

		// save campaign link sexp
		bool mission_loop = false;
		flag = 0;
		for (j=0; j<Total_links; j++) {
			if (Links[j].from == m) {
				if (!flag) {
					if (optional_string_fred("+Formula:", "$Mission:"))
						parse_comments();
					else
						fout("\n+Formula:");

					fout(" ( cond\n");
					flag = 1;
				}

				//save_campaign_sexp(Links[j].sexp, Campaign.missions[Links[j].to].name);
				if (Links[j].mission_loop) {
					mission_loop = true;
				} else {
					save_campaign_sexp(Links[j].sexp, Links[j].to);
				}
			}
		}

		if (flag) {
			fout(")");
		}

		// now save campaign loop sexp
		if (mission_loop) {
			required_string_fred("\n+Mission Loop:");
			parse_comments();

			int num_mission_loop = 0;
			for (j=0; j<Total_links; j++) {
				if ( (Links[j].from == m) && (Links[j].mission_loop) ) {

					num_mission_loop++;

					// maybe write out mission loop descript
					if ((num_mission_loop == 1) && Links[j].mission_loop_txt) {
						required_string_fred("+Mission Loop Text:");
						parse_comments();
						fout("\n");
						fout_ext("%s", Links[j].mission_loop_txt);
						fout("\n$end_multi_text");
					}

					// maybe write out mission loop descript
					if ((num_mission_loop == 1) && Links[j].mission_loop_brief_anim) {
						required_string_fred("+Mission Loop Brief Anim:");
						parse_comments();
						fout("\n");
						fout_ext("%s", Links[j].mission_loop_brief_anim);
						fout("\n$end_multi_text");
					}

					// maybe write out mission loop descript
					if ((num_mission_loop == 1) && Links[j].mission_loop_brief_sound) {
						required_string_fred("+Mission Loop Brief Sound:");
						parse_comments();
						fout("\n");
						fout_ext("%s", Links[j].mission_loop_brief_sound);
						fout("\n$end_multi_text");
					}

					if (num_mission_loop == 1) {
						// write out mission loop formula
						fout("\n+Formula:");
						fout(" ( cond\n");
						save_campaign_sexp(Links[j].sexp, Links[j].to);
						fout(")");
					}
				}
			}
			if (num_mission_loop > 1) {
				char buffer[1024];
				sprintf(buffer, "Multiple branching loop error from mission %s\nEdit campaign for *at most* 1 loop from each mission.", Campaign.missions[m].name);
				MessageBox((HWND)os_get_window(), buffer, "Error", MB_OK);
			}
		}

		if (optional_string_fred("+Level:", "$Mission:")){
			parse_comments();
		} else {
			fout("\n\n+Level:");
		}

		fout(" %d", Campaign.missions[m].level);

		if (optional_string_fred("+Position:", "$Mission:")){
			parse_comments();
		} else {
			fout("\n+Position:");
		}

		fout(" %d", Campaign.missions[m].pos);
	}

	required_string_fred("#End");
	parse_comments(2);
	token_found = NULL;
	parse_comments();
	fout("\n");

	cfclose(fp);
	if (err)
		mprintf(("Campaign saving error code #%d", err));
	else
		Campaign_wnd->error_checker();

	return err;
}

void CFred_mission_save::save_campaign_sexp(int node, int link_num)
{
	char out[4096];

	Sexp_string = out;
	*out = 0;
	Assert(node >= 0);

	// if the link num is -1, then this is a end-of-campaign location
	if ( link_num != -1 ) {
		if (build_sexp_string(node, 2, SEXP_SAVE_MODE))
			fout("   (\n      %s\n      ( next-mission \"%s\" )\n   )\n", out, Campaign.missions[link_num].name);
		else
			fout("   ( %s( next-mission \"%s\" ) )\n", out, Campaign.missions[link_num].name);
	} else {
		if (build_sexp_string(node, 2, SEXP_SAVE_MODE)){
			fout("   (\n      %s\n      ( end-of-campaign )\n   )\n", out);
		} else {
			fout("   ( %s( end-of-campaign ) )\n", out );
		}
	}
}


