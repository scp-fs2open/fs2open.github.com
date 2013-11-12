/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "stdafx.h"
#include "FRED.h"
#include "BriefingEditorDlg.h"
#include "FREDDoc.h"
#include "mission/missionbriefcommon.h"
#include "mission/missionparse.h"
#include "FredRender.h"
#include "Management.h"
#include "globalincs/linklist.h"
#include "MainFrm.h"
#include "bmpman/bmpman.h"
#include "gamesnd/eventmusic.h"
#include "starfield/starfield.h"
#include "jumpnode/jumpnode.h"
#include "cfile/cfile.h"
#include "object/objectdock.h"
#include "iff_defs/iff_defs.h"
#include "sound/audiostr.h"
#include "localization/localize.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_MENU		9000

static int Max_icons_for_lines;

/////////////////////////////////////////////////////////////////////////////
// briefing_editor_dlg dialog

briefing_editor_dlg::briefing_editor_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(briefing_editor_dlg::IDD, pParent)
{
	int i, z;

	// figure out max icons we can have with lines to each other less than max allowed lines.
	// Basically, # lines = (# icons - 1) factorial
	Max_icons_for_lines = 0;
	do {
		i = ++Max_icons_for_lines + 1;
		z = 0;
		while (--i)
			z += i;

	} while (z < MAX_BRIEF_STAGE_LINES);

	//{{AFX_DATA_INIT(briefing_editor_dlg)
	m_hilight = FALSE;
	m_icon_image = -1;
	m_icon_label = _T("");
	m_stage_title = _T("");
	m_text = _T("");
	m_time = _T("");
	m_voice = _T("");
	m_icon_text = _T("");
	m_icon_team = -1;
	m_ship_type = -1;
	m_change_local = FALSE;
	m_id = 0;
	m_briefing_music = -1;
	m_substitute_briefing_music = _T("");
	m_cut_next = FALSE;
	m_cut_prev = FALSE;
	m_current_briefing = -1;
	m_flipicon = FALSE;
	m_use_wing = FALSE;
	//}}AFX_DATA_INIT
	m_voice_id = -1;
	m_cur_stage = 0;
	m_last_stage = m_cur_icon = m_last_icon = -1;
	m_tree.link_modified(&modified);  // provide way to indicate trees are modified in dialog

	// copy view initialization
	m_copy_view_set = 0;
}

void briefing_editor_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(briefing_editor_dlg)
	DDX_Control(pDX, IDC_TREE, m_tree);
	DDX_Control(pDX, IDC_LINES, m_lines);
	DDX_Check(pDX, IDC_HILIGHT, m_hilight);
	DDX_CBIndex(pDX, IDC_ICON_IMAGE, m_icon_image);
	DDX_Text(pDX, IDC_ICON_LABEL, m_icon_label);
	DDX_Text(pDX, IDC_STAGE_TITLE, m_stage_title);
	DDX_Text(pDX, IDC_TEXT, m_text);
	DDX_Text(pDX, IDC_TIME, m_time);
	DDX_Text(pDX, IDC_VOICE, m_voice);
	DDX_Text(pDX, IDC_ICON_TEXT, m_icon_text);
	DDX_CBIndex(pDX, IDC_TEAM, m_icon_team);
	DDX_CBIndex(pDX, IDC_SHIP_TYPE, m_ship_type);
	DDX_Check(pDX, IDC_LOCAL, m_change_local);
	DDX_Text(pDX, IDC_ID, m_id);
	DDX_CBIndex(pDX, IDC_BRIEFING_MUSIC, m_briefing_music);
	DDX_Text(pDX, IDC_SUBSTITUTE_BRIEFING_MUSIC, m_substitute_briefing_music);
	DDX_Check(pDX, IDC_CUT_NEXT, m_cut_next);
	DDX_Check(pDX, IDC_CUT_PREV, m_cut_prev);
	DDX_Check(pDX, IDC_FLIP_ICON, m_flipicon);
	DDX_Check(pDX, IDC_USE_WING_ICON, m_use_wing);
	//}}AFX_DATA_MAP

	DDV_MaxChars(pDX, m_voice, MAX_FILENAME_LEN - 1);
	DDV_MaxChars(pDX, m_icon_label, MAX_LABEL_LEN - 1);
	DDV_MaxChars(pDX, m_icon_text, MAX_ICON_TEXT_LEN - 1);
}

BEGIN_MESSAGE_MAP(briefing_editor_dlg, CDialog)
	//{{AFX_MSG_MAP(briefing_editor_dlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_BN_CLICKED(IDC_PREV, OnPrev)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_ADD_STAGE, OnAddStage)
	ON_BN_CLICKED(IDC_DELETE_STAGE, OnDeleteStage)
	ON_BN_CLICKED(IDC_INSERT_STAGE, OnInsertStage)
	ON_BN_CLICKED(IDC_MAKE_ICON, OnMakeIcon)
	ON_BN_CLICKED(IDC_DELETE_ICON, OnDeleteIcon)
	ON_BN_CLICKED(IDC_GOTO_VIEW, OnGotoView)
	ON_BN_CLICKED(IDC_SAVE_VIEW, OnSaveView)
	ON_CBN_SELCHANGE(IDC_ICON_IMAGE, OnSelchangeIconImage)
	ON_CBN_SELCHANGE(IDC_TEAM, OnSelchangeTeam)
	ON_CBN_SELCHANGE(IDC_SHIP_TYPE, OnSelchangeShipType)
	ON_BN_CLICKED(IDC_PROPAGATE_ICONS, OnPropagateIcons)
	ON_WM_INITMENU()
	ON_BN_CLICKED(IDC_LINES, OnLines)
	ON_NOTIFY(NM_RCLICK, IDC_TREE, OnRclickTree)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE, OnBeginlabeleditTree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE, OnEndlabeleditTree)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_COPY_VIEW, OnCopyView)
	ON_BN_CLICKED(IDC_PASTE_VIEW, OnPasteView)
	ON_BN_CLICKED(IDC_FLIP_ICON, OnFlipIcon)
	ON_BN_CLICKED(IDC_USE_WING_ICON, OnWingIcon)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// briefing_editor_dlg message handlers

void briefing_editor_dlg::OnInitMenu(CMenu* pMenu)
{
	int i;
	CMenu *m;

	// disable any items we should disable
	m = pMenu->GetSubMenu(0);

	// uncheck all menu items
	for (i=0; i<Num_teams; i++)
		m->CheckMenuItem(i, MF_BYPOSITION | MF_UNCHECKED);

	for (i=Num_teams; i<MAX_TVT_TEAMS; i++)
		m->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);

	// put a check next to the currently selected item
	m->CheckMenuItem(m_current_briefing, MF_BYPOSITION | MF_CHECKED );

	// Karajorma - it might be nice to autobalance the briefings and debriefings but I'm not doing anything till I
	// understand how the how system works better. disabling the option for now. 
	m = pMenu->GetSubMenu(1); 
	m->EnableMenuItem(ID_AUTOBALANCE, MF_GRAYED); 

	CDialog::OnInitMenu(pMenu);
}

void briefing_editor_dlg::create()
{
	int i;
	CComboBox *box;

	CDialog::Create(IDD);
	theApp.init_window(&Briefing_wnd_data, this);
	box = (CComboBox *) GetDlgItem(IDC_ICON_IMAGE);
	for (i=0; i<MIN_BRIEF_ICONS; i++)
		box->AddString(Icon_names[i]);

	box = (CComboBox *) GetDlgItem(IDC_TEAM);
	for (i=0; i<Num_iffs; i++)
		box->AddString(Iff_info[i].iff_name);

	box = (CComboBox *) GetDlgItem(IDC_SHIP_TYPE);
	for (i=0; i<Num_ship_classes; i++)
		box->AddString(Ship_info[i].name);

	box = (CComboBox *) GetDlgItem(IDC_BRIEFING_MUSIC);
	box->AddString("None");
	for (i=0; i<Num_music_files; i++)
		box->AddString(Spooled_music[i].name);

	box = (CComboBox *) GetDlgItem(IDC_SUBSTITUTE_BRIEFING_MUSIC);
	box->AddString("None");
	for (i=0; i<Num_music_files; i++)
		box->AddString(Spooled_music[i].name);

	m_play_bm.LoadBitmap(IDB_PLAY);
	((CButton *) GetDlgItem(IDC_PLAY)) -> SetBitmap(m_play_bm);

	m_current_briefing = 0;
	Briefing = &Briefings[m_current_briefing];
	m_briefing_music = Mission_music[SCORE_BRIEFING] + 1;
	m_substitute_briefing_music = The_mission.substitute_briefing_music_name;

	UpdateData(FALSE);
	update_data();
	OnGotoView();
	update_map_window();
}

void briefing_editor_dlg::focus_sexp(int select_sexp_node)
{
	int i, n;

	n = m_tree.select_sexp_node = select_sexp_node;
	if (n != -1) {
		for (i=0; i<Briefing->num_stages; i++)
			if (query_node_in_sexp(n, Briefing->stages[i].formula))
				break;

		if (i < Briefing->num_stages) {
			m_cur_stage = i;
			update_data();
			GetDlgItem(IDC_TREE) -> SetFocus();
			m_tree.hilite_item(m_tree.select_sexp_node);
		}
	}
}

void briefing_editor_dlg::OnOK()
{
}

void briefing_editor_dlg::OnCancel()
{
	OnClose();
}

void briefing_editor_dlg::OnClose() 
{
	int bs, i, j, s, t, dup = 0;
	briefing_editor_dlg *ptr;
	brief_stage *sp;

	m_cur_stage = -1;
	update_data(1);

	audiostream_close_file(m_voice_id, 0);

	for ( bs = 0; bs < Num_teams; bs++ ) {
		for (s=0; s<Briefing[bs].num_stages; s++) {
			sp = &Briefing[bs].stages[s];
			t = sp->num_icons;
			for (i=0; i<t-1; i++)
				for (j=i+1; j<t; j++) {
					if ((sp->icons[i].id >= 0) && (sp->icons[i].id == sp->icons[j].id))
						dup = 1;
				}
		}
	}

	if (dup)
		MessageBox("You have duplicate icons names.  You should resolve these.", "Warning");

	theApp.record_window_data(&Briefing_wnd_data, this);
	ptr = Briefing_dialog;
	Briefing_dialog = NULL;
	delete ptr;
}

void briefing_editor_dlg::reset_editor()
{
	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	m_cur_stage = -1;
	update_data(0);
}

// some kind of hackish code to get around the problem if saving (which implicitly loads,
// which implicitly clears all mission info) not affecting the editor's state at save.
void briefing_editor_dlg::save_editor_state()
{
	stage_saved = m_cur_stage;
	icon_saved = m_cur_icon;
}

void briefing_editor_dlg::restore_editor_state()
{
	m_cur_stage = stage_saved;
	m_cur_icon = icon_saved;
}

void briefing_editor_dlg::update_data(int update)
{
	char buf[MAX_LABEL_LEN], buf2[MAX_ICON_TEXT_LEN];
	SCP_string buf3;
	int i, j, l, lines, count, enable = TRUE, valid = 0, invalid = 0;
	object *objp;
	brief_stage *ptr = NULL;

	if (update)
		UpdateData(TRUE);

	// save off current data before we update over it with new briefing stage/team stuff
	Briefing = save_briefing;

	Mission_music[SCORE_BRIEFING] = m_briefing_music - 1;
	strcpy_s(The_mission.substitute_briefing_music_name, m_substitute_briefing_music);
	if (m_last_stage >= 0) {
		ptr = &Briefing->stages[m_last_stage];
		deconvert_multiline_string(buf3, m_text);
		lcl_fred_replace_stuff(buf3);
		if (ptr->text != buf3)
			set_modified();

		ptr->text = buf3;
		MODIFY(ptr->camera_time, atoi(m_time));
		string_copy(ptr->voice, m_voice, MAX_FILENAME_LEN, 1);
		i = ptr->flags;
		if (m_cut_prev)
			i |= BS_BACKWARD_CUT;
		else
			i &= ~BS_BACKWARD_CUT;

		if (m_cut_next)
			i |= BS_FORWARD_CUT;
		else
			i &= ~BS_FORWARD_CUT;

		MODIFY(ptr->flags, i);
		ptr->formula = m_tree.save_tree();
		switch (m_lines.GetCheck()) {
			case 1:
				// add lines between every pair of 2 marked icons if there isn't one already.
				for (i=0; i<ptr->num_icons - 1; i++)
					for (j=i+1; j<ptr->num_icons; j++) {
						if ( icon_marked[i] && icon_marked[j] ) {
							for (l=0; l<ptr->num_lines; l++)
								if ( ((ptr->lines[l].start_icon == i) && (ptr->lines[l].end_icon == j)) || ((ptr->lines[l].start_icon == j) && (ptr->lines[l].end_icon == i)) )
									break;

							if ((l == ptr->num_lines) && (l < MAX_BRIEF_STAGE_LINES)) {
								ptr->lines[l].start_icon = i;
								ptr->lines[l].end_icon = j;
								ptr->num_lines++;
							}
						}
					}

				break;

			case 0:
				// remove all existing lines between any 2 marked icons
				i = ptr->num_lines;
				while (i--)
					if ( icon_marked[ptr->lines[i].start_icon] && icon_marked[ptr->lines[i].end_icon] ) {
						ptr->num_lines--;
						for (l=i; l<ptr->num_lines; l++)
							ptr->lines[l] = ptr->lines[l + 1];
					}

				break;
		}

		//WMC - Safeguard against broken lines
		i = ptr->num_lines;
		while(i--)
		{
			if(ptr->lines[i].start_icon < 0 || ptr->lines[i].end_icon < 0 || ptr->lines[i].start_icon >= ptr->num_icons || ptr->lines[i].end_icon >= ptr->num_icons)
			{
				ptr->num_lines--;
				for(l = i; l < ptr->num_lines; l++)
					ptr->lines[l] = ptr->lines[l+1];
			}
		}

		if (m_last_icon >= 0) {
			valid = (m_id != ptr->icons[m_last_icon].id);
			if (m_id >= 0) {
				if (valid && !m_change_local) {
					for (i=m_last_stage+1; i<Briefing->num_stages; i++) {
						if (find_icon(m_id, i) >= 0) {
							char msg[1024];

							valid = 0;
							sprintf(msg, "Icon ID #%d is already used in a later stage.  You can only\n"
								"change to that ID locally.  Icon ID has been reset back to %d", m_id, ptr->icons[m_last_icon].id);

							m_id = ptr->icons[m_last_icon].id;
							MessageBox(msg);
							break;
						}
					}
				}

				for (i=0; i<ptr->num_icons; i++)
					if ((i != m_last_icon) && (ptr->icons[i].id == m_id)) {
						char msg[1024];

						sprintf(msg, "Icon ID #%d is already used in this stage.  Icon ID has been reset back to %d",
							m_id, ptr->icons[m_last_icon].id);

						m_id = ptr->icons[m_last_icon].id;
						MessageBox(msg);
						break;
					}

				if (valid && !m_change_local) {
					set_modified();
					reset_icon_loop(m_last_stage);
					while (get_next_icon(ptr->icons[m_last_icon].id))
						iconp->id = m_id;
				}
			}

			ptr->icons[m_last_icon].id = m_id;
			string_copy(buf, m_icon_label, MAX_LABEL_LEN);
			if (stricmp(ptr->icons[m_last_icon].label, buf) && !m_change_local) {
				set_modified();
				reset_icon_loop(m_last_stage);
				while (get_next_icon(m_id))
					strcpy_s(iconp->label, buf);
			}

			strcpy_s(ptr->icons[m_last_icon].label, buf);

			if ( m_hilight )
				ptr->icons[m_last_icon].flags |= BI_HIGHLIGHT;
			else
				ptr->icons[m_last_icon].flags &= ~BI_HIGHLIGHT;

			if (m_flipicon)
				ptr->icons[m_last_icon].flags |= BI_MIRROR_ICON;
			else
				ptr->icons[m_last_icon].flags &= ~BI_MIRROR_ICON;

			if (m_use_wing)
				ptr->icons[m_last_icon].flags |= BI_USE_WING_ICON;
			else
				ptr->icons[m_last_icon].flags &= ~BI_USE_WING_ICON;

			if ((ptr->icons[m_last_icon].type != m_icon_image) && !m_change_local) {
				set_modified();
				reset_icon_loop(m_last_stage);
				while (get_next_icon(m_id))
					iconp->type = m_icon_image;
			}
			ptr->icons[m_last_icon].type = m_icon_image;

			if ((ptr->icons[m_last_icon].team != m_icon_team) && !m_change_local) {
				set_modified();
				reset_icon_loop(m_last_stage);
				while (get_next_icon(m_id))
					iconp->team = m_icon_team;
			}
			ptr->icons[m_last_icon].team = m_icon_team;

			if ((ptr->icons[m_last_icon].ship_class != m_ship_type) && !m_change_local) {
				set_modified();
				reset_icon_loop(m_last_stage);
				while (get_next_icon(m_id))
					iconp->ship_class = m_ship_type;
			}
			MODIFY(ptr->icons[m_last_icon].ship_class, m_ship_type);

			deconvert_multiline_string(buf2, m_icon_text, MAX_ICON_TEXT_LEN);
/*
			if (stricmp(ptr->icons[m_last_icon].text, buf2) && !m_change_local) {
				set_modified();
				reset_icon_loop(m_last_stage);
				while (get_next_icon(m_id))
					strcpy_s(iconp->text, buf2);
			}

			strcpy_s(ptr->icons[m_last_icon].text, buf2);
*/
		}
	}

	if (!::IsWindow(m_hWnd))
		return;

	// set briefing pointer to correct team
	Briefing = &Briefings[m_current_briefing];

	if ((m_cur_stage >= 0) && (m_cur_stage < Briefing->num_stages)) {
		ptr = &Briefing->stages[m_cur_stage];
		m_stage_title.Format("Stage %d of %d", m_cur_stage + 1, Briefing->num_stages);
		convert_multiline_string(m_text, ptr->text);
		m_time.Format("%d", ptr->camera_time);
		m_voice = ptr->voice;
		m_cut_prev = (ptr->flags & BS_BACKWARD_CUT) ? 1 : 0;
		m_cut_next = (ptr->flags & BS_FORWARD_CUT) ? 1 : 0;
		m_tree.load_tree(ptr->formula);

	} else {
		m_stage_title = _T("No stages");
		m_text = _T("");
		m_time = _T("");
		m_voice = _T("");
		m_cut_prev = m_cut_next = 0;
		m_tree.clear_tree();
		enable = FALSE;
		m_cur_stage = -1;
	}

	if (m_cur_stage == Briefing->num_stages - 1)
		GetDlgItem(IDC_NEXT) -> EnableWindow(FALSE);
	else
		GetDlgItem(IDC_NEXT) -> EnableWindow(enable);

	if (m_cur_stage)
		GetDlgItem(IDC_PREV) -> EnableWindow(enable);
	else
		GetDlgItem(IDC_PREV) -> EnableWindow(FALSE);

	if (Briefing->num_stages >= MAX_BRIEF_STAGES)
		GetDlgItem(IDC_ADD_STAGE) -> EnableWindow(FALSE);
	else
		GetDlgItem(IDC_ADD_STAGE) -> EnableWindow(TRUE);

	if (Briefing->num_stages) {
		GetDlgItem(IDC_DELETE_STAGE) -> EnableWindow(enable);
		GetDlgItem(IDC_INSERT_STAGE) -> EnableWindow(enable);
	} else {
		GetDlgItem(IDC_DELETE_STAGE) -> EnableWindow(FALSE);
		GetDlgItem(IDC_INSERT_STAGE) -> EnableWindow(FALSE);
	}

	GetDlgItem(IDC_TIME) -> EnableWindow(enable);
	GetDlgItem(IDC_VOICE) -> EnableWindow(enable);
	GetDlgItem(IDC_BROWSE) -> EnableWindow(enable);
	GetDlgItem(IDC_TEXT) -> EnableWindow(enable);
	GetDlgItem(IDC_SAVE_VIEW) -> EnableWindow(enable);
	GetDlgItem(IDC_GOTO_VIEW) -> EnableWindow(enable);
	GetDlgItem(IDC_CUT_PREV) -> EnableWindow(enable);
	GetDlgItem(IDC_CUT_NEXT) -> EnableWindow(enable);
	GetDlgItem(IDC_TREE) -> EnableWindow(enable);
	GetDlgItem(IDC_PLAY) -> EnableWindow(enable);

	if ((m_cur_stage >= 0) && (m_cur_icon >= 0) && (m_cur_icon < ptr->num_icons)) {
		m_hilight = (ptr->icons[m_cur_icon].flags & BI_HIGHLIGHT)?1:0;
		m_flipicon = (ptr->icons[m_cur_icon].flags & BI_MIRROR_ICON)?1:0;
		m_use_wing = (ptr->icons[m_cur_icon].flags & BI_USE_WING_ICON)?1:0;
		m_icon_image = ptr->icons[m_cur_icon].type;
		m_icon_team = ptr->icons[m_cur_icon].team;
		m_icon_label = ptr->icons[m_cur_icon].label;
		m_ship_type = ptr->icons[m_cur_icon].ship_class;
//		m_icon_text = convert_multiline_string(ptr->icons[m_cur_icon].text);
		m_id = ptr->icons[m_cur_icon].id;
		enable = TRUE;

	} else {
		m_flipicon = FALSE;
		m_hilight = FALSE;
		m_use_wing = FALSE;
		m_icon_image = -1;
		m_icon_team = -1;
		m_ship_type = -1;
		m_icon_label = _T("");
		m_cur_icon = -1;
		m_id = 0;
		enable = FALSE;
	}

	// see if icon is overridden by ships.tbl
	// if so, disable the icon type box
	int sip_bii_ship = (m_ship_type >= 0) ? Ship_info[m_ship_type].bii_index_ship : -1;
	int sip_bii_wing = (sip_bii_ship >= 0) ? Ship_info[m_ship_type].bii_index_wing : -1;

	GetDlgItem(IDC_USE_WING_ICON) -> ShowWindow(sip_bii_wing >= 0);

	GetDlgItem(IDC_ICON_TEXT) -> EnableWindow(enable);
	GetDlgItem(IDC_ICON_LABEL) -> EnableWindow(enable);
	GetDlgItem(IDC_ICON_IMAGE) -> EnableWindow(enable && (sip_bii_ship < 0));
	GetDlgItem(IDC_SHIP_TYPE) -> EnableWindow(enable);
	GetDlgItem(IDC_HILIGHT) -> EnableWindow(enable);
	GetDlgItem(IDC_FLIP_ICON) -> EnableWindow(enable);
	GetDlgItem(IDC_USE_WING_ICON) -> EnableWindow(enable);
	GetDlgItem(IDC_LOCAL) -> EnableWindow(enable);
	GetDlgItem(IDC_TEAM) -> EnableWindow(enable);
	GetDlgItem(IDC_ID) -> EnableWindow(enable);
	GetDlgItem(IDC_DELETE_ICON) -> EnableWindow(enable);

	valid = invalid = 0;
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if (objp->flags & OF_MARKED) {
			if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START) || (objp->type == OBJ_WAYPOINT) || (objp->type == OBJ_JUMP_NODE))
				valid = 1;
			else
				invalid = 1;
		}

		objp = GET_NEXT(objp);
	}

	if (m_cur_stage >= 0)
		ptr = &Briefing->stages[m_cur_stage];

	if (valid && !invalid && (m_cur_stage >= 0) && (ptr->num_icons < MAX_STAGE_ICONS))
		GetDlgItem(IDC_MAKE_ICON) -> EnableWindow(TRUE);
	else
		GetDlgItem(IDC_MAKE_ICON) -> EnableWindow(FALSE);

	if (m_cur_stage >= 0)
		for (i=0; i<ptr->num_icons; i++)
			icon_marked[i] = 0;

	valid = invalid = 0;
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if (objp->flags & OF_MARKED) {
			if (objp->type == OBJ_POINT) {
				valid++;
				icon_marked[objp->instance] = 1;

			} else
				invalid++;
		}

		objp = GET_NEXT(objp);
	}

	if (valid && !invalid && (m_cur_stage >= 0))
		GetDlgItem(IDC_PROPAGATE_ICONS) -> EnableWindow(TRUE);
	else
		GetDlgItem(IDC_PROPAGATE_ICONS) -> EnableWindow(FALSE);

	count = 0;
	lines = 1;  // default lines checkbox to checked
	
	if (m_cur_stage >= 0) {
		for (i=0; i<ptr->num_lines; i++)
			line_marked[i] = 0;

		// go through and locate all lines between marked icons
		for (i=0; i<ptr->num_icons - 1; i++)
			for (j=i+1; j<ptr->num_icons; j++) {
				if ( icon_marked[i] && icon_marked[j] ) {
					for (l=0; l<ptr->num_lines; l++)
						if ( ((ptr->lines[l].start_icon == i) && (ptr->lines[l].end_icon == j)) || ((ptr->lines[l].start_icon == j) && (ptr->lines[l].end_icon == i)) ) {
							line_marked[l] = 1;
							count++;  // track number of marked lines (lines between 2 icons that are both marked)
							break;
						}

					// at least 1 line missing between 2 marked icons, so use mixed state
					if (l == ptr->num_lines)
						lines = 2;
				}
			}
	}

	// not even 1 line between any 2 marked icons?  Set checkbox to unchecked.
	if (!count)
		lines = 0;

	i = 0;
	if (m_cur_stage >= 0){
		i = calc_num_lines_for_icons(valid) + ptr->num_lines - count;
	}

	if ((valid > 1) && !invalid && (m_cur_stage >= 0) && (i <= MAX_BRIEF_STAGE_LINES))
		GetDlgItem(IDC_LINES) -> EnableWindow(TRUE);
	else
		GetDlgItem(IDC_LINES) -> EnableWindow(FALSE);

	m_lines.SetCheck(lines);

	UpdateData(FALSE);
	if ((m_last_stage != m_cur_stage) || (Briefing != save_briefing)) {
		if (m_last_stage >= 0) {
			for (i=0; i<save_briefing->stages[m_last_stage].num_icons; i++) {
				// save positions of all icons, in case they have moved
				save_briefing->stages[m_last_stage].icons[i].pos = Objects[icon_obj[i]].pos;
				// release objects being used by last stage
				obj_delete(icon_obj[i]);
			}
		}

		if (m_cur_stage >= 0) {
			for (i=0; i<ptr->num_icons; i++) {
				// create an object for each icon for display/manipulation purposes
				icon_obj[i] = obj_create(OBJ_POINT, -1, i, NULL, &ptr->icons[i].pos, 0.0f, OF_RENDERS);
			}

			obj_merge_created_list();
		}

		m_last_stage = m_cur_stage;
	}

	m_last_icon = m_cur_icon;
	Update_window = 1;
	save_briefing = Briefing;
}

// Given a number of icons, figure out how many lines would be required to connect each one
// with all of the others.
//
int briefing_editor_dlg::calc_num_lines_for_icons(int num)
{
	int lines = 0;

	if (num < 2)
		return 0;

	// Basically, this is # lines = (# icons - 1) factorial
	while (--num)
		lines += num;

	return lines;
}

void briefing_editor_dlg::OnNext()
{
	m_cur_stage++;
	m_cur_icon = -1;
	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;
	update_data();
	OnGotoView();
}

void briefing_editor_dlg::OnPrev()
{
	m_cur_stage--;
	m_cur_icon = -1;
	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;
	update_data();
	OnGotoView();
}

void briefing_editor_dlg::OnBrowse() 
{
	int z;
	CString name;

	UpdateData(TRUE);

	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	if (The_mission.game_type & MISSION_TYPE_TRAINING)
		z = cfile_push_chdir(CF_TYPE_VOICE_TRAINING);
	else
		z = cfile_push_chdir(CF_TYPE_VOICE_BRIEFINGS);

	CFileDialog dlg(TRUE, "wav", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR,
		"Voice Files (*.ogg, *.wav)|*.ogg;*.wav|Ogg Vorbis Files (*.ogg)|*.ogg|Wave Files (*.wav)|*.wav||");

	if (dlg.DoModal() == IDOK) {
		m_voice = dlg.GetFileName();
		UpdateData(FALSE);
	}

	if (!z)
		cfile_pop_dir();
}

void briefing_editor_dlg::OnAddStage() 
{
	int i;

	if (Briefing->num_stages >= MAX_BRIEF_STAGES)
		return;

	m_cur_stage = i = Briefing->num_stages++;
	copy_stage(i - 1, i);
	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;
	update_data(1);
}

void briefing_editor_dlg::OnDeleteStage() 
{
	int i, z;

	if (m_cur_stage < 0)
		return;

	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	Assert(Briefing->num_stages);
	z = m_cur_stage;
	m_cur_stage = -1;
	update_data(1);
	for (i=z+1; i<Briefing->num_stages; i++)	{
		copy_stage(i, i-1);
	}

	Briefing->num_stages--;
	m_cur_stage = z;
	if (m_cur_stage >= Briefing->num_stages)
		m_cur_stage = Briefing->num_stages - 1;

	update_data(0);
}

void briefing_editor_dlg::draw_icon(object *objp)
{
	if (m_cur_stage < 0)
		return;

	brief_render_icon(m_cur_stage, objp->instance, 1.0f/30.0f, objp->flags & OF_MARKED,
		(float) True_rw / BRIEF_GRID_W, (float) True_rh / BRIEF_GRID_H);
}

void briefing_editor_dlg::batch_render()
{
	int i, num_lines;


	if (m_cur_stage < 0)
		return;

	num_lines = Briefing->stages[m_cur_stage].num_lines;
	for (i=0; i<num_lines; i++)
		brief_render_icon_line(m_cur_stage, i);
}

void briefing_editor_dlg::icon_select(int num)
{
	m_cur_icon = num;
	update_data(1);
}

void briefing_editor_dlg::OnInsertStage() 
{
	int i, z;

	if (Briefing->num_stages >= MAX_BRIEF_STAGES)
		return;

	if (!Briefing->num_stages) {
		OnAddStage();
		return;
	}

	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	z = m_cur_stage;
	m_cur_stage = -1;
	update_data(1);
	for (i=Briefing->num_stages; i>z; i--)	{
		copy_stage(i-1, i);
	}

	Briefing->num_stages++;
	copy_stage(z, z + 1);
	m_cur_stage = z;
	update_data(0);
}

void briefing_editor_dlg::copy_stage(int from, int to)
{
	if ((from < 0) || (from >= Briefing->num_stages)) {
		Briefing->stages[to].text = "<Text here>";
		strcpy_s(Briefing->stages[to].voice, "none.wav");
		Briefing->stages[to].camera_pos = view_pos;
		Briefing->stages[to].camera_orient = view_orient;
		Briefing->stages[to].camera_time = 500;
		Briefing->stages[to].num_icons = 0;
		Briefing->stages[to].formula = Locked_sexp_true;
		return;
	}

	// Copy all the data in the stage structure.
	Briefing->stages[to].text = Briefing->stages[from].text;
	strcpy_s( Briefing->stages[to].voice, Briefing->stages[from].voice );
	Briefing->stages[to].camera_pos = Briefing->stages[from].camera_pos;
	Briefing->stages[to].camera_orient = Briefing->stages[from].camera_orient;
	Briefing->stages[to].camera_time = Briefing->stages[from].camera_time;
	Briefing->stages[to].flags = Briefing->stages[from].flags;
	Briefing->stages[to].num_icons = Briefing->stages[from].num_icons;
	Briefing->stages[to].num_lines = Briefing->stages[from].num_lines;
	Briefing->stages[to].formula = Briefing->stages[from].formula;

	memmove( Briefing->stages[to].icons, Briefing->stages[from].icons, sizeof(brief_icon)*MAX_STAGE_ICONS );
	memmove( Briefing->stages[to].lines, Briefing->stages[from].lines, sizeof(brief_line)*MAX_BRIEF_STAGE_LINES );
}

void briefing_editor_dlg::update_positions()
{
	int i, s, z;
	vec3d v1, v2;

	if (m_cur_stage < 0)
		return;

	for (i=0; i<Briefing->stages[m_cur_stage].num_icons; i++) {
		v1 = Briefing->stages[m_cur_stage].icons[i].pos;
		v2 = Objects[icon_obj[i]].pos;
		if ((v1.xyz.x != v2.xyz.x) || (v1.xyz.y != v2.xyz.y) || (v1.xyz.z != v2.xyz.z)) {
			Briefing->stages[m_cur_stage].icons[i].pos = Objects[icon_obj[i]].pos;
			if (!m_change_local)  // propagate changes through rest of stages..
				for (s=m_cur_stage+1; s<Briefing->num_stages; s++) {
					z = find_icon(Briefing->stages[m_cur_stage].icons[i].id, s);
					if (z >= 0)
						Briefing->stages[s].icons[z].pos = Objects[icon_obj[i]].pos;
				}
		}
	}
}

void briefing_editor_dlg::OnMakeIcon() 
{
	char *name;
	int z, len, team, ship, waypoint, count = -1;
	int cargo = 0, cargo_count = 0, freighter_count = 0;
	object *ptr;
	vec3d min, max, pos;
	brief_icon *iconp;

	if (Briefing->stages[m_cur_stage].num_icons >= MAX_STAGE_ICONS)
		return;

	m_cur_icon = Briefing->stages[m_cur_stage].num_icons++;
	iconp = &Briefing->stages[m_cur_stage].icons[m_cur_icon];
	ship = waypoint = -1;
	team = 0;
	SCP_list<CJumpNode>::iterator jnp;

	vm_vec_make(&min, 9e19f, 9e19f, 9e19f);
	vm_vec_make(&max, -9e19f, -9e19f, -9e19f);
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags & OF_MARKED) {
			if (ptr->pos.xyz.x < min.xyz.x)
				min.xyz.x = ptr->pos.xyz.x;
			if (ptr->pos.xyz.x > max.xyz.x)
				max.xyz.x = ptr->pos.xyz.x;
			if (ptr->pos.xyz.y < min.xyz.y)
				min.xyz.y = ptr->pos.xyz.y;
			if (ptr->pos.xyz.y > max.xyz.y)
				max.xyz.y = ptr->pos.xyz.y;
			if (ptr->pos.xyz.z < min.xyz.z)
				min.xyz.z = ptr->pos.xyz.z;
			if (ptr->pos.xyz.z > max.xyz.z)
				max.xyz.z = ptr->pos.xyz.z;
			
			switch (ptr->type) {
				case OBJ_SHIP:
				case OBJ_START:
					ship = ptr->instance;
					break;

				case OBJ_WAYPOINT:
					waypoint = ptr->instance;
					break;
				
				case OBJ_JUMP_NODE:
					for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) { 
						if(jnp->GetSCPObject() == ptr) 
							break; 
					} 
					break;

				default:
					Int3();
			}

			if (ship >= 0) {
				team = Ships[ship].team;

				z = ship_query_general_type(ship);
				if (Ship_info[Ships[ship].ship_info_index].flags & SIF_CARGO)
					cargo_count++;

				if (Ship_info[Ships[ship].ship_info_index].flags & SIF_FREIGHTER)
				{
					// direct docked with any marked cargo?
					for (dock_instance *dock_ptr = ptr->dock_list; dock_ptr != NULL; dock_ptr = dock_ptr->next)
					{
						if (dock_ptr->docked_objp->flags & OF_MARKED)
						{
							if (Ship_info[Ships[dock_ptr->docked_objp->instance].ship_info_index].flags & SIF_CARGO)
								freighter_count++;
						}
					}
				}
			}

			count++;
		}

		ptr = GET_NEXT(ptr);
	}

	if (cargo_count && cargo_count == freighter_count)
		cargo = 1;

	vm_vec_avg(&pos, &min, &max);
	if (ship >= 0)
		name = Ships[ship].ship_name;
	else if (waypoint >= 0)
	{
		waypoint_list *wp_list = find_waypoint_list_with_instance(waypoint);
		Assert(wp_list != NULL);
		name = wp_list->get_name();
	}
	else if (jnp != Jump_nodes.end())
		name = jnp->GetName();
	else
		return;

	len = strlen(name);
	if (len >= MAX_LABEL_LEN - 1)
		len = MAX_LABEL_LEN - 1;

	strncpy(iconp->label, name, len);
	iconp->label[len] = 0;
//	iconp->text[0] = 0;
	iconp->type = 0;
	iconp->team = team;
	iconp->pos = pos;
	iconp->flags = 0;
	iconp->id = Cur_brief_id++;
	if (ship >= 0) {
		iconp->ship_class = Ships[ship].ship_info_index;
		switch (Ship_info[Ships[ship].ship_info_index].flags & SIF_ALL_SHIP_TYPES) {
			case SIF_KNOSSOS_DEVICE:
				iconp->type = ICON_KNOSSOS_DEVICE;
				break;

			case SIF_CORVETTE:
				iconp->type = ICON_CORVETTE;
				break;

			case SIF_GAS_MINER:
				iconp->type = ICON_GAS_MINER;
				break;

			case SIF_SUPERCAP:
				iconp->type = ICON_SUPERCAP;
				break;

			case SIF_SENTRYGUN:
				iconp->type = ICON_SENTRYGUN;
				break;

			case SIF_AWACS:
				iconp->type = ICON_AWACS;
				break;

			case SIF_CARGO:
				if (cargo)
					iconp->type = (count == 1) ? ICON_FREIGHTER_WITH_CARGO : ICON_FREIGHTER_WING_WITH_CARGO;
				else
					iconp->type = count ? ICON_CARGO_WING : ICON_CARGO;

				break;

			case SIF_SUPPORT:
				iconp->type = ICON_SUPPORT_SHIP;
				break;

			case SIF_FIGHTER:
				iconp->type = count ? ICON_FIGHTER_WING : ICON_FIGHTER;
				break;

			case SIF_BOMBER:
				iconp->type = count ? ICON_BOMBER_WING : ICON_BOMBER;
				break;

			case SIF_FREIGHTER:
				if (cargo)
					iconp->type = (count == 1) ? ICON_FREIGHTER_WITH_CARGO : ICON_FREIGHTER_WING_WITH_CARGO;
				else
					iconp->type = count ? ICON_FREIGHTER_WING_NO_CARGO : ICON_FREIGHTER_NO_CARGO;

				break;

			case SIF_CRUISER:
				iconp->type = count ? ICON_CRUISER_WING : ICON_CRUISER;
				break;

			case SIF_TRANSPORT:
				iconp->type = count ? ICON_TRANSPORT_WING : ICON_TRANSPORT;
				break;

			case SIF_CAPITAL:			
			case SIF_DRYDOCK:
				iconp->type = ICON_CAPITAL;
				break;			

			case SIF_NAVBUOY:
				iconp->type = ICON_WAYPOINT;
				break;

			default:
				iconp->type = ICON_ASTEROID_FIELD;
				break;
		}
	}
	// jumpnodes
	else if(jnp != Jump_nodes.end()){
		// find the first navbuoy
		iconp->ship_class = -1;
		for (int i = 0; i < Num_ship_classes; i++)
		{
			if (Ship_info[i].flags & SIF_NAVBUOY)
			{
				iconp->ship_class = i;
				break;
			}
		}
		iconp->type = ICON_JUMP_NODE;
	} 
	// everything else
	else {
		// find the first navbuoy
		iconp->ship_class = -1;
		for (int i = 0; i < Num_ship_classes; i++)
		{
			if (Ship_info[i].flags & SIF_NAVBUOY)
			{
				iconp->ship_class = i;
				break;
			}
		}
		iconp->type = ICON_WAYPOINT;
	}

	if (!m_change_local){
		propagate_icon(m_cur_icon);
	}

	icon_obj[m_cur_icon] = obj_create(OBJ_POINT, -1, m_cur_icon, NULL, &pos, 0.0f, OF_RENDERS);
	Assert(icon_obj[m_cur_icon] >= 0);
	obj_merge_created_list();
	unmark_all();
	set_cur_object_index(icon_obj[m_cur_icon]);
	GetDlgItem(IDC_MAKE_ICON) -> EnableWindow(FALSE);
	GetDlgItem(IDC_PROPAGATE_ICONS) -> EnableWindow(TRUE);
	update_data(1);
}

void briefing_editor_dlg::OnDeleteIcon()
{
	delete_icon(m_cur_icon);
}

void briefing_editor_dlg::delete_icon(int num)
{
	int i, z;

	if (num < 0)
		num = m_cur_icon;

	if (num < 0)
		return;

	Assert(m_cur_stage >= 0);
	Assert(Briefing->stages[m_cur_stage].num_icons);
	z = m_cur_icon;
	if (z == num)
		z = -1;
	if (z > num)
		z--;

	m_cur_icon = -1;
	update_data(1);
	obj_delete(icon_obj[num]);
	for (i=num+1; i<Briefing->stages[m_cur_stage].num_icons; i++) {
		Briefing->stages[m_cur_stage].icons[i-1] = Briefing->stages[m_cur_stage].icons[i];
		icon_obj[i-1] = icon_obj[i];
		Objects[icon_obj[i-1]].instance = i - 1;
	}

	Briefing->stages[m_cur_stage].num_icons--;
	if (z >= 0) {
		m_cur_icon = z;
		update_data(0);
	}
}

void briefing_editor_dlg::OnGotoView()
{
	if (m_cur_stage < 0)
		return;

	view_pos = Briefing->stages[m_cur_stage].camera_pos;
	view_orient = Briefing->stages[m_cur_stage].camera_orient;
	Update_window = 1;
}

void briefing_editor_dlg::OnSaveView()
{
	if (m_cur_stage < 0)
		return;

	Briefing->stages[m_cur_stage].camera_pos = view_pos;
	Briefing->stages[m_cur_stage].camera_orient = view_orient;
}

void briefing_editor_dlg::OnSelchangeIconImage()
{
	update_data(1);
}

void briefing_editor_dlg::OnSelchangeTeam()
{
	update_data(1);
}

void briefing_editor_dlg::OnSelchangeShipType()
{
	update_data(1);
}

int briefing_editor_dlg::check_mouse_hit(int x, int y)
{
	int i;
	brief_icon *ptr;

	if (m_cur_stage < 0)
		return -1;

	for (i=0; i<Briefing->stages[m_cur_stage].num_icons; i++) {
		ptr = &Briefing->stages[m_cur_stage].icons[i];
		if ((x > ptr->x) && (x < ptr->x + ptr->w) && (y > ptr->y) && (y < ptr->y + ptr->h))	{
			return icon_obj[i];
		}
	}

	return -1;
}

void briefing_editor_dlg::OnPropagateIcons() 
{
	object *ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_POINT) && (ptr->flags & OF_MARKED)) {
			propagate_icon(ptr->instance);
		}

		ptr = GET_NEXT(ptr);
	}
}

void briefing_editor_dlg::propagate_icon(int num)
{
	int i, s;

	for (s=m_cur_stage+1; s<Briefing->num_stages; s++) {
		i = Briefing->stages[s].num_icons;
		if (i >= MAX_STAGE_ICONS)
			continue;

		if (find_icon(Briefing->stages[m_cur_stage].icons[num].id, s) >= 0)
			continue;  // don't change if icon exists here already.

		Briefing->stages[s].icons[i] = Briefing->stages[m_cur_stage].icons[num];
		Briefing->stages[s].num_icons++;
	}
}

int briefing_editor_dlg::find_icon(int id, int stage)
{
	int i;

	if (id >= 0)
		for (i=0; i<Briefing->stages[stage].num_icons; i++)
			if (Briefing->stages[stage].icons[i].id == id)
				return i;

	return -1;
}

void briefing_editor_dlg::reset_icon_loop(int stage)
{
	stage_loop = stage + 1;
	icon_loop = -1;
}

int briefing_editor_dlg::get_next_icon(int id)
{
	while (1) {
		icon_loop++;
		if (icon_loop >= Briefing->stages[stage_loop].num_icons) {
			stage_loop++;
			if (stage_loop > Briefing->num_stages)
				return 0;

			icon_loop = -1;
			continue;
		}

		iconp = &Briefing->stages[stage_loop].icons[icon_loop];
		if ((id >= 0) && (iconp->id == id))
			return 1;
	}
}

BOOL briefing_editor_dlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id;

	// deal with figuring out menu stuff
	id = LOWORD(wParam);
	if ( (id >= ID_TEAM_1) && (id < ID_TEAM_3) ) {
		m_current_briefing = id - ID_TEAM_1;

		// put user back at first stage for this team (or no current stage is there are none).
		Briefing = &Briefings[m_current_briefing];
		if ( Briefing->num_stages > 0 )
			m_cur_stage = 0;
		else
			m_cur_stage = -1;

		update_data(1);
		OnGotoView();
		return 1;
	}

	return CDialog::OnCommand(wParam, lParam);
}

void briefing_editor_dlg::OnLines() 
{
	if (m_lines.GetCheck() == 1)
		m_lines.SetCheck(0);
	else
		m_lines.SetCheck(1);

	update_data(1);
}

void briefing_editor_dlg::OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_tree.right_clicked();	
	*pResult = 0;
}

void briefing_editor_dlg::OnBeginlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (m_tree.edit_label(pTVDispInfo->item.hItem) == 1)	{
		*pResult = 0;
		modified = 1;

	} else
		*pResult = 1;
}

void briefing_editor_dlg::OnEndlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_tree.end_label_edit(pTVDispInfo->item);
}

BOOL briefing_editor_dlg::DestroyWindow() 
{
	m_play_bm.DeleteObject();
	audiostream_close_file(m_voice_id, 0);
	return CDialog::DestroyWindow();
}

void briefing_editor_dlg::OnPlay() 
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

void briefing_editor_dlg::OnCopyView() 
{
	// TODO: Add your control notification handler code here

	m_copy_view_set = 1;
	m_copy_view_pos = view_pos;
	m_copy_view_orient = view_orient;
}

void briefing_editor_dlg::OnPasteView() 
{
	// TODO: Add your control notification handler code here
	if (m_cur_stage < 0)
		return;

	if (m_copy_view_set == 0) {
		MessageBox("No view set", "Unable to copy view");
	} else {
		Briefing->stages[m_cur_stage].camera_pos = m_copy_view_pos;
		Briefing->stages[m_cur_stage].camera_orient = m_copy_view_orient;
		
		update_data(1);
		OnGotoView();
	}
}

void briefing_editor_dlg::OnFlipIcon() 
{
	update_data(1);	
}

void briefing_editor_dlg::OnWingIcon()
{
	update_data(1);
}
