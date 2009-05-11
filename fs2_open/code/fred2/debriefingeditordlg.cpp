/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/DebriefingEditorDlg.cpp $
 * $Revision: 1.2.2.4 $
 * $Date: 2007-04-08 08:28:20 $
 * $Author: karajorma $
 *
 * Debriefing editor dialog.  Used to edit mission debriefings of course.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2.2.3  2007/03/07 22:36:51  karajorma
 * Make .ogg selectable in FRED.
 *
 * Revision 1.2.2.2  2006/09/20 04:53:53  taylor
 * properly fix the new_recommendation_text FRED problem, plus some real cleanup for that code in general, and some memory leak fixes
 *
 * Revision 1.2.2.1  2006/09/14 23:29:03  taylor
 * add default recommendation text for debriefing, it's either this or making sure that it's nul
 *
 * Revision 1.2  2006/01/26 04:01:58  Goober5000
 * spelling
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.4  2005/10/22 05:54:48  wmcoolmon
 * Fixed cf_find_file_location
 *
 * Revision 1.3  2005/09/29 05:18:59  Goober5000
 * the FRED stuff
 * --Goober5000
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:53  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:01p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 20    7/07/98 2:09p Hoffoss
 * Fixed bug where sexp for debriefing stages get cleared when inserting
 * or deleting stages.
 * 
 * 19    4/30/98 8:23p John
 * Fixed some bugs with Fred caused by my new cfile code.
 * 
 * 18    4/22/98 9:56a Sandeep
 * 
 * 17    4/20/98 4:40p Hoffoss
 * Added a button to 4 editors to play the chosen wave file.
 * 
 * 16    4/17/98 1:41p Allender
 * took out function calls in NDEBUG mode
 * 
 * 15    4/06/98 10:43a John
 * Fixed bugs with inserting/deleting stages
 * 
 * 14    4/03/98 12:39p Hoffoss
 * Changed starting directory for browse buttons in several editors.
 * 
 * 13    4/03/98 11:34a John
 * Fixed the stuff I broke in Fred from the new briefing
 * 
 * 12    3/17/98 2:06p Hoffoss
 * Made enter key not close the dialog box (default windows behavior, even
 * when no ok button.  Talk about stupid. :)
 * 
 * 11    2/09/98 9:25p Allender
 * team v team support.  multiple pools and briefings
 * 
 * 10    2/04/98 4:32p Allender
 * support for multiple briefings and debriefings.  Changes to mission
 * type (now a bitfield).  Bitfield defs for multiplayer modes
 * 
 * 9     1/02/98 3:29p Duncan
 * Fixed bug with sexp tree freeing.
 * 
 * 8     11/10/97 12:09p Johnson
 * Fixed sexp free error in debriefing editor.
 * 
 * 7     11/10/97 11:58a Johnson
 * Added support to debriefing editor for "press cancel to go to reference
 * of sexp".
 * 
 * 6     10/28/97 6:07p Jasen
 * Added some debugging code to detect sexp leaks.  Looks there are some I
 * will need to investigate more.
 * 
 * 5     10/27/97 2:06p Hoffoss
 * Fixed editor to allow correct character limit for recommendation text.
 * 
 * 4     10/14/97 12:06p Hoffoss
 * Recoded debriefing editor to utilize new format.
 * 
 * 3     10/14/97 10:35a Hoffoss
 * Hacked the hell out of this file to allow Fred to compile for now.
 * Next step is to fix it all.
 * 
 * 2     7/08/97 2:03p Hoffoss
 * Debriefing editor coded and implemented.
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"
#include "DebriefingEditorDlg.h"
#include "FREDDoc.h"
#include "mission/missionbriefcommon.h"
#include "parse/sexp.h"
#include "cfile/cfile.h"
#include "sound/audiostr.h"
#include "localization/localize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// debriefing_editor_dlg dialog

debriefing_editor_dlg::debriefing_editor_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(debriefing_editor_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(debriefing_editor_dlg)
	m_text = _T("");
	m_voice = _T("");
	m_stage_title = _T("");
	m_rec_text = _T("");
	m_current_debriefing = -1;
	//}}AFX_DATA_INIT

	modified = 0;
	m_cur_stage = 0;
	m_last_stage = -1;
	m_voice_id = -1;
	select_sexp_node = -1;
}

void debriefing_editor_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(debriefing_editor_dlg)
	DDX_Control(pDX, IDC_TREE, m_tree);
	DDX_Text(pDX, IDC_TEXT, m_text);
	DDX_Text(pDX, IDC_VOICE, m_voice);
	DDX_Text(pDX, IDC_STAGE_TITLE, m_stage_title);
	DDX_Text(pDX, IDC_REC_TEXT, m_rec_text);
	//}}AFX_DATA_MAP

	DDV_MaxChars(pDX, m_text, MAX_BRIEF_LEN - 1);
	DDV_MaxChars(pDX, m_voice, MAX_FILENAME_LEN - 1);
	DDV_MaxChars(pDX, m_rec_text, MAX_RECOMMENDATION_LEN - 1);
}

BEGIN_MESSAGE_MAP(debriefing_editor_dlg, CDialog)
	//{{AFX_MSG_MAP(debriefing_editor_dlg)
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_BN_CLICKED(IDC_PREV, OnPrev)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_ADD_STAGE, OnAddStage)
	ON_BN_CLICKED(IDC_DELETE_STAGE, OnDeleteStage)
	ON_BN_CLICKED(IDC_INSERT_STAGE, OnInsertStage)
	ON_NOTIFY(NM_RCLICK, IDC_TREE, OnRclickTree)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE, OnBeginlabeleditTree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE, OnEndlabeleditTree)
	ON_WM_CLOSE()
	ON_WM_INITMENU()
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// debriefing_editor_dlg message handlers

void debriefing_editor_dlg::OnInitMenu(CMenu* pMenu)
{
	int i;
	CMenu *m;

	// disable any items we should disable
	m = pMenu->GetSubMenu(0);

	// uncheck all menu items
	for (i = 0; i < Num_teams; i++ )
		m->CheckMenuItem( i, MF_BYPOSITION | MF_UNCHECKED );

	for ( i = Num_teams; i < MAX_TVT_TEAMS; i++ )
		m->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);


	// put a check next to the currently selected item
	m->CheckMenuItem(m_current_debriefing, MF_BYPOSITION | MF_CHECKED );

	CDialog::OnInitMenu(pMenu);
}

BOOL debriefing_editor_dlg::OnInitDialog()
{
	int i, n;

	CDialog::OnInitDialog();
	m_play_bm.LoadBitmap(IDB_PLAY);
	((CButton *) GetDlgItem(IDC_PLAY)) -> SetBitmap(m_play_bm);

	m_current_debriefing = 0;
	UpdateData(FALSE);

	Debriefing = &Debriefings[m_current_debriefing];

	m_tree.link_modified(&modified);  // provide way to indicate trees are modified in dialog
	n = m_tree.select_sexp_node = select_sexp_node;
	select_sexp_node = -1;
	if (n != -1) {
		for (i=0; i<Debriefing->num_stages; i++)
			if (query_node_in_sexp(n, Debriefing->stages[i].formula))
				break;

		if (i < Debriefing->num_stages) {
			m_cur_stage = i;
			update_data();
			GetDlgItem(IDC_TREE) -> SetFocus();
			m_tree.hilite_item(m_tree.select_sexp_node);
			set_modified();
			return FALSE;
		}
	}

	update_data();
	set_modified();

	// hard coded stuff to deal with the multiple briefings per mission.

	return TRUE;
}

void debriefing_editor_dlg::update_data(int update)
{
	int enable, save_debriefing;
	debrief_stage *ptr;

	save_debriefing = m_current_debriefing;

	if (update)
		UpdateData(TRUE);

	// based on the game type, enable the multiple briefings combo box (or disable it)

	// set up the pointer to the briefing that we are editing
	if ( save_debriefing != m_current_debriefing )
		Debriefing = &Debriefings[save_debriefing];
	else
		Debriefing = &Debriefings[m_current_debriefing];

	if (m_last_stage >= 0) {
		ptr = &Debriefing->stages[m_last_stage];
		if (ptr->formula >= 0)
			free_sexp2(ptr->formula);

		ptr->formula = m_tree.save_tree();
		deconvert_multiline_string(ptr->new_text, m_text, MAX_DEBRIEF_LEN);
		lcl_fred_replace_stuff(ptr->new_text, MAX_DEBRIEF_LEN);
		deconvert_multiline_string(ptr->new_recommendation_text, m_rec_text, MAX_RECOMMENDATION_LEN);
		lcl_fred_replace_stuff(ptr->new_recommendation_text,MAX_RECOMMENDATION_LEN);
		string_copy(ptr->voice, m_voice, MAX_FILENAME_LEN);
	}

	// now get new stage data
	if ((m_cur_stage >= 0) && (m_cur_stage < Debriefing->num_stages)) {
		ptr = &Debriefing->stages[m_cur_stage];
		m_stage_title.Format("Stage %d of %d", m_cur_stage + 1, Debriefing->num_stages);
		m_tree.load_tree(ptr->formula);
		m_text = convert_multiline_string(ptr->new_text);
		m_rec_text = convert_multiline_string(ptr->new_recommendation_text);
		m_voice = ptr->voice;
		enable = TRUE;

	} else {
		m_stage_title = _T("No stages");
		m_tree.clear_tree();
		m_text = _T("");
		m_rec_text = _T("");
		m_voice = _T("");
		enable = FALSE;
		m_cur_stage = -1;
	}

	if (m_cur_stage == Debriefing->num_stages - 1)
		GetDlgItem(IDC_NEXT) -> EnableWindow(FALSE);
	else
		GetDlgItem(IDC_NEXT) -> EnableWindow(enable);

	if (m_cur_stage)
		GetDlgItem(IDC_PREV) -> EnableWindow(enable);
	else
		GetDlgItem(IDC_PREV) -> EnableWindow(FALSE);

	if (Debriefing->num_stages >= MAX_DEBRIEF_STAGES)
		GetDlgItem(IDC_ADD_STAGE) -> EnableWindow(FALSE);
	else
		GetDlgItem(IDC_ADD_STAGE) -> EnableWindow(TRUE);

	if (Debriefing->num_stages) {
		GetDlgItem(IDC_DELETE_STAGE) -> EnableWindow(enable);
		GetDlgItem(IDC_INSERT_STAGE) -> EnableWindow(enable);

	} else {
		GetDlgItem(IDC_DELETE_STAGE) -> EnableWindow(FALSE);
		GetDlgItem(IDC_INSERT_STAGE) -> EnableWindow(FALSE);
	}

	GetDlgItem(IDC_VOICE) -> EnableWindow(enable);
	GetDlgItem(IDC_BROWSE) -> EnableWindow(enable);
	GetDlgItem(IDC_TEXT) -> EnableWindow(enable);
	GetDlgItem(IDC_REC_TEXT) -> EnableWindow(enable);
	GetDlgItem(IDC_TREE) -> EnableWindow(enable);

	m_last_stage = m_cur_stage;
	UpdateData(FALSE);

	#ifndef NDEBUG
	count_free_sexp_nodes();
	#endif
}

void debriefing_editor_dlg::OnNext() 
{
	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	m_cur_stage++;
	update_data();
}

void debriefing_editor_dlg::OnPrev() 
{
	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	m_cur_stage--;
	update_data();
}

void debriefing_editor_dlg::OnBrowse() 
{
	int z;
	CString name;

	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	UpdateData(TRUE);

	if (The_mission.game_type & MISSION_TYPE_TRAINING)
		z = cfile_push_chdir(CF_TYPE_VOICE_TRAINING);
	else
		z = cfile_push_chdir(CF_TYPE_VOICE_DEBRIEFINGS);

	CFileDialog dlg(TRUE, "wav", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR,
		"Voice Files (*.ogg, *.wav)|*.ogg;*.wav|Ogg Vorbis Files (*.ogg)|*.ogg|Wave Files (*.wav)|*.wav||");

	if (dlg.DoModal() == IDOK) {
		m_voice = dlg.GetFileName();
		UpdateData(FALSE);
	}

	if (!z)
		cfile_pop_dir();
}

void debriefing_editor_dlg::OnAddStage() 
{
	int i;

	if (Debriefing->num_stages >= MAX_DEBRIEF_STAGES)
		return;

	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	m_cur_stage = i = Debriefing->num_stages++;
	copy_stage(i - 1, i, 1);
	update_data(1);
}

void debriefing_editor_dlg::OnDeleteStage() 
{
	int i, z;

	if (m_cur_stage < 0)
		return;

	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	Assert(Debriefing->num_stages);
	z = m_cur_stage;
	m_cur_stage = -1;
	update_data(1);
	for (i=z+1; i<Debriefing->num_stages; i++) {
		copy_stage(i, i - 1);
	}

	Debriefing->num_stages--;
	m_cur_stage = z;
	if (m_cur_stage >= Debriefing->num_stages)
		m_cur_stage = Debriefing->num_stages - 1;

	update_data(0);
}

void debriefing_editor_dlg::OnInsertStage() 
{
	int i, z;

	if (Debriefing->num_stages >= MAX_DEBRIEF_STAGES)
		return;

	if (!Debriefing->num_stages) {
		OnAddStage();
		return;
	}

	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	z = m_cur_stage;
	m_cur_stage = -1;
	update_data(1);
	for (i=Debriefing->num_stages; i>z; i--) {
		copy_stage(i - 1, i);
	}

	Debriefing->num_stages++;
	copy_stage(z, z + 1);
	Debriefing->stages[z].formula = -1;
	m_cur_stage = z;
	update_data(0);
}

void debriefing_editor_dlg::copy_stage(int from, int to, int clear_formula)
{
	if ((from < 0) || (from >= Debriefing->num_stages)) {
		strcpy(Debriefing->stages[to].new_text, "<Text here>");
		strcpy(Debriefing->stages[to].voice, "none.wav");
		Debriefing->stages[to].formula = -1;
		return;
	}

	
	if (clear_formula)
		Debriefing->stages[to].formula = -1;
	else
		Debriefing->stages[to].formula = Debriefing->stages[from].formula;

	strcpy( Debriefing->stages[to].new_text, Debriefing->stages[from].new_text );
	strcpy( Debriefing->stages[to].voice, Debriefing->stages[from].voice );
	strcpy( Debriefing->stages[to].new_recommendation_text, Debriefing->stages[from].new_recommendation_text );
}

void debriefing_editor_dlg::OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_tree.right_clicked();	
	*pResult = 0;
}

void debriefing_editor_dlg::OnBeginlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (m_tree.edit_label(pTVDispInfo->item.hItem) == 1)	{
		*pResult = 0;
		modified = 1;

	} else
		*pResult = 1;
}

void debriefing_editor_dlg::OnEndlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_tree.end_label_edit(pTVDispInfo->item.hItem, pTVDispInfo->item.pszText);
}

void debriefing_editor_dlg::OnClose() 
{
	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;
	m_cur_stage = -1;
	update_data(1);
	CDialog::OnClose();
}

void debriefing_editor_dlg::OnOK()
{
}

BOOL debriefing_editor_dlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id;

	// deal with figuring out menu stuff
	id = LOWORD(wParam);
	if ( (id >= ID_TEAM_1) && (id < ID_TEAM_3) ) {
		update_data(1);

		// set the current debriefing
		m_current_debriefing = id - ID_TEAM_1;

		// put user back at first stage for this team (or no current stage is there are none).
		Debriefing = &Debriefings[m_current_debriefing];
		if ( Debriefing->num_stages > 0 )
			m_cur_stage = 0;
		else
			m_cur_stage = -1;

		m_last_stage = -1;
		update_data(0);
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

BOOL debriefing_editor_dlg::DestroyWindow() 
{
	audiostream_close_file(m_voice_id, 0);
	m_play_bm.DeleteObject();
	return CDialog::DestroyWindow();
}

void debriefing_editor_dlg::OnPlay() 
{
	GetDlgItem(IDC_VOICE)->GetWindowText(m_voice);

	if (m_voice_id >= 0) {
		audiostream_close_file(m_voice_id, 0);
		m_voice_id = -1;
		return;
	}

	// we use ASF_EVENTMUSIC here so that it will keep the extension in place
	m_voice_id = audiostream_open((char *)(LPCSTR) m_voice, ASF_EVENTMUSIC);

	if (m_voice_id >= 0) {
		audiostream_play(m_voice_id, 1.0f, 0);
	}
}
