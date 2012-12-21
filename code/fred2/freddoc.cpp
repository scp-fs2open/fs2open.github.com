/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
extern int Num_unknown_loadout_classes;

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
		strcpy_s(mission_pathname, pathname);

	if (Briefing_dialog)
		Briefing_dialog->icon_select(-1);  // clean things up first

	len = strlen(mission_pathname);
	strcpy_s(name, mission_pathname);
	if (name[len - 4] == '.')
		len -= 4;

	name[len] = 0;  // drop extension
	i = len;
	while (i--)
		if ((name[i] == '\\') || (name[i] == ':'))
			break;

	strcpy_s(Mission_filename, name + i + 1);
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
	strcpy_s(name, pathname);
	if (name[len - 4] == '.')
		len -= 4;

	name[len] = 0;  // drop extension
	while (len--)
		if ((name[len] == '\\') || (name[len] == ':'))
			break;

	strcpy_s(Mission_filename, name + len + 1);
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
	strcat_s(name, MISSION_BACKUP_NAME);
	strcat_s(name, ".002");
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
	strcat_s(name, MISSION_BACKUP_NAME);
	strcpy_s(backup_name, name);
	strcat_s(name, ".002");
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
	strcat_s(backup_name, ".001");
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

	if ((Num_unknown_ship_classes > 0) || (Num_unknown_weapon_classes > 0) || (Num_unknown_loadout_classes > 0))
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
					update_texture_replacements(old_name, name);
					for (k=0; k<Num_reinforcements; k++)
						if (!strcmp(old_name, Reinforcements[k].name)) {
							Assert(strlen(name) < NAME_LENGTH);
							strcpy_s(Reinforcements[k].name, name);
						}

					strcpy_s(Ships[Wings[i].ship_index[j]].ship_name, name);
				}
			}
		}
	}

	for ( i = 0; i < Num_teams; i++ ) {
		generate_weaponry_usage_list(i, used_pool);
		for (j=0; j<Team_data[i].num_weapon_choices; j++) {
			// The amount used in wings is always set by a static loadout entry so skip any that were set by Sexp variables
			if ( (!strlen(Team_data[i].weaponry_pool_variable[j])) && (!strlen(Team_data[i].weaponry_amount_variable[j])) ) {
				// convert weaponry_pool to be extras available beyond the current ships weapons
				Team_data[i].weaponry_count[j] -= used_pool[Team_data[i].weaponry_pool[j]];
				if (Team_data[i].weaponry_count[j] < 0) {
					Team_data[i].weaponry_count[j] = 0;				
				}

				// zero the used pool entry
				used_pool[Team_data[i].weaponry_pool[j]] = 0;
			}
		}
		// double check the used pool is empty
		for (j=0; j<MAX_WEAPON_TYPES; j++) {
			if (used_pool[j] != 0) {
				Warning(LOCATION, "%s is used in wings of team %d but was not in the loadout. Fixing now", Weapon_info[j].name , i+1);
				
				// add the weapon as a new entry
				Team_data[i].weaponry_pool[Team_data[i].num_weapon_choices] = j;
				Team_data[i].weaponry_count[Team_data[i].num_weapon_choices] = used_pool[j]; 
				strcpy_s(Team_data[i].weaponry_amount_variable[Team_data[i].num_weapon_choices], "");
				strcpy_s(Team_data[i].weaponry_pool_variable[Team_data[i].num_weapon_choices++], "");		
			}
		}
	}

	Assert(Mission_palette >= 0);
	Assert(Mission_palette <= 98);

	// RT, don't need this anymore
#if 0

	if (The_mission.flags & MISSION_FLAG_SUBSPACE) {
		strcpy_s(name, NOX("gamepalette-subspace"));
	} else {
		strcpy_s(name, "gamepalette1-01");
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

	recreate_dialogs();

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
//	strcpy_s(filename, tfilename);
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
	recreate_dialogs();
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
		strcpy_s(fs1_mission_path, Fred_exe_dir);
		ch = strrchr(fs1_mission_path, DIR_SEPARATOR_CHAR);
		if (ch != NULL)
			*ch = '\0';
		strcpy_s(fs2_mission_path, Fred_exe_dir);
		ch = strrchr(fs2_mission_path, DIR_SEPARATOR_CHAR);
		if (ch != NULL)
			*ch = '\0';

		// estimate the mission path for FS1
		if ((ch = stristr(fs1_mission_path, "FreeSpace2")) != NULL)
		{
			strcpy(ch, "FreeSpace\\Data\\Missions");
		}

		// estimate the mission path for FS2
		strcat_s(fs2_mission_path, "\\Data\\Missions");
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

	strcpy_s( dest_directory, dlgFolder.GetFolderPath() );
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
		strcpy_s(fs1_path, fs1_path_mfc);

		// load mission into memory
		if (load_mission(fs1_path, MPF_IMPORT_FSM))
			continue;

		// get filename
		ch = strrchr(fs1_path, DIR_SEPARATOR_CHAR) + 1;
		if (ch != NULL)
			strcpy_s(filename, ch);
		else
			strcpy_s(filename, fs1_path);

		// truncate extension
		ch = strrchr(filename, '.');
		if (ch != NULL)
			*ch = '\0';

		// add new extension
		strcat_s(filename, ".fs2");

		strcpy_s(Mission_filename, filename);

		// get new path
		strcpy_s(dest_path, dest_directory);
		strcat_s(dest_path, "\\");
		strcat_s(dest_path, filename);

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
	recreate_dialogs();
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
	strcpy_s(temp, Fred_exe_dir);
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

	strcpy_s(ships_tbl, ships_tbl_mfc);

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

void CFREDDoc::recreate_dialogs()
{
	if(Briefing_dialog)
	{
		Briefing_dialog->OnCancel();
		Briefing_dialog = new briefing_editor_dlg;
		Briefing_dialog->create();
		Briefing_dialog->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0,
		SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		Briefing_dialog->ShowWindow(SW_RESTORE);
	}

	if(Bg_bitmap_dialog)
	{
		Bg_bitmap_dialog->OnClose();
		Bg_bitmap_dialog = new bg_bitmap_dlg;
		Bg_bitmap_dialog->create();
		Bg_bitmap_dialog->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0,
		SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		Bg_bitmap_dialog->ShowWindow(SW_RESTORE);
	}
}
