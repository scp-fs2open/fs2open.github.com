/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Fred2/FREDDoc.cpp $
 * $Revision: 1.6.2.10 $
 * $Date: 2008-01-17 07:43:23 $
 * $Author: Goober5000 $
 *
 * FREDDoc.cpp : implementation of the CFREDDoc class
 * Document class for document/view architechure, which we don't really use in
 * Fred, but MFC forces you do use like it or not.  Handles loading/saving
 * mainly.  Most of the MFC related stuff is handled in FredView.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.6.2.9  2007/09/02 02:07:40  Goober5000
 * added fixes for #1415 and #1483, made sure every read_file_text had a corresponding setjmp, and sync'd the parse error messages between HEAD and stable
 *
 * Revision 1.6.2.8  2007/07/28 21:31:04  Goober5000
 * this should really be capitalized
 *
 * Revision 1.6.2.7  2007/05/28 19:47:46  taylor
 * this is a bit cleaner way to do it :)
 *
 * Revision 1.6.2.6  2007/05/26 15:11:15  Goober5000
 * that was weird
 *
 * Revision 1.6.2.5  2007/05/20 21:21:30  wmcoolmon
 * FRED2 support for numbered SEXP operator arguments, minihelp box, fixed "Insert Event" when no events are present.
 *
 * Revision 1.6.2.4  2006/10/08 05:24:03  Goober5000
 * bah!
 *
 * Revision 1.6.2.3  2006/10/08 05:21:35  Goober5000
 * nitpick
 *
 * Revision 1.6.2.2  2006/10/01 00:54:29  Goober5000
 * fix crlf
 *
 * Revision 1.6.2.1  2006/07/10 21:47:17  taylor
 * fix undo system
 * restore the two autosave points that got removed for some reason (yes, these are needed for things to work properly)
 *
 * Revision 1.6  2006/02/11 02:58:23  Goober5000
 * yet more various and sundry fixes
 * --Goober5000
 *
 * Revision 1.5  2006/02/11 00:13:55  Goober5000
 * more FS1 import goodness
 * --Goober5000
 *
 * Revision 1.4  2006/01/31 04:13:00  Goober5000
 * oh noes! :(
 *
 * Revision 1.3  2006/01/31 01:53:36  Goober5000
 * update FSM import for FSPort v3.0
 * --Goober5000
 *
 * Revision 1.2  2006/01/30 06:27:59  taylor
 * dynamic starfield bitmaps
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.12  2005/10/29 23:02:33  Goober5000
 * partial FRED commit of changes
 * --Goober5000
 *
 * Revision 1.11  2005/03/29 03:43:11  phreak
 * ai directory fixes as well as fixes for the new jump node code
 *
 * Revision 1.10  2005/03/21 00:29:25  phreak
 * make sure the starfield bitmaps are generated upon mission load
 *
 * Revision 1.9  2004/07/29 00:18:42  Kazan
 * taylor and I fixed fred2 by making code.lib and fred2's defines match up
 *
 * Revision 1.8  2004/05/02 18:03:19  randomtiger
 * Fred doesnt need subspace palette any more
 *
 * Revision 1.7  2004/04/03 18:06:35  Kazan
 * Fixes:
 *
 * Revision 1.6  2003/10/06 06:25:10  Goober5000
 * added confirmation messages when no errors are detected on import
 * --Goober5000
 *
 * Revision 1.5  2003/09/30 04:05:08  Goober5000
 * updated FRED to import FS1 default weapons loadouts as well as missions
 * --Goober5000
 *
 * Revision 1.4  2003/09/28 21:22:58  Goober5000
 * added the option to import FSM missions, added a replace function, spruced
 * up my $player, $rank, etc. code, and fixed encrypt being misspelled as 'encrpyt'
 * --Goober5000
 *
 * Revision 1.3  2003/01/03 03:12:53  Goober5000
 * updated obselete code for upcoming stuff, just in case it's ever used again :)
 * --Goober5000
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:57  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 7     7/23/99 2:12p Jamesa
 * fix gamepalette request
 * 
 * 6     5/20/99 6:59p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 5     10/29/98 6:49p Dave
 * Finished up Fred support for externalizing mission and campaign files.
 * 
 * 4     10/29/98 12:50p Dave
 * Intermediate checkin for fred hash table stuff.
 * 
 * 3     10/28/98 11:30a Dave
 * Temporary checkin.
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 94    9/21/98 8:46p Dave
 * Put in special check in fred for identifying unknown ships.
 * 
 * 93    9/16/98 6:54p Dave
 * Upped  max sexpression nodes to 1800 (from 1600). Changed FRED to sort
 * the ship list box. Added code so that tracker stats are not stored with
 * only 1 player.
 * 
 * 92    8/13/98 9:53a Hoffoss
 * Fixed bug where loading mission doesn't update palette used for that
 * mission.
 * 
 * 91    4/30/98 8:23p John
 * Fixed some bugs with Fred caused by my new cfile code.
 * 
 * 90    4/28/98 2:13p Hoffoss
 * Added code to help keep invalid player ship types from existing in
 * mission.
 * 
 * 89    4/10/98 10:22p Mike
 * Fix bug in parsing medals.tbl.
 * 
 * 88    4/10/98 4:51p Hoffoss
 * Made several changes related to tooltips.
 * 
 * 87    3/25/98 4:14p Hoffoss
 * Split ship editor up into ship editor and a misc dialog, which tracks
 * flags and such.
 * 
 * 86    3/10/98 4:26p Hoffoss
 * Changed jump node structure to include a name.  Position is now taken
 * from the object (each jump node has an associated object now).
 * 
 * 85    2/26/98 4:59p Allender
 * groundwork for team vs team briefings.  Moved weaponry pool into the
 * Team_data structure.  Added team field into the p_info structure.
 * Allow for mutliple structures in the briefing code.
 * 
 * 84    1/28/98 11:00a Johnson
 * Fixed parse code for medals table (new field)
 * 
 * 83    11/06/97 4:37p Allender
 * a ton of medal work.  Removed an uneeded element in the scoring
 * structure.  Fix up medals screen to apprioriate display medals (after
 * mask was changed). Fix Fred to only display medals which may actually
 * be granted.  Added image_filename to player struct for Jason Hoffoss
 * 
 * 82    11/05/97 10:19p Mike
 * Comment out call to parse_medal_tbl() to allow Fred to run.
 * 
 * 81    11/05/97 4:43p Allender
 * reworked medal/rank system to read all data from tables.  Made Fred
 * read medals.tbl.  Changed ai-warp to ai-warp-out which doesn't require
 * waypoint for activation
 * 
 * 80    11/04/97 4:50p Hoffoss
 * Made mission loading failure display an error message stating so.
 * 
 * 79    11/04/97 4:33p Hoffoss
 * Made saving keep the current briefing state intact.
 * 
 * 78    10/20/97 1:20p Hoffoss
 * Fixed bug loading mission file after a save.
 * 
 * 77    10/19/97 11:36p Hoffoss
 * Made mission saving call reload the mission after saving it to verify
 * it was written error free.
 * 
 * 76    10/08/97 11:47a Hoffoss
 * Added better fred handling of Weaponry_pool.
 * 
 * 75    9/20/97 8:16a John
 * Made .clr files go into the Cache directory. Replaced cfopen(name,NULL)
 * to delete a file with cf_delete.
 * 
 * 74    9/18/97 11:52a Johnson
 * Fixed bug with player starts count being incorrect.
 * 
 * 73    9/17/97 11:58a Hoffoss
 * Fixed mission load code to properly set arrival cues to player ships.
 * 
 * 72    9/17/97 10:32a Allender
 * fixup references to old "Player" ship to new real shipname
 * 
 * 71    9/16/97 9:41p Hoffoss
 * Changed Fred code around to stop using Parse_player structure for
 * player information, and use actual ships instead.
 * 
 * 70    9/09/97 6:50p Hoffoss
 * Fixed bug with mission saving.
 * 
 * 69    9/09/97 1:55p Hoffoss
 * Fixed bug with saving missions that are readonly.
 * 
 * 68    8/21/97 1:49p Hoffoss
 * Fixed bugs with loading files.  Wasn't clearing the last mission first,
 * and multiplayer starts in a mission caused it to think it was modified
 * right away.
 * 
 * 67    8/20/97 5:45p Hoffoss
 * Fixed bugs with mission name in sexp in non campaign mode in Fred.
 * 
 * 66    8/20/97 4:03p Hoffoss
 * Fixed bug in reload current mission (revert).
 * 
 * 65    8/17/97 10:22p Hoffoss
 * Fixed several bugs in Fred with Undo feature.  In the process, recoded
 * a lot of CFile.cpp.
 * 
 * 64    8/16/97 9:24p Hoffoss
 * Added support for team of players in multiplayer.
 * 
 * 63    8/16/97 6:44p Hoffoss
 * Changes to allow any player to be in a wing.
 * 
 * 62    8/16/97 3:53p Hoffoss
 * Added OF_NO_SHIELDS define and support in Fred and mission load/save.
 * 
 * 61    8/13/97 5:49p Hoffoss
 * Fixed bugs, made additions.
 * 
 * 60    8/13/97 12:46p Hoffoss
 * Added campaign error checker, accelerator table, and mission goal data
 * listings to sexp tree right click menu.
 * 
 * 59    8/12/97 1:55a Hoffoss
 * Made extensive changes to object reference checking and handling for
 * object deletion call.
 * 
 * 58    7/31/97 5:55p John
 * made so you pass flags to obj_create.
 * Added new collision code that ignores any pairs that will never
 * collide.
 * 
 * 57    7/30/97 2:10p Hoffoss
 * Made new missions that have never been saved yet save backups.
 * 
 * 56    7/28/97 3:50p Hoffoss
 * Added global error checker call to mission load and save.
 * 
 * 55    7/10/97 2:32p Hoffoss
 * Made message editor dialog box modeless.
 * 
 * 54    7/08/97 11:35a Hoffoss
 * Fixed bug in initial orders, and also redid how shields are handled by
 * Fred.
 * 
 * 53    6/26/97 5:18p Hoffoss
 * Major rework of briefing editor functionality.
 * 
 * 52    6/24/97 10:22a Hoffoss
 * Added a forced update for briefing info prior to a mission save.
 * 
 * 51    6/19/97 11:34a Hoffoss
 * Fixed bug in auto mission convertion.
 * 
 * 50    6/18/97 3:25p Hoffoss
 * Added autoconverting of old ship names (for ships in wings) at mission
 * load time, and added an hourglass cursor change when autosaving.
 * 
 * 49    6/17/97 11:07a Hoffoss
 * Fixed a few bugs: revert doesn't reset mission first, and sexp tree
 * copy doesn't act act as a single chain.
 * 
 * 48    6/11/97 10:02a Comet
 * fixed a bug.
 * 
 * 47    6/09/97 4:57p Hoffoss
 * Added autosave and undo to Fred.
 * 
 * 46    6/05/97 6:10p Hoffoss
 * Added features: Autosaving, object hiding.  Also fixed some minor bugs.
 * 
 * 45    5/23/97 3:18p Hoffoss
 * Event editor made modeless.
 * 
 * 44    5/21/97 5:42p Hoffoss
 * Added features requested on Todo list.
 * 
 * 43    5/01/97 4:14p Hoffoss
 * Viewer position saved to missions now for Fred restoration of state.
 * 
 * 42    4/25/97 3:53p Hoffoss
 * Fixed bug in weapons save/load and moved weapon look up function to
 * weapons.cpp.
 * 
 * 41    4/25/97 3:28p Mike
 * Making shield multi-part.
 * 
 * 40    4/21/97 5:02p Hoffoss
 * Player/player status editing supported, and both saved and loaded from
 * Mission files.
 * 
 * 39    3/20/97 3:55p Hoffoss
 * Major changes to how dialog boxes initialize (load) and update (save)
 * their internal data.  This should simplify things and create less
 * problems.
 * 
 * 38    3/04/97 6:27p Hoffoss
 * Changes to Fred to handle new wing structure.
 * 
 * 37    2/28/97 11:31a Hoffoss
 * Implemented modeless dialog saving and restoring, and changed some
 * variables names.
 * 
 * 36    2/27/97 3:09p Allender
 * major wing structure enhancement.  simplified wing code.  All around
 * better wing support
 * 
 * 35    2/20/97 4:03p Hoffoss
 * Several ToDo items: new reinforcement clears arrival cue, reinforcement
 * control from ship and wing dialogs, show grid toggle.
 * 
 * 34    2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 * 
 * 14    2/12/97 12:25p Hoffoss
 * Expanded on global error checker, added initial orders conflict
 * checking and warning, added waypoint editor dialog and code.
 * 
 * 13    1/30/97 2:24p Hoffoss
 * Added remaining mission file structures and implemented load/save of
 * them.
 *
 * $NoKeywords: $
 */

#include <direct.h>

#include "stdafx.h"
#include "FRED.h"
#include <stdlib.h>
//#include <atlbase.h>
//#include <atlconv.h>

#include "FREDDoc.h"
#include "FREDView.h"
#include "PrefsDlg.h"

#include "render/3d.h"
#include "object/object.h"
#include "editor.h"
#include "ai/ai.h"
#include "ai/ailocal.h"
#include "cfile/cfile.h"

#include "cfile/cfilesystem.h"
#include "ship/ship.h"
#include "mission/missionparse.h"
#include "mission/missiongoals.h"
#include "MissionSave.h"
#include "weapon/weapon.h"
#include "Management.h"
#include "globalincs/linklist.h"
#include "FredRender.h"
#include "MainFrm.h"
#include "EventEditor.h"
#include "ai/aigoals.h"
#include "MessageEditorDlg.h"
#include "palman/palman.h"
#include "localization/fhash.h"
#include "cmdline/cmdline.h"
#include "starfield/starfield.h"
#include "localization/localize.h"

extern int Num_objects;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//	In editor mode, use class CFile, in game, use CFILE (our file)
#define	XFILE CFile

// stupid
#ifndef __FOLDERDLG_H__
    #include "FolderDlg.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CFREDDoc

IMPLEMENT_DYNCREATE(CFREDDoc, CDocument)

BEGIN_MESSAGE_MAP(CFREDDoc, CDocument)
	//{{AFX_MSG_MAP(CFREDDoc)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_DUPLICATE, OnDuplicate)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_HOLD, OnEditHold)
	ON_COMMAND(ID_EDIT_FETCH, OnEditFetch)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_FILE_PREFERENCES, OnFilePreferences)
	ON_COMMAND(ID_FILE_IMPORT_FSM, OnFileImportFSM)
	ON_COMMAND(ID_FILE_IMPORT_WEAPONS, OnFileImportWeapons)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFREDDoc construction/destruction

//	Global pointer to the FREDDoc class.
//	Used by MK to, among other things, I hope, update the modified flag from
//	outside the FREDDoc class.
CFREDDoc *FREDDoc_ptr = NULL;
int Local_modified = 0;
int Undo_available = 0;
int Undo_count = 0;

extern int Num_unknown_ship_classes;
extern int Num_unknown_weapon_classes;

CFREDDoc::CFREDDoc()
{
	int i;

	FREDDoc_ptr = this;
	confirm_deleting = TRUE;
	show_capital_ships = TRUE;
	show_elevations = TRUE;
	show_fighters = TRUE;
	show_grid = TRUE;
	show_misc_objects = TRUE;
	show_planets = TRUE;
	show_waypoints = TRUE;
	show_starfield = TRUE;

	for (i=0; i<BACKUP_DEPTH; i++)
		undo_desc[i].Empty();
}

CFREDDoc::~CFREDDoc()
{
}

BOOL CFREDDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	confirm_deleting = TRUE;
	show_capital_ships = FALSE;
	show_elevations = TRUE;
	show_fighters = TRUE;
	show_grid = FALSE;
	show_misc_objects = TRUE;
	show_planets = FALSE;
	show_waypoints = TRUE;
	show_starfield = FALSE;
	// (SDI documents will reuse this document)

	return TRUE;
}

// read in a new mission file from disk
BOOL CFREDDoc::OnOpenDocument(LPCTSTR pathname)
{
	char name[1024];
	int i, len;

	if (pathname)
		strcpy(mission_pathname, pathname);

	if (Briefing_dialog)
		Briefing_dialog->icon_select(-1);  // clean things up first

	len = strlen(mission_pathname);
	strcpy(name, mission_pathname);
	if (name[len - 4] == '.')
		len -= 4;

	name[len] = 0;  // drop extension
	i = len;
	while (i--)
		if ((name[i] == '\\') || (name[i] == ':'))
			break;

	strcpy(Mission_filename, name + i + 1);
//	for (i=1; i<=BACKUP_DEPTH; i++) {
//		sprintf(name + len, ".%.3d", i);
//		unlink(name);
//	}

	if (load_mission(mission_pathname)) {
		*Mission_filename = 0;
		return FALSE;
	}

	Fred_view_wnd->global_error_check();
	autosave("nothing");
	Undo_count = 0;
	return TRUE;
}

// save mission to a file
BOOL CFREDDoc::OnSaveDocument(LPCTSTR pathname)
{
	CFred_mission_save save;
	char name[1024];
	int len;
	DWORD attrib;
	FILE *fp;

	len = strlen(pathname);
	strcpy(name, pathname);
	if (name[len - 4] == '.')
		len -= 4;

	name[len] = 0;  // drop extension
	while (len--)
		if ((name[len] == '\\') || (name[len] == ':'))
			break;

	strcpy(Mission_filename, name + len + 1);
	Fred_view_wnd->global_error_check();
	if (Briefing_dialog) {
		Briefing_dialog->update_data(1);
		Briefing_dialog->save_editor_state();
	}
	
	if (Event_editor_dlg)
		Fred_main_wnd->MessageBox("Event editor dialog is still open, so changes there won't be saved");

	if (Message_editor_dlg)
		Fred_main_wnd->MessageBox("Message editor dialog is still open, so changes there won't be saved");

	fp = fopen(pathname, "r");
	if (fp) {
		fclose(fp);
		attrib = GetFileAttributes(pathname);
		if (attrib & FILE_ATTRIBUTE_READONLY) {
			Fred_main_wnd->MessageBox("File is read-only.  You need to check it out before saving to it");
			return FALSE;
		}
	}	

	if (save.save_mission_file((char *) pathname)) {
		Fred_main_wnd->MessageBox("An error occured while saving!", NULL, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	SetModifiedFlag(FALSE);
	if (load_mission((char *) pathname))
		Error(LOCATION, "Failed attempting to reload mission after saving.  Report this bug now!");

	if (Briefing_dialog) {
		Briefing_dialog->restore_editor_state();
		Briefing_dialog->update_data(1);
	}

	return TRUE;
//	return CDocument::OnSaveDocument(pathname);
}

int CFREDDoc::check_undo()
{
	char name[256];
	FILE *fp;

	Undo_available = 0;
	if (!Undo_count)
		return 0;


	cf_create_default_path_string(name, sizeof(name) - 1, CF_TYPE_MISSIONS);
	SAFE_STRCAT(name, MISSION_BACKUP_NAME, (sizeof(name) - 1));
	SAFE_STRCAT(name, ".002", (sizeof(name) - 1));
	fp = fopen(name, "r");
	if (!fp)
		return 0;

	fclose(fp);
	Undo_available = 1;
	return 1;
}

int CFREDDoc::autosave(char *desc)
{
	int i;
	CFred_mission_save save;
	CWaitCursor wait;

	if (Autosave_disabled)
		return 0;

	if (Briefing_dialog)
		Briefing_dialog->update_data(1);
	
	if (save.autosave_mission_file(MISSION_BACKUP_NAME)) {
		Undo_count = Undo_available = 0;
		return -1;
	}

	for (i=BACKUP_DEPTH; i>1; i--)
		undo_desc[i] = undo_desc[i - 1];

	if (desc)
		undo_desc[1] = desc;
	else
		undo_desc[1].Empty();

	Undo_count++;
	check_undo();
	return 0;
}

int CFREDDoc::autoload()
{
	char name[256], backup_name[256];
	int i, r, len;
	FILE *fp;


	cf_create_default_path_string(name, sizeof(name) - 1, CF_TYPE_MISSIONS);
	SAFE_STRCAT(name, MISSION_BACKUP_NAME, (sizeof(name) - 1));
	strcpy(backup_name, name);
	SAFE_STRCAT(name, ".002", (sizeof(name) - 1));
	fp = fopen(name, "r");
	if (!fp)
		return 0;

	fclose(fp);
	if (Briefing_dialog)
		Briefing_dialog->icon_select(-1);  // clean things up first

//	editor_init_mission();  
	r = load_mission(name);
	Update_window = 1;

	len = strlen(backup_name);
	strcat(backup_name, ".001");
	cf_delete(backup_name, CF_TYPE_MISSIONS);

	for (i=1; i<BACKUP_DEPTH; i++) {
		sprintf(backup_name + len, ".%.3d", i + 1);
		sprintf(name + len, ".%.3d", i);
		cf_rename(backup_name, name, CF_TYPE_MISSIONS);
		undo_desc[i] = undo_desc[i + 1];
	}

	Undo_count--;
	check_undo();
	return r;
}

// read in a new mission file from disk
int CFREDDoc::load_mission(char *pathname, int flags)
{
	// make sure we're in the correct working directory!!!!!!
	chdir(Fred_base_dir);

	char name[512], *old_name;
	int i, j, k, ob;
	int used_pool[MAX_WEAPON_TYPES];
	waypoint_list *wptr;
	object *objp;

	Parse_viewer_pos = view_pos;
	Parse_viewer_orient = view_orient;

	// activate the localizer hash table
	fhash_flush();	

	clear_mission();

	if (parse_main(pathname, flags))
	{
		if (flags & MPF_IMPORT_FSM)
		{
			sprintf(name, "Unable to import the file \"%s\".", pathname);
			Fred_view_wnd->MessageBox(name);
		}
		else
		{
			sprintf(name, "Unable to load the file \"%s\".", pathname);
			Fred_view_wnd->MessageBox(name);
		}
		create_new_mission();		
		return -1;
	}

	if ((Num_unknown_ship_classes > 0) || (Num_unknown_weapon_classes > 0))
	{
		if (flags & MPF_IMPORT_FSM)
		{
			char msg[256];
			sprintf(msg, "Fred encountered unknown ship/weapon classes when importing \"%s\" (path \"%s\"). You will have to manually edit the converted mission to correct this.", The_mission.name, pathname);
			Fred_view_wnd->MessageBox(msg);
		}
		else
		{
			Fred_view_wnd->MessageBox("Fred encountered unknown ship/weapon classes when parsing the mission file. This may be due to mission disk data you do not have.");
		}
	}

	for (i=0; i<Num_waypoint_lists; i++) {
		wptr = &Waypoint_lists[i];
		for (j=0; j<wptr->count; j++){
			ob = obj_create(OBJ_WAYPOINT, -1, i * 65536 + j, NULL, &wptr->waypoints[j], 0.0f, OF_RENDERS);
		}
	}

	obj_merge_created_list();
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if (objp->flags & OF_PLAYER_SHIP) {
			Assert(objp->type == OBJ_SHIP);
			objp->type = OBJ_START;
//			Player_starts++;
		}

		objp = GET_NEXT(objp);
	}

	for (i=0; i<Num_wings; i++) {
		for (j=0; j<Wings[i].wave_count; j++) {
			ob = Ships[Wings[i].ship_index[j]].objnum;
			wing_objects[i][j] = ob;
			Ships[Wings[i].ship_index[j]].wingnum = i;
			Ships[Wings[i].ship_index[j]].arrival_cue = Locked_sexp_false;
		}

		// fix old ship names for ships in wings if needed
		while (j--) {
			if ( (Objects[wing_objects[i][j]].type == OBJ_SHIP) || (Objects[wing_objects[i][j]].type == OBJ_START) ) {  // don't change player ship names
				sprintf(name, "%s %d", Wings[i].name, j + 1);
				old_name = Ships[Wings[i].ship_index[j]].ship_name;
				if (stricmp(name, old_name)) {  // need to fix name
					update_sexp_references(old_name, name);
					ai_update_goal_references(REF_TYPE_SHIP, old_name, name);
					for (k=0; k<Num_reinforcements; k++)
						if (!stricmp(old_name, Reinforcements[k].name)) {
							Assert(strlen(name) < NAME_LENGTH);
							strcpy(Reinforcements[k].name, name);
						}

					strcpy(Ships[Wings[i].ship_index[j]].ship_name, name);
				}
			}
		}
	}

	generate_weaponry_usage_list(used_pool);
	for ( j = 0; j < Num_teams; j++ ) {
		for (i=0; i<Num_weapon_types; i++) {
			Team_data[j].weaponry_pool[i] -= used_pool[i];  // convert weaponry_pool to be extras available beyond the current ships weapons
			if (Team_data[j].weaponry_pool[i] < 0)
				Team_data[j].weaponry_pool[i] = 0;
		}
	}

	Assert(Mission_palette >= 0);
	Assert(Mission_palette <= 98);

	// RT, dont need this anymore
#if 0

	if (The_mission.flags & MISSION_FLAG_SUBSPACE) {
		strcpy(name, NOX("gamepalette-subspace"));
	} else {
		strcpy(name, "gamepalette1-01");
		// sprintf(name, NOX("gamepalette1-%02d"), Mission_palette + 1);
	}

  	palette_load_table(name);
#endif

	// go through all ships and translate their callsign and alternate name indices	
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		// if this is a ship, check it, and mark its possible alternate name down in the auxiliary array
		if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->instance >= 0)) {
			if (Ships[objp->instance].alt_type_index >= 0) {
				mission_parse_lookup_alt_index(Ships[objp->instance].alt_type_index, Fred_alt_names[objp->instance]);

				// also zero it
				Ships[objp->instance].alt_type_index = -1;
			}
			
			if (Ships[objp->instance].callsign_index >= 0) {
				mission_parse_lookup_callsign_index(Ships[objp->instance].callsign_index, Fred_callsigns[objp->instance]);

				// also zero it
				Ships[objp->instance].callsign_index = -1;
			}
		}

		objp = GET_NEXT(objp);
	}


	view_pos = Parse_viewer_pos;
	view_orient = Parse_viewer_orient;
	set_modified(0);
	stars_post_level_init();

	return 0;
}

//	Editor-level interface to mission load/save.

// Does nothing now.. Handled by OnOpenDocument and OnSaveDocument.  This is because we
// want to avoid using the CArchive for file I/O  -JH
void CFREDDoc::Serialize(CArchive& ar)
{
	return;
/*  The original Serialize code
	int		rw_flag;
	XFILE		*fp;
//	CString	CSfilename;
//	char		filename[128], *tfilename;

	fp = ar.GetFile();
	rw_flag = ar.IsStoring();

//	CSfilename = fp->GetFileName();
//	tfilename = CSfilename.GetBuffer(16);
//	strcpy(filename, tfilename);
//	CSfilename.ReleaseBuffer();
// -- Don't close this, it gets closed by MFC --	ar.Close();

	cfile_serialize(fp, rw_flag);
	cfile_serialize_editor(fp, rw_flag);
*/
}

/////////////////////////////////////////////////////////////////////////////
// CFREDDoc diagnostics

#ifdef _DEBUG
void CFREDDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFREDDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CFREDDoc commands

void CFREDDoc::OnEditDelete() 
{
	// TODO: Add your command handler code here
	
}

void CFREDDoc::OnDuplicate() 
{
	// TODO: Add your command handler code here
	
}

void CFREDDoc::OnEditCopy() 
{
	// TODO: Add your command handler code here
	
}

void CFREDDoc::OnEditCut() 
{
	// TODO: Add your command handler code here
	
}

void CFREDDoc::OnEditPaste() 
{
	// TODO: Add your command handler code here
	
}

void CFREDDoc::OnEditHold() 
{
	// TODO: Add your command handler code here
	
}

void CFREDDoc::OnEditFetch() 
{
	// TODO: Add your command handler code here
	
}

void CFREDDoc::OnEditUndo() 
{
	// TODO: Add your command handler code here
	
}

void CFREDDoc::OnFilePreferences() 
{
	CPrefsDlg dlg;
	// Initialize dialog data
	dlg.m_ConfirmDeleting = confirm_deleting;
	dlg.m_ShowCapitalShips = show_capital_ships;
	dlg.m_ShowElevations = show_elevations;
	dlg.m_ShowFighters = show_fighters;
	dlg.m_ShowGrid = show_grid;
	dlg.m_ShowMiscObjects = show_misc_objects;
	dlg.m_ShowPlanets = show_planets;
	dlg.m_ShowWaypoints = show_waypoints;
	dlg.m_ShowStarfield = show_starfield;

	// Invoke the dialog box
	if (dlg.DoModal() == IDOK)
	{
		// retrieve the dialog data
		confirm_deleting = dlg.m_ConfirmDeleting;
		show_capital_ships = dlg.m_ShowCapitalShips;
		show_elevations = dlg.m_ShowElevations;
		show_fighters = dlg.m_ShowFighters;
		show_grid = dlg.m_ShowGrid;
		show_misc_objects = dlg.m_ShowMiscObjects;
		show_planets = dlg.m_ShowPlanets;
		show_waypoints = dlg.m_ShowWaypoints;
		show_starfield = dlg.m_ShowStarfield;
	}

}

// initialize (clear out) the mission, so it's empty and ready to use.
void CFREDDoc::editor_init_mission()
{
	reset_mission();
	SetModifiedFlag(FALSE);
}
/*
void CFREDDoc::OnFileNew() 
{
	// If mission has been modified, offer to save before continuing.
	while (IsModified()) {
		int	rval;
		
		rval = MessageBox(NULL,
			"You have not saved your work.\n(Which isn't surprising...)\nSave it now?",
			"Creating New Mission",
			MB_YESNOCANCEL + MB_ICONEXCLAMATION);

		if (rval == IDYES) {
			OnFileSave();
		} else if (rval == IDCANCEL)
			return;
		else if (rval == IDNO)
			break;
	}

	editor_init_mission();

}
*/
void CFREDDoc::UpdateStatus(int flags)
{
	if (FREDDoc_ptr)
		if (flags & US_WORLD_CHANGED)
			FREDDoc_ptr->SetModifiedFlag();
}

void CFREDDoc::OnEditClearAll()
{
   DeleteContents();
}
  
  
void CFREDDoc::DeleteContents()
{
	editor_init_mission();
}

void set_modified(BOOL arg)
{
	Local_modified = arg;
	FREDDoc_ptr->SetModifiedFlag(arg);
}

 //////////////////////////////////////////////////////////////////////////
//
// Below is old, obsolete code, kept around just in case it might be found
// useful some time in the future for something.
//
 //////////////////////////////////////////////////////////////////////////

/*
#define SerializeFloat(fp, mode, f) if (mode == 1) fp->Write(&f, sizeof(float)); else fp->Read(&f, sizeof(float))
#define SerializeInt(fp, mode, f) if (mode == 1) fp->Write(&f, sizeof(int)); else fp->Read(&f, sizeof(int))

void SerializeVector(XFILE *fp, int mode, vector *v)
{
	SerializeFloat(fp, mode, v->x);
	SerializeFloat(fp, mode, v->y);
	SerializeFloat(fp, mode, v->z);
}

void SerializeMatrix(XFILE *fp, int mode, matrix *m)
{
	SerializeVector(fp, mode, &m->rvec);
	SerializeVector(fp, mode, &m->uvec);
	SerializeVector(fp, mode, &m->fvec);
}

void SerializePhysicsInfo(XFILE *fp, int mode, physics_info *pi)
{
	SerializeFloat(fp, mode, pi->mass);
	SerializeFloat(fp, mode, pi->drag);
	SerializeVector(fp, mode, &pi->max_thrust);
	SerializeVector(fp, mode, &pi->max_rotthrust);
	SerializeFloat(fp, mode, pi->turnroll);
	SerializeInt(fp, mode, pi->flags);
	SerializeVector(fp, mode, &pi->velocity);
	SerializeVector(fp, mode, &pi->rotvel);
	SerializeVector(fp, mode, &pi->thrust);
	SerializeVector(fp, mode, &pi->rotthrust);
}

/////////////////////////////////////////////////////////////////////////////
// CFREDDoc serialization
void SerializeObject(XFILE *fp, int mode, object *objp)
{
	SerializeInt(fp, mode, objp->signature);
	SerializeInt(fp, mode, objp->type);
	SerializeInt(fp, mode, objp->parent);
	SerializeInt(fp, mode, objp->parent_sig);
	SerializeInt(fp, mode, objp->parent_type);
	SerializeInt(fp, mode, objp->instance);
	SerializeInt(fp, mode, objp->flags);
	SerializeInt(fp, mode, objp->flags2);	// Goober5000 - code is obsolete, but I added this just in case
	SerializeFloat(fp, mode, objp->radius);
//	SerializeInt(fp, mode, objp->wing);
	SerializePhysicsInfo(fp, mode, &objp->phys_info);
	SerializeVector(fp, mode, &objp->pos);
	SerializeMatrix(fp, mode, &objp->orient);
}

void SerializeAI(XFILE *fp, int mode, ai_info *aip)
{
	SerializeInt(fp, mode, aip->shipnum);
	SerializeInt(fp, mode, aip->type);
	SerializeInt(fp, mode, aip->wing);
//MWA --	SerializeInt(fp, mode, aip->current_waypoint);
}

void SerializeShip(XFILE *fp, int mode, ship *shipp)
{
	SerializeInt(fp, mode, shipp->objnum);
	SerializeInt(fp, mode, shipp->ai_index);
	SerializeInt(fp, mode, shipp->subtype);
	SerializeInt(fp, mode, shipp->modelnum);
	SerializeInt(fp, mode, shipp->hits);
	SerializeInt(fp, mode, shipp->dying);
}

void SerializeGrid(XFILE *fp, int mode, grid *gridp)
{
	int	i;

	SerializeInt(fp, mode, gridp->nrows);
	SerializeInt(fp, mode, gridp->ncols);
	SerializeMatrix(fp, mode, &gridp->gmatrix);
	SerializePhysicsInfo(fp, mode, &gridp->physics);
	SerializeFloat(fp, mode, gridp->square_size);
	SerializeFloat(fp, mode, gridp->planeD);
	
	for (i=0; i<MAX_GRID_POINTS; i++)
		SerializeVector(fp, mode, &gridp->gpoints[i]);

}

void cfile_serialize(XFILE *fp, int flag)
{
	int	i;
	int	highest_object_index = 0, highest_ship_index = 0, highest_ai_index = 0;

	Assert((flag == 0) || (flag == 1));

//	fp = cfopen(filename, flag ? "wb" : "rb");
//	if (!fp)
//		MessageBox(NULL, strerror(errno), "File Open Error!", MB_ICONSTOP);

	//	Find highest used object if writing.
	if (flag == 1) {
		for (i=MAX_OBJECTS-1; i>0; i--)
			if (Objects[i].type != OBJ_NONE) {
				highest_object_index = i;
				break;
			}
	}

	if (flag == 0) {
		num_ships = 0;
		Num_objects = 0;
	}

	SerializeInt(fp, flag, highest_object_index);

	for (i=1; i<=highest_object_index; i++) {
		SerializeObject(fp, flag, &Objects[i]);
		if (flag == 0)
			if (Objects[i].type != OBJ_NONE)
				Num_objects++;
	}

	//	Read/write ships
	if (flag == 1) {
		for (i=MAX_SHIPS-1; i>0; i--)
			if (Ships[i].objnum) {
				highest_ship_index = i;
				break;
			}
	}

	SerializeInt(fp, flag, highest_ship_index);

	for (i=1; i<=highest_ship_index; i++) {
		SerializeShip(fp, flag, &Ships[i]);
		if (flag == 0)
			if (Ships[i].objnum)
				num_ships++;
	}

	// Read/write AI info
	if (flag == 1) {
		for (i=MAX_AI_INFO-1; i>0; i--)
			if (Ai_info[i].shipnum) {
				highest_ai_index = i;
				break;
			}
	}

	SerializeInt(fp, flag, highest_ai_index);
	
	for (i=1; i<=highest_ai_index; i++)
		SerializeAI(fp, flag, &Ai_info[i]);
}

void cfile_serialize_editor(XFILE *fp, int flag)
{
	//	Editor only stuff
	SerializeMatrix(fp, flag, &view_orient);
	SerializeVector(fp, flag, &view_pos);
	
	SerializeInt(fp, flag, Control_mode);
	SerializeInt(fp, flag, cur_object_index);
	SerializeInt(fp, flag, cur_wing);

	SerializeGrid(fp, flag, The_grid);

}
*/

// Goober5000
void CFREDDoc::OnFileImportFSM() 
{
	char fs1_mission_path[MAX_PATH_LEN];
	char fs2_mission_path[MAX_PATH_LEN];
	char dest_directory[MAX_PATH+1];

	// path stuff
	{
		char *ch;

		// get base paths
		strcpy(fs1_mission_path, Fred_exe_dir);
		ch = strrchr(fs1_mission_path, DIR_SEPARATOR_CHAR);
		if (ch != NULL)
			*ch = '\0';
		strcpy(fs2_mission_path, Fred_exe_dir);
		ch = strrchr(fs2_mission_path, DIR_SEPARATOR_CHAR);
		if (ch != NULL)
			*ch = '\0';

		// estimate the mission path for FS1
		if ((ch = stristr(fs1_mission_path, "FreeSpace2")) != NULL)
		{
			strcpy(ch, "FreeSpace\\Data\\Missions");
		}

		// estimate the mission path for FS2
		strcat(fs2_mission_path, "\\Data\\Missions");
	}

	// if mission has been modified, offer to save before continuing.
	if (!SaveModified())
		return;


	// get location to import from
	CFileDialog dlgFile(TRUE, "fsm", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT, "FreeSpace Missions (*.fsm)|*.fsm|All files (*.*)|*.*||");
	dlgFile.m_ofn.lpstrTitle = "Select one or more missions to import";
	dlgFile.m_ofn.lpstrInitialDir = fs1_mission_path;

	// get FSM files
	if (dlgFile.DoModal() != IDOK)
		return;

	memset( dest_directory, 0, sizeof(dest_directory) );

	// get location to save to    
#if ( _MFC_VER >= 0x0700 )
	//ITEMIDLIST fs2_mission_pidl = {0};

	//SHParseDisplayName(A2CW(fs2_mission_path), NULL, fs2_mission_pidl, 0, 0);

	BROWSEINFO bi;
	bi.hwndOwner = theApp.GetMainWnd()->GetSafeHwnd();
	//bi.pidlRoot = &fs2_mission_pidl;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = dest_directory;
	bi.lpszTitle = "Select a location to save in";
	bi.ulFlags = 0;
	bi.lpfn = NULL;
	bi.lParam = NULL;
	bi.iImage = NULL;

	LPCITEMIDLIST ret_val = SHBrowseForFolder(&bi);

	if(ret_val == NULL)
		return;

	SHGetPathFromIDList(ret_val, dest_directory);
#else
    CFolderDialog dlgFolder(_T("Select a location to save in"), fs2_mission_path, NULL);
    if(dlgFolder.DoModal() != IDOK)
        return;

	strcpy( dest_directory, dlgFolder.GetFolderPath() );
#endif

	// clean things up first
	if (Briefing_dialog)
		Briefing_dialog->icon_select(-1);

	clear_mission();

	// process all missions
	POSITION pos(dlgFile.GetStartPosition());
	while(pos)
	{
		char *ch;
		char filename[1024];
		char fs1_path[MAX_PATH_LEN];
		char dest_path[MAX_PATH_LEN];

		CString fs1_path_mfc(dlgFile.GetNextPathName(pos));
		CFred_mission_save save;

		DWORD attrib;
		FILE *fp;


		// path name too long?
		if (strlen(fs1_path_mfc) > MAX_PATH_LEN - 1)
			continue;

		// nothing here?
		if (!strlen(fs1_path_mfc))
			continue;

		// get our mission
		strcpy(fs1_path, fs1_path_mfc);

		// load mission into memory
		if (load_mission(fs1_path, MPF_IMPORT_FSM))
			continue;

		// get filename
		ch = strrchr(fs1_path, DIR_SEPARATOR_CHAR) + 1;
		if (ch != NULL)
			strcpy(filename, ch);
		else
			strcpy(filename, fs1_path);

		// truncate extension
		ch = strrchr(filename, '.');
		if (ch != NULL)
			*ch = '\0';

		// add new extension
		strcat(filename, ".fs2");

		strcpy(Mission_filename, filename);

		// get new path
		strcpy(dest_path, dest_directory);
		strcat(dest_path, "\\");
		strcat(dest_path, filename);

		// check attributes
		fp = fopen(dest_path, "r");
		if (fp)
		{
			fclose(fp);
			attrib = GetFileAttributes(dest_path);
			if (attrib & FILE_ATTRIBUTE_READONLY)
				continue;
		}	

		// try to save it
		if (save.save_mission_file(dest_path))
			continue;

		// success
	}

	create_new_mission();

	MessageBox(NULL, "Import complete.  Please check the destination folder to verify all missions were imported successfully.", "Status", MB_OK);
}

void restore_default_weapons(char *ships_tbl);

void CFREDDoc::OnFileImportWeapons() 
{
	int rval;
	CString ships_tbl_mfc;
	char ships_tbl[MAX_PATH_LEN];
	char *ships_tbl_text;
	char *ships_tbl_text_raw;
	char *ch;
	char temp[MAX_PATH_LEN];
	char error_str[1024];

	// set up import dialog
	CFileDialog dlg(TRUE, "tbl", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, "FreeSpace Tables (*.tbl)|*.tbl|All files (*.*)|*.*||");
	dlg.m_ofn.lpstrTitle = "Specify the ships.tbl with the required loadouts";

	// set initial path
	strcpy(temp, Fred_exe_dir);
	if ((ch = stristr(temp, "FreeSpace2")) != NULL)
	{
		strcpy(ch, "FreeSpace\\Data\\Tables");
		dlg.m_ofn.lpstrInitialDir = temp;
	}

	// get ships.tbl
	if (dlg.DoModal() != IDOK)
		return;

	ships_tbl_mfc = dlg.GetPathName();

	if (strlen(ships_tbl_mfc) > MAX_PATH_LEN - 1)
	{
		MessageBox(NULL, "Path name is too long", "Error", MB_OK);
		return;
	}

	if (!strlen(ships_tbl_mfc))
		return;

	strcpy(ships_tbl, ships_tbl_mfc);

	// allocate junk
	ships_tbl_text = (char *) malloc(sizeof(char) * MISSION_TEXT_SIZE);
	ships_tbl_text_raw = (char *) malloc(sizeof(char) * MISSION_TEXT_SIZE);
	if (!ships_tbl_text || !ships_tbl_text_raw)
	{
		free(ships_tbl_text);
		free(ships_tbl_text_raw);

		Error(LOCATION, "Not enough memory to import weapons.");
		return;
	}

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("FREDDOC: Unable to parse '%s'!  Error code = %i.\n", ships_tbl, rval));
		sprintf(error_str, "Could not parse file: %s", ships_tbl);

		MessageBox(NULL, error_str, "Unable to import weapon loadouts!", MB_ICONERROR | MB_OK);
		lcl_ext_close();
		return;
	}

	// load the other table and convert, freeing memory after use
	read_file_text(ships_tbl, CF_TYPE_ANY, ships_tbl_text, ships_tbl_text_raw);
	free(ships_tbl_text_raw);
	restore_default_weapons(ships_tbl_text);
	free(ships_tbl_text);

	// close localization
	lcl_ext_close();

	// we haven't saved it yet
	set_modified(TRUE);

	// error check and notify
	if (!Fred_view_wnd->global_error_check())
		Fred_view_wnd->MessageBox("Weapon loadouts successfully imported with no errors.", "Woohoo!");
}
