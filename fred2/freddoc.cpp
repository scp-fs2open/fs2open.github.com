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
#include <shlobj.h>

#include "FREDDoc.h"
#include "FREDView.h"
#include "EventEditor.h"
#include "FredRender.h"
#include "MainFrm.h"
#include "Management.h"
#include "MessageEditorDlg.h"

#include "ai/ai.h"
#include "ai/aigoals.h"
#include "cfile/cfile.h"
#include "cfile/cfilesystem.h"
#include "cmdline/cmdline.h"
#include "globalincs/linklist.h"
#include "localization/fhash.h"
#include "localization/localize.h"
#include "mission/missiongoals.h"
#include "mission/missionparse.h"
#include "object/object.h"
#include "render/3d.h"
#include "ship/ship.h"
#include "starfield/starfield.h"
#include "weapon/weapon.h"
#include "scripting/global_hooks.h"

extern int Num_objects;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// In editor mode, use class CFile, in game, use CFILE (our file)
#define XFILE CFile


IMPLEMENT_DYNCREATE(CFREDDoc, CDocument)

BEGIN_MESSAGE_MAP(CFREDDoc, CDocument)
	//{{AFX_MSG_MAP(CFREDDoc)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_FILE_IMPORT_FSM, OnFileImportFSM)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CFREDDoc *FREDDoc_ptr = NULL;
int Local_modified = 0;
int Undo_available = 0;
int Undo_count = 0;


CFREDDoc::CFREDDoc() {
	int i;

	FREDDoc_ptr = this;

	for (i = 0; i < MISSION_BACKUP_DEPTH; i++)
		undo_desc[i].Empty();
}

CFREDDoc::~CFREDDoc() {}

#ifdef _DEBUG
void CFREDDoc::AssertValid() const {
	CDocument::AssertValid();
}
#endif //_DEBUG

bool CFREDDoc::autoload() {
	char name[256], backup_name[256];
	int i;
	bool r;
	FILE *fp;

	if ( !cf_create_default_path_string(name, sizeof(name) - 1, CF_TYPE_MISSIONS) ) {
		return 0;
	}

	strcat_s(name, MISSION_BACKUP_NAME);
	strcpy_s(backup_name, name);
	strcat_s(name, ".002");

	// Check if Backup.002 exists
	fp = fopen(name, "r");
	if (!fp)
		return 0;
	fclose(fp);

	clean_up_selections();

	// Load Backup.002
	r = load_mission(name, MPF_FAST_RELOAD);
	Update_window = 1;

	// Delete Backup.001
	auto len = strlen(backup_name);
	strcat_s(backup_name, ".001");
	cf_delete(backup_name, CF_TYPE_MISSIONS);

	// Rename Backups. .002 becomes .001, .003 becomes .002, etc.
	for (i = 1; i < MISSION_BACKUP_DEPTH; i++) {
		sprintf(backup_name + len, ".%.3d", i + 1);
		sprintf(name + len, ".%.3d", i);
		cf_rename(backup_name, name, CF_TYPE_MISSIONS);
		undo_desc[i] = undo_desc[i + 1];
	}

	// Reduce the undo count and check if we can do another undo op.
	Undo_count--;
	check_undo();
	return r;
}

int CFREDDoc::autosave(char *desc) {
	int i;
	Fred_mission_save save;
	if (Mission_save_format == FSO_FORMAT_RETAIL) {
		save.set_save_format(MissionFormat::RETAIL);
	} else if (Mission_save_format == FSO_FORMAT_COMPATIBILITY_MODE) {
		save.set_save_format(MissionFormat::COMPATIBILITY_MODE);
	} else {
		save.set_save_format(MissionFormat::STANDARD);
	}
	save.set_always_save_display_names(Always_save_display_names);
	save.set_view_pos(view_pos);
	save.set_view_orient(view_orient);
	save.set_fred_alt_names(Fred_alt_names);
	save.set_fred_callsigns(Fred_callsigns);

	CWaitCursor wait;

	if (Autosave_disabled) {
		return 0;
	}

	if (Briefing_dialog) {
		Briefing_dialog->update_data(1);
	}

	if (save.autosave_mission_file(MISSION_BACKUP_NAME)) {
		Undo_count = Undo_available = 0;
		return -1;
	}

	for (i = MISSION_BACKUP_DEPTH; i > 1; i--) {
		undo_desc[i] = undo_desc[i - 1];
	}

	if (desc) {
		undo_desc[1] = desc;
	} else {
		undo_desc[1].Empty();
	}

	Undo_count++;
	check_undo();
	return 0;
}

int CFREDDoc::check_undo() {
	char name[256];
	FILE *fp;

	Undo_available = 0;
	if (!Undo_count) {
		return 0;
	}

	if ( !cf_create_default_path_string(name, sizeof(name) - 1, CF_TYPE_MISSIONS) ) {
		return 0;
	}

	strcat_s(name, MISSION_BACKUP_NAME);
	strcat_s(name, ".002");

	// Check if Backup.002 exists
	fp = fopen(name, "r");
	if (!fp)
		return 0;
	fclose(fp);

	Undo_available = 1;
	return 1;
}

void CFREDDoc::DeleteContents() {
	editor_init_mission();
}

#ifdef _DEBUG
void CFREDDoc::Dump(CDumpContext& dc) const {
	CDocument::Dump(dc);
}
#endif // _DEBUG

void CFREDDoc::editor_init_mission() {
	reset_mission();
	SetModifiedFlag(FALSE);
	recreate_dialogs();
}

bool CFREDDoc::load_mission(const char *pathname, int flags) {
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

	clear_mission(flags & MPF_FAST_RELOAD);

	// message 1: required version
	if (!parse_main(pathname, flags)) {
		auto term = (flags & MPF_IMPORT_FSM) ? "import" : "load";

		// the version will have been assigned before loading was aborted
		if (!gameversion::check_at_least(The_mission.required_fso_version)) {
			sprintf(name, "The file \"%s\" cannot be %sed because it requires FSO version %s", pathname, term, format_version(The_mission.required_fso_version, true).c_str());
		} else {
			sprintf(name, "Unable to %s the file \"%s\".", term, pathname);
		}

		Fred_view_wnd->MessageBox(name);
		create_new_mission();
		return false;
	}

	// message 2: unknown classes
	if ((Num_unknown_ship_classes > 0) || (Num_unknown_prop_classes > 0) || (Num_unknown_weapon_classes > 0) || (Num_unknown_loadout_classes > 0)) {
		if (flags & MPF_IMPORT_FSM) {
			char msg[256];
			sprintf(msg, "Fred encountered unknown ship/prop/weapon classes when importing \"%s\" (path \"%s\"). You will have to manually edit the converted mission to correct this.", The_mission.name, pathname);
			Fred_view_wnd->MessageBox(msg);
		} else {
			Fred_view_wnd->MessageBox("Fred encountered unknown ship/prop/weapon classes when parsing the mission file. This may be due to mission disk data you do not have.");
		}
	}

	// message 3: warning about saving under a new version
	if (!(flags & MPF_IMPORT_FSM) && (The_mission.required_fso_version != LEGACY_MISSION_VERSION) && (MISSION_VERSION > The_mission.required_fso_version)) {
		SCP_string msg = "This mission's file format is ";
		msg += format_version(The_mission.required_fso_version, true);
		msg += ".  When you save this mission, the file format will be migrated to ";
		msg += format_version(MISSION_VERSION, true);
		msg += ".  FSO versions earlier than this will not be able to load the mission.";
		Fred_view_wnd->MessageBox(msg.c_str());
	}

	// message 4: check for "immobile" flag migration
	if (!Fred_migrated_immobile_ships.empty()) {
		SCP_string msg = "The \"immobile\" ship flag has been superseded by the \"don't-change-position\", and \"don't-change-orientation\" flags.  "
			"All ships which previously had \"Does Not Move\" checked in the ship flags editor will now have both \"Does Not Change Position\" and "
			"\"Does Not Change Orientation\" checked.  After you close this dialog, the error checker will check for any potential issues, including "
			"issues involving these flags.\n\nThe following ships have been migrated:";

		for (int shipnum : Fred_migrated_immobile_ships) {
			msg += "\n\t";
			msg += Ships[shipnum].ship_name;
		}

		truncate_message_lines(msg, 30);
		Fred_view_wnd->MessageBox(msg.c_str());
		Error_checker_checks_potential_issues_once = true;
	}

	obj_merge_created_list();
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if (objp->flags[Object::Object_Flags::Player_ship]) {
			Assert(objp->type == OBJ_SHIP);
			objp->type = OBJ_START;
			//			Player_starts++;
		}

		objp = GET_NEXT(objp);
	}

	for (i = 0; i < Num_wings; i++) {
		for (j = 0; j < Wings[i].wave_count; j++) {
			ob = Ships[Wings[i].ship_index[j]].objnum;
			wing_objects[i][j] = ob;
			Ships[Wings[i].ship_index[j]].wingnum = i;
			Ships[Wings[i].ship_index[j]].arrival_cue = Locked_sexp_false;
		}

		// fix old ship names for ships in wings if needed
		while (j--) {
			if ((Objects[wing_objects[i][j]].type == OBJ_SHIP) || (Objects[wing_objects[i][j]].type == OBJ_START)) {  // don't change player ship names
				wing_bash_ship_name(name, Wings[i].name, j + 1);
				old_name = Ships[Wings[i].ship_index[j]].ship_name;
				if (stricmp(name, old_name)) {  // need to fix name
					update_sexp_references(old_name, name);
					ai_update_goal_references(sexp_ref_type::SHIP, old_name, name);
					update_texture_replacements(old_name, name);
					for (k = 0; k < Num_reinforcements; k++)
						if (!strcmp(old_name, Reinforcements[k].name)) {
							Assert(strlen(name) < NAME_LENGTH);
							strcpy_s(Reinforcements[k].name, name);
						}

					strcpy_s(Ships[Wings[i].ship_index[j]].ship_name, name);
				}
			}
		}
	}

	for (i = 0; i < Num_teams; i++) {
		generate_weaponry_usage_list(i, used_pool);
		for (j = 0; j < Team_data[i].num_weapon_choices; j++) {
			// The amount used in wings is always set by a static loadout entry so skip any that were set by Sexp variables
			if ((!strlen(Team_data[i].weaponry_pool_variable[j])) && (!strlen(Team_data[i].weaponry_amount_variable[j]))) {
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
		for (j = 0; j < weapon_info_size(); j++) {
			if (!Team_data[i].do_not_validate && used_pool[j] != 0) {
				Warning(LOCATION, "%s is used in wings of team %d but was not in the loadout. Fixing now", Weapon_info[j].name, i + 1);

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
	
	// go through all ships and translate their callsign and alternate name indices	
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		// if this is a ship, check it, and mark its possible alternate name down in the auxiliary array
		if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->instance >= 0)) {
			if (Ships[objp->instance].alt_type_index >= 0) {
				strcpy_s(Fred_alt_names[objp->instance], mission_parse_lookup_alt_index(Ships[objp->instance].alt_type_index));

				// also zero it
				Ships[objp->instance].alt_type_index = -1;
			}

			if (Ships[objp->instance].callsign_index >= 0) {
				strcpy_s(Fred_callsigns[objp->instance], mission_parse_lookup_callsign_index(Ships[objp->instance].callsign_index));

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

	// This hook will allow for scripts to know when a mission has been loaded
	// which will then allow them to update any LuaEnums that may be related to sexps
	if (scripting::hooks::FredOnMissionLoad->isActive()) {
		scripting::hooks::FredOnMissionLoad->run();
	}

	return true;
}

void CFREDDoc::OnEditClearAll() {
	DeleteContents();
}

void CFREDDoc::OnEditUndo() {
	// TODO: Add your command handler code here

}

void CFREDDoc::OnFileImportFSM() {
	char fs1_mission_path[MAX_PATH_LEN];
	char fs2_mission_path[MAX_PATH_LEN];
	char dest_directory[MAX_PATH + 1];

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
		if ((ch = stristr(fs1_mission_path, "FreeSpace2")) != NULL) {
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

	memset(dest_directory, 0, sizeof(dest_directory));

	// get location to save to
	BROWSEINFO bi;
	bi.hwndOwner = theApp.GetMainWnd()->GetSafeHwnd();
	bi.pidlRoot = NULL;
	bi.pszDisplayName = dest_directory;
	bi.lpszTitle = "Select a location to save in";
	bi.ulFlags = 0;
	bi.lpfn = NULL;
	bi.lParam = NULL;
	bi.iImage = NULL;

	LPCITEMIDLIST ret_val = SHBrowseForFolder(&bi);

	if (ret_val == NULL)
		return;

	SHGetPathFromIDList(ret_val, dest_directory);

	if (*dest_directory == '\0')
		return;

	clear_mission(true);

	int num_files = 0;
	int successes = 0;
	char dest_path[MAX_PATH_LEN] = "";

	// process all missions
	POSITION pos(dlgFile.GetStartPosition());
	while (pos) {
		char *ch;
		char filename[1024];
		char fs1_path[MAX_PATH_LEN];

		CString fs1_path_mfc(dlgFile.GetNextPathName(pos));
		num_files++;
		Fred_mission_save save;
		if (Mission_save_format == FSO_FORMAT_RETAIL) {
			save.set_save_format(MissionFormat::RETAIL);
		} else if (Mission_save_format == FSO_FORMAT_COMPATIBILITY_MODE) {
			save.set_save_format(MissionFormat::COMPATIBILITY_MODE);
		} else {
			save.set_save_format(MissionFormat::STANDARD);
		}
		save.set_always_save_display_names(Always_save_display_names);
		save.set_view_pos(view_pos);
		save.set_view_orient(view_orient);
		save.set_fred_alt_names(Fred_alt_names);
		save.set_fred_callsigns(Fred_callsigns);

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
		if (!load_mission(fs1_path, MPF_IMPORT_FSM | MPF_FAST_RELOAD))
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
		if (fp) {
			fclose(fp);
			attrib = GetFileAttributes(dest_path);
			if (attrib & FILE_ATTRIBUTE_READONLY)
				continue;
		}

		// try to save it
		if (save.save_mission_file(dest_path))
			continue;

		// success
		successes++;
	}

	if (num_files > 1)
	{
		create_new_mission();
		Fred_view_wnd->MessageBox("Import complete.  Please check the destination folder to verify all missions were imported successfully.", "Status", MB_OK);
	}
	else if (num_files == 1)
	{
		if (successes == 1)
			SetModifiedFlag(FALSE);

		if (Briefing_dialog) {
			Briefing_dialog->restore_editor_state();
			Briefing_dialog->update_data(1);
		}

		if (successes == 1)
		{
			// these aren't done automatically for imports
			theApp.AddToRecentFileList((LPCTSTR)dest_path);
			SetTitle((LPCTSTR)Mission_filename);
		}
	}

	recreate_dialogs();
}

BOOL CFREDDoc::OnNewDocument() {
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

BOOL CFREDDoc::OnOpenDocument(LPCTSTR pathname)
{
	// don't process any objects if the window focus is lost and reacquired
	clean_up_selections();

	auto sep_ch = strrchr(pathname, '\\');
	auto filename = (sep_ch != nullptr) ? (sep_ch + 1) : pathname;
	auto len = strlen(filename);

	// drop extension and copy to Mission_filename
	auto ext_ch = strrchr(filename, '.');
	if (ext_ch != nullptr)
		len = ext_ch - filename;
	if (len >= 80)
		len = 79;
	strncpy(Mission_filename, filename, len);
	Mission_filename[len] = 0;

	// first, just grab the info of this mission
	if (!parse_main(pathname, MPF_ONLY_MISSION_INFO))
	{
		*Mission_filename = 0;
		return FALSE;
	}
	SCP_string created = The_mission.created;
	CFileLocation res = cf_find_file_location(pathname, CF_TYPE_ANY);
	time_t modified = res.m_time;
	if (!res.found)
	{
		UNREACHABLE("Couldn't find path '%s' even though parse_main() succeeded!", pathname);
		created = "";	// prevent any backup check from succeeding so we just load the actual specified file
	}

	// now check all the autosaves
	SCP_string backup_name;
	CFileLocation backup_res;
	for (int i = 1; i <= MISSION_BACKUP_DEPTH; ++i)
	{
		backup_name = MISSION_BACKUP_NAME;
		char extension[5];
		sprintf(extension, ".%.3d", i);
		backup_name += extension;

		backup_res = cf_find_file_location(backup_name.c_str(), CF_TYPE_MISSIONS);
		if (backup_res.found && parse_main(backup_res.full_name.c_str(), MPF_ONLY_MISSION_INFO))
		{
			SCP_string this_created = The_mission.created;
			time_t this_modified = backup_res.m_time;

			if (created == this_created && this_modified > modified)
				break;
		}

		backup_name.clear();
	}

	// maybe load from the backup instead
	if (!backup_name.empty())
	{
		SCP_string prompt = "Autosaved file ";
		prompt += backup_name;
		prompt += " has a file modification time more recent than the specified file.  Do you want to load the autosave instead?";
		int z = Fred_view_wnd->MessageBox(prompt.c_str(), "Recover from autosave", MB_ICONQUESTION | MB_YESNO);
		if (z == IDYES)
			pathname = backup_res.full_name.c_str();
	}

	// now we can actually load either the original path or the backup path
	if (!load_mission(pathname))
	{
		*Mission_filename = 0;
		return FALSE;
	}

	Fred_view_wnd->global_error_check();
	autosave("nothing");
	Undo_count = 0;
	return TRUE;
}

// For tokenizing
#define MAX_FILENAME_LEN_1	31
#if (MAX_FILENAME_LEN_1) != (MAX_FILENAME_LEN - 1)
#error MAX_FILENAME_LEN_1 must be equal to MAX_FILENAME_LEN - 1!
#endif

BOOL CFREDDoc::OnSaveDocument(LPCTSTR pathname) {
	Fred_mission_save save;
	if (Mission_save_format == FSO_FORMAT_RETAIL) {
		save.set_save_format(MissionFormat::RETAIL);
	} else if (Mission_save_format == FSO_FORMAT_COMPATIBILITY_MODE) {
		save.set_save_format(MissionFormat::COMPATIBILITY_MODE);
	} else {
		save.set_save_format(MissionFormat::STANDARD);
	}
	save.set_always_save_display_names(Always_save_display_names);
	save.set_view_pos(view_pos);
	save.set_view_orient(view_orient);
	save.set_fred_alt_names(Fred_alt_names);
	save.set_fred_callsigns(Fred_callsigns);

	DWORD attrib;
	FILE *fp;

	auto sep_ch = strrchr(pathname, '\\');
	auto filename = (sep_ch != nullptr) ? (sep_ch + 1) : pathname;
	auto len = strlen(filename);

	if (len >= MAX_FILENAME_LEN)
		Fred_main_wnd->MessageBox("The filename is too long for FreeSpace.  The game will not be able to read this file.  Max length, including extension, is " SCP_TOKEN_TO_STR(MAX_FILENAME_LEN_1) " characters.", NULL, MB_OK | MB_ICONEXCLAMATION);

	// drop extension and copy to Mission_filename
	auto ext_ch = strrchr(filename, '.');
	if (ext_ch != nullptr)
		len = ext_ch - filename;
	if (len >= 80)
		len = 79;
	strncpy(Mission_filename, filename, len);
	Mission_filename[len] = 0;


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

	if (save.save_mission_file(pathname)) {
		Fred_main_wnd->MessageBox("An error occured while saving!", NULL, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	SetModifiedFlag(FALSE);
	if (!load_mission(pathname, MPF_FAST_RELOAD))
		Error(LOCATION, "Failed attempting to reload mission after saving.  Report this bug now!");

	if (Briefing_dialog) {
		Briefing_dialog->restore_editor_state();
		Briefing_dialog->update_data(1);
	}

	return TRUE;
}

void CFREDDoc::Serialize(CArchive& ar) {
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

void CFREDDoc::recreate_dialogs() {
	if (Briefing_dialog) {
		Briefing_dialog->OnCancel();
		Briefing_dialog = new briefing_editor_dlg;
		Briefing_dialog->create();
		Briefing_dialog->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0,
									  SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		Briefing_dialog->ShowWindow(SW_RESTORE);
	}

	if (Bg_bitmap_dialog) {
		Bg_bitmap_dialog->OnClose();
		Bg_bitmap_dialog = new bg_bitmap_dlg;
		Bg_bitmap_dialog->create();
		Bg_bitmap_dialog->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0,
									   SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		Bg_bitmap_dialog->ShowWindow(SW_RESTORE);
	}
}

void CFREDDoc::UpdateStatus(int flags) {
	if (FREDDoc_ptr)
		if (flags & US_WORLD_CHANGED)
			FREDDoc_ptr->SetModifiedFlag();
}

void set_modified(BOOL arg) {
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
//		Fred_view_wnd->MessageBox(strerror(errno), "File Open Error!", MB_ICONSTOP);

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
