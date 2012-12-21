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
#include "FREDDoc.h"
#include "EventEditor.h"
#include "FREDView.h"
#include "Management.h"
#include "Sexp_tree.h"
#include "mission/missionmessage.h"
#include "cfile/cfile.h"
#include "sound/audiostr.h"
#include "localization/localize.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

event_editor *Event_editor_dlg = NULL; // global reference needed by event tree class

/////////////////////////////////////////////////////////////////////////////
// event_editor dialog

event_editor::event_editor(CWnd* pParent /*=NULL*/)
	: CDialog(event_editor::IDD, pParent)
{
	//{{AFX_DATA_INIT(event_editor)
	m_repeat_count = 0;
	m_trigger_count = 0;
	m_interval = 0;
	m_event_score = 0;
	m_chain_delay = 0;
	m_chained = FALSE;
	m_obj_text = _T("");
	m_obj_key_text = _T("");
	m_avi_filename = _T("");
	m_message_name = _T("");
	m_message_text = _T("");
	m_persona = -1;
	m_wave_filename = _T("");
	m_cur_msg = -1;
	m_team = -1;
	m_message_team = -1;
	m_last_message_node = -1;
	//}}AFX_DATA_INIT
	m_event_tree.m_mode = MODE_EVENTS;
	m_num_events = 0;
	m_event_tree.link_modified(&modified);
	modified = 0;
	select_sexp_node = -1;
	m_wave_id = -1;
	m_log_true = 0;
	m_log_false = 0;
	m_log_always_true = 0;
	m_log_always_false = 0;
	m_log_1st_repeat = 0;
	m_log_last_repeat = 0;
	m_log_1st_trigger = 0;
	m_log_last_trigger = 0;
}

void event_editor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(event_editor)
	DDX_Control(pDX, IDC_EVENT_TREE, m_event_tree);
	DDX_Text(pDX, IDC_REPEAT_COUNT, m_repeat_count);
	DDX_Text(pDX, IDC_TRIGGER_COUNT, m_trigger_count);
	DDX_Text(pDX, IDC_INTERVAL_TIME, m_interval);
	DDX_Text(pDX, IDC_EVENT_SCORE, m_event_score);
	DDX_Text(pDX, IDC_CHAIN_DELAY, m_chain_delay);
	DDX_Check(pDX, IDC_CHAINED, m_chained);
	DDX_Text(pDX, IDC_OBJ_TEXT, m_obj_text);
	DDX_Text(pDX, IDC_OBJ_KEY_TEXT, m_obj_key_text);
	DDX_CBString(pDX, IDC_AVI_FILENAME, m_avi_filename);
	DDX_Text(pDX, IDC_MESSAGE_NAME, m_message_name);
	DDX_Text(pDX, IDC_MESSAGE_TEXT, m_message_text);
	DDX_CBIndex(pDX, IDC_PERSONA_NAME, m_persona);
	DDX_CBString(pDX, IDC_WAVE_FILENAME, m_wave_filename);
	DDX_LBIndex(pDX, IDC_MESSAGE_LIST, m_cur_msg);
	DDX_Check(pDX, IDC_MISSION_LOG_TRUE, m_log_true);
	DDX_Check(pDX, IDC_MISSION_LOG_FALSE, m_log_false);
	DDX_Check(pDX, IDC_MISSION_LOG_ALWAYS_TRUE, m_log_always_true);
	DDX_Check(pDX, IDC_MISSION_LOG_ALWAYS_FALSE, m_log_always_false);
	DDX_Check(pDX, IDC_MISSION_LOG_1ST_REPEAT, m_log_1st_repeat);
	DDX_Check(pDX, IDC_MISSION_LOG_LAST_REPEAT, m_log_last_repeat);
	DDX_Check(pDX, IDC_MISSION_LOG_1ST_TRIGGER, m_log_1st_trigger);
	DDX_Check(pDX, IDC_MISSION_LOG_LAST_TRIGGER, m_log_last_trigger);


	// m_team == -1 maps to 2
	if(m_team == -1){
		m_team = MAX_TVT_TEAMS;
	}

	DDX_CBIndex(pDX, IDC_EVENT_TEAM, m_team);

	// m_message_team == -1 maps to 2
	if(m_message_team == -1){
		m_message_team = MAX_TVT_TEAMS;
	}
	DDX_CBIndex(pDX, IDC_MESSAGE_TEAM, m_message_team);
	//}}AFX_DATA_MAP

	DDV_MaxChars(pDX, m_obj_text, NAME_LENGTH - 1);
	DDV_MaxChars(pDX, m_obj_key_text, NAME_LENGTH - 1);
	DDV_MaxChars(pDX, m_message_name, NAME_LENGTH - 1);
	DDV_MaxChars(pDX, m_message_text, MESSAGE_LENGTH - 1);
	DDV_MaxChars(pDX, m_avi_filename, MAX_FILENAME_LEN - 1);
	DDV_MaxChars(pDX, m_wave_filename, MAX_FILENAME_LEN - 1);
}

BEGIN_MESSAGE_MAP(event_editor, CDialog)
	//{{AFX_MSG_MAP(event_editor)
	ON_NOTIFY(NM_RCLICK, IDC_EVENT_TREE, OnRclickEventTree)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_EVENT_TREE, OnBeginlabeleditEventTree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_EVENT_TREE, OnEndlabeleditEventTree)
	ON_BN_CLICKED(IDC_BUTTON_NEW_EVENT, OnButtonNewEvent)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(ID_OK, OnOk)
	ON_WM_CLOSE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_EVENT_TREE, OnSelchangedEventTree)
	ON_EN_UPDATE(IDC_REPEAT_COUNT, OnUpdateRepeatCount)
	ON_EN_UPDATE(IDC_TRIGGER_COUNT, OnUpdateTriggerCount)
	ON_BN_CLICKED(IDC_CHAINED, OnChained)
	ON_BN_CLICKED(IDC_INSERT, OnInsert)
	ON_LBN_SELCHANGE(IDC_MESSAGE_LIST, OnSelchangeMessageList)
	ON_BN_CLICKED(IDC_NEW_MSG, OnNewMsg)
	ON_BN_CLICKED(IDC_DELETE_MSG, OnDeleteMsg)
	ON_BN_CLICKED(IDC_BROWSE_AVI, OnBrowseAvi)
	ON_BN_CLICKED(IDC_BROWSE_WAVE, OnBrowseWave)
	ON_CBN_SELCHANGE(IDC_WAVE_FILENAME, OnSelchangeWaveFilename)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_UPDATE, OnUpdate)
	ON_BN_CLICKED(ID_CANCEL, On_Cancel)
	ON_CBN_SELCHANGE(IDC_EVENT_TEAM, OnSelchangeTeam)
	ON_CBN_SELCHANGE(IDC_MESSAGE_TEAM, OnSelchangeMessageTeam)
	ON_LBN_DBLCLK(IDC_MESSAGE_LIST, OnDblclkMessageList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// event_editor message handlers

void maybe_add_head(CComboBox *box, char* name)
{
	if (box->FindStringExact(-1, name) == CB_ERR) {
		box->AddString(name);
	}
}

BOOL event_editor::OnInitDialog() 
{
	int i, adjust = 0;
	BOOL r = TRUE;
	CListBox *list;
	CComboBox *box;
	MMessage msg; 

	CDialog::OnInitDialog();  // let the base class do the default work
	m_play_bm.LoadBitmap(IDB_PLAY);
	((CButton *) GetDlgItem(IDC_PLAY)) -> SetBitmap(m_play_bm);

	if (!Show_sexp_help)
		adjust = -SEXP_HELP_BOX_SIZE;

	theApp.init_window(&Events_wnd_data, this, adjust);
	m_event_tree.setup((CEdit *) GetDlgItem(IDC_HELP_BOX));
	load_tree();
	create_tree();
	if (m_num_events >= MAX_MISSION_EVENTS){
		GetDlgItem(IDC_BUTTON_NEW_EVENT)->EnableWindow(FALSE);
	}

	update_cur_event();
	i = m_event_tree.select_sexp_node;
	if (i != -1) {
		GetDlgItem(IDC_EVENT_TREE) -> SetFocus();
		m_event_tree.hilite_item(i);
		r = FALSE;
	}

	m_num_messages = Num_messages - Num_builtin_messages;
	for (i=0; i<m_num_messages; i++) {
		msg = Messages[i + Num_builtin_messages];
		m_messages.push_back(msg); 
		if (m_messages[i].avi_info.name){
			m_messages[i].avi_info.name = strdup(m_messages[i].avi_info.name);
		}
		if (m_messages[i].wave_info.name){
			m_messages[i].wave_info.name = strdup(m_messages[i].wave_info.name);
		}
	}

	((CEdit *) GetDlgItem(IDC_MESSAGE_NAME))->LimitText(NAME_LENGTH - 1);
	((CEdit *) GetDlgItem(IDC_MESSAGE_TEXT))->LimitText(MESSAGE_LENGTH - 1);
	((CComboBox *) GetDlgItem(IDC_AVI_FILENAME))->LimitText(MAX_FILENAME_LEN - 1);
	((CComboBox *) GetDlgItem(IDC_WAVE_FILENAME))->LimitText(MAX_FILENAME_LEN - 1);

	list = (CListBox *) GetDlgItem(IDC_MESSAGE_LIST);
	list->ResetContent();
	for (i=0; i<m_num_messages; i++) {
		list->AddString(m_messages[i].name);
	}

	box = (CComboBox *) GetDlgItem(IDC_AVI_FILENAME);
	box->ResetContent();
	box->AddString("<None>");
	for (i=0; i<Num_messages; i++) {
		if (Messages[i].avi_info.name) {
			maybe_add_head(box, Messages[i].avi_info.name);
		}
	}

	// add new heads, if not already in
	maybe_add_head(box, "Head-TP2");
	maybe_add_head(box, "Head-VC2");
	maybe_add_head(box, "Head-TP4");
	maybe_add_head(box, "Head-TP5");
	maybe_add_head(box, "Head-TP6");
	maybe_add_head(box, "Head-TP7");
	maybe_add_head(box, "Head-TP8");
	maybe_add_head(box, "Head-VP2");
	maybe_add_head(box, "Head-VP2");
	maybe_add_head(box, "Head-CM2");
	maybe_add_head(box, "Head-CM3");
	maybe_add_head(box, "Head-CM4");
	maybe_add_head(box, "Head-CM5");
	maybe_add_head(box, "Head-BSH");

/*
	box->AddString("Head-VC");  // force it in, since Sandeep wants it and it's not used in built-in messages
	box->AddString("Head-VC2");

	// add terran pilot heads
	box->AddString("Head-TP4");
	box->AddString("Head-TP5");
	box->AddString("Head-TP6");
	box->AddString("Head-TP7");
	box->AddString("Head-TP8");

	// add vasudan pilot heads
	box->AddString("Head-VP2");

	// BSH and CM2
	box->AddString("Head-CM2");
	box->AddString("Head-BSH");
	*/

	box = (CComboBox *) GetDlgItem(IDC_WAVE_FILENAME);
	box->ResetContent();
	box->AddString("<None>");
	for (i=0; i<Num_messages; i++){
		if (Messages[i].wave_info.name){
			if (box->FindStringExact(i, Messages[i].wave_info.name) == CB_ERR){
				box->AddString(Messages[i].wave_info.name);
			}
		}
	}

	// add the persona names into the combo box
	box = (CComboBox *) GetDlgItem(IDC_PERSONA_NAME);
	box->ResetContent();
	box->AddString("<None>");
	for (i = 0; i < Num_personas; i++ ){
		box->AddString( Personas[i].name );
	}

	// set the first message to be the first non-builtin message (if it exists)
	if ( Num_messages > Num_builtin_messages ){
		m_cur_msg = 0;
	} else {
		m_cur_msg = -1;
	}

	update_cur_message();
	return r;
}

void event_editor::load_tree()
{
	int i;

	m_event_tree.select_sexp_node = select_sexp_node;
	select_sexp_node = -1;

	m_event_tree.clear_tree();
	m_num_events = Num_mission_events;
	for (i=0; i<m_num_events; i++) {
		m_events[i] = Mission_events[i];
		if (Mission_events[i].objective_text){
			m_events[i].objective_text = strdup(Mission_events[i].objective_text);
		} else {
			m_events[i].objective_text = NULL;
		}

		if (Mission_events[i].objective_key_text){
			m_events[i].objective_key_text = strdup(Mission_events[i].objective_key_text);
		} else {
			m_events[i].objective_key_text = NULL;
		}
		
		m_sig[i] = i;
		if (!(*m_events[i].name)){
			strcpy_s(m_events[i].name, "<Unnamed>");
		}

		m_events[i].formula = m_event_tree.load_sub_tree(Mission_events[i].formula, false, "do-nothing");

		// we must check for the case of the repeat count being 0.  This would happen if the repeat
		// count is not specified in a mission
		if ( m_events[i].repeat_count <= 0 ){
			m_events[i].repeat_count = 1;
		}
	}

	m_event_tree.post_load();
	cur_event = -1;
}

void event_editor::create_tree()
{
	int i;
	HTREEITEM h;

	m_event_tree.DeleteAllItems();
	for (i=0; i<m_num_events; i++) {

		// set the proper bitmap
		int image;
		if (m_events[i].chain_delay >= 0) {
			image = BITMAP_CHAIN;
			if (m_events[i].objective_text) {
				image = BITMAP_CHAIN_DIRECTIVE;
			}
		} else {
			image = BITMAP_ROOT;
			if (m_events[i].objective_text) {
				image = BITMAP_ROOT_DIRECTIVE;
			}
		}

		h = m_event_tree.insert(m_events[i].name, image, image);

		m_event_tree.SetItemData(h, m_events[i].formula);
		m_event_tree.add_sub_tree(m_events[i].formula, h);
	}

	cur_event = -1;
}

void event_editor::OnRclickEventTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	save();
	m_event_tree.right_clicked(MODE_EVENTS);
	*pResult = 0;
}

void event_editor::OnBeginlabeleditEventTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	CEdit *edit;

	if (m_event_tree.edit_label(pTVDispInfo->item.hItem) == 1)	{
		*pResult = 0;
		modified = 1;
		edit = m_event_tree.GetEditControl();
		Assert(edit);
		edit->SetLimitText(NAME_LENGTH - 1);

	} else
		*pResult = 1;
}

void event_editor::OnEndlabeleditEventTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_event_tree.end_label_edit(pTVDispInfo->item);
}

// This is needed as a HACK around default MFC standard
// It is not required, but overrides default MFC and links no errors without.
void event_editor::OnOK()
{
	HWND h;
	CWnd *w;

	save();
	w = GetFocus();
	if (w) {
		h = w->m_hWnd;
		GetDlgItem(IDC_EVENT_TREE)->SetFocus();
		::SetFocus(h);
	}
	((CListBox *) GetDlgItem(IDC_MESSAGE_LIST))->SetCurSel(m_cur_msg);
}

int event_editor::query_modified()
{
	int i;
	char *ptr, buf[MESSAGE_LENGTH];

	UpdateData(TRUE);
	if (modified)
		return 1;

	if (Num_mission_events != m_num_events)
		return 1;

	for (i=0; i<m_num_events; i++) {
		if (stricmp(m_events[i].name, Mission_events[i].name))
			return 1;
		if (m_events[i].repeat_count != Mission_events[i].repeat_count)
			return 1;
		if (m_events[i].trigger_count != Mission_events[i].trigger_count)
			return 1;
		if (m_events[i].interval != Mission_events[i].interval)
			return 1;
		if (m_events[i].score != Mission_events[i].score)
			return 1;
		if (m_events[i].chain_delay != Mission_events[i].chain_delay)
			return 1;
		if (advanced_stricmp(m_events[i].objective_text, Mission_events[i].objective_text))
			return 1;
		if (advanced_stricmp(m_events[i].objective_key_text, Mission_events[i].objective_key_text))
			return 1;
		if (m_events[i].mission_log_flags != Mission_events[i].mission_log_flags)
			return 1;
	}

	if (m_cur_msg < 0)
		return 0;

	if (m_num_messages != Num_messages)
		return 1;

	ptr = (char *) (LPCTSTR) m_message_name;
	for (i=0; i<Num_builtin_messages; i++)
		if (!stricmp(ptr, Messages[i].name))
			return 1;

	for (i=0; i<m_num_messages; i++) {

		if ((i != m_cur_msg) && (!stricmp(ptr, m_messages[m_cur_msg].name)))
			return 1;
	}

	if (stricmp(ptr, m_messages[m_cur_msg].name))
		return 1;  // name is different and allowed to update

	string_copy(buf, m_message_text, MESSAGE_LENGTH - 1);
	if (stricmp(buf, m_messages[m_cur_msg].message))
		return 1;

	ptr = (char *) (LPCTSTR) m_avi_filename;
	if (advanced_stricmp(ptr, m_messages[m_cur_msg].avi_info.name))
		return 1;

	ptr = (char *) (LPCTSTR) m_wave_filename;
	if (advanced_stricmp(ptr, m_messages[m_cur_msg].wave_info.name))
		return 1;

	// check to see if persona changed.  use -1 since we stuck a "None" for persona
	// at the beginning of the list.
	if ( (m_persona - 1 ) != m_messages[m_cur_msg].persona_index )
		return 1;

	return 0;
}

void event_editor::OnOk()
{
	char buf[256], names[2][MAX_MISSION_EVENTS][NAME_LENGTH];
	int i, count;

	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	save();
	if (query_modified())
		set_modified();

	for (i=0; i<Num_mission_events; i++) {
		free_sexp2(Mission_events[i].formula);
		if (Mission_events[i].objective_text)
			free(Mission_events[i].objective_text);
		if (Mission_events[i].objective_key_text)
			free(Mission_events[i].objective_key_text);
	}

	count = 0;
	for (i=0; i<Num_mission_events; i++)
		Mission_events[i].result = 0;  // use this as a processed flag
	
	// rename all sexp references to old events
	for (i=0; i<m_num_events; i++)
		if (m_sig[i] >= 0) {
			strcpy_s(names[0][count], Mission_events[m_sig[i]].name);
			strcpy_s(names[1][count], m_events[i].name);
			count++;
			Mission_events[m_sig[i]].result = 1;
		}

	// invalidate all sexp references to deleted events.
	for (i=0; i<Num_mission_events; i++)
		if (!Mission_events[i].result) {
			sprintf(buf, "<%s>", Mission_events[i].name);
			strcpy(buf + NAME_LENGTH - 2, ">");  // force it to be not too long
			strcpy_s(names[0][count], Mission_events[i].name);
			strcpy_s(names[1][count], buf);
			count++;
		}

	Num_mission_events = m_num_events;
	for (i=0; i<m_num_events; i++) {
		Mission_events[i] = m_events[i];
		Mission_events[i].formula = m_event_tree.save_tree(m_events[i].formula);
		Mission_events[i].objective_text = m_events[i].objective_text;
		Mission_events[i].objective_key_text = m_events[i].objective_key_text;
		Mission_events[i].mission_log_flags = m_events[i].mission_log_flags;
	}

	// now update all sexp references
	while (count--)
		update_sexp_references(names[0][count], names[1][count], OPF_EVENT_NAME);

	for (i=Num_builtin_messages; i<Num_messages; i++) {
		if (Messages[i].avi_info.name)
			free(Messages[i].avi_info.name);

		if (Messages[i].wave_info.name)
			free(Messages[i].wave_info.name);
	}

	Num_messages = m_num_messages + Num_builtin_messages;
	for (i=0; i<m_num_messages; i++)
		Messages[i + Num_builtin_messages] = m_messages[i];

	theApp.record_window_data(&Events_wnd_data, this);
	delete Event_editor_dlg;
	Event_editor_dlg = NULL;
}

// load controls with structure data
void event_editor::update_cur_message()
{
	int enable = TRUE;

	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	if (m_cur_msg < 0) {
		enable = FALSE;
		m_message_name = _T("");
		m_message_text = _T("");
		m_avi_filename = _T("");
		m_wave_filename = _T("");
		m_persona = 0;
		m_message_team = -1;
	} else {
		m_message_name = m_messages[m_cur_msg].name;
		m_message_text = m_messages[m_cur_msg].message;
		if (m_messages[m_cur_msg].avi_info.name){
			m_avi_filename = _T(m_messages[m_cur_msg].avi_info.name);
		} else {
			m_avi_filename = _T("<None>");
		}

		if (m_messages[m_cur_msg].wave_info.name){
			m_wave_filename = _T(m_messages[m_cur_msg].wave_info.name);
		} else {
			m_wave_filename = _T("<None>");
		}

		// add persona id
		if ( m_messages[m_cur_msg].persona_index != -1 ){
			m_persona = m_messages[m_cur_msg].persona_index + 1;  // add one for the "none" at the beginning of the list
		} else {
			m_persona = 0;
		}

		if(m_messages[m_cur_msg].multi_team >= MAX_TVT_TEAMS){
			m_message_team = -1;
			m_messages[m_cur_msg].multi_team = -1;
		} else {
			m_message_team = m_messages[m_cur_msg].multi_team;
		}
/*
		m_event_num = find_event();
		if (m_event_num < 0) {
			node = -1;
			m_sender = m_priority = 0;

		} else
			node = CADR(Mission_events[m_event_num].formula);
*/	}

	GetDlgItem(IDC_MESSAGE_NAME)->EnableWindow(enable);
	GetDlgItem(IDC_MESSAGE_TEXT)->EnableWindow(enable);
	GetDlgItem(IDC_AVI_FILENAME)->EnableWindow(enable);
	GetDlgItem(IDC_BROWSE_AVI)->EnableWindow(enable);
	GetDlgItem(IDC_BROWSE_WAVE)->EnableWindow(enable);
	GetDlgItem(IDC_WAVE_FILENAME)->EnableWindow(enable);
	GetDlgItem(IDC_DELETE_MSG)->EnableWindow(enable);
	GetDlgItem(IDC_PERSONA_NAME)->EnableWindow(enable);
	GetDlgItem(IDC_MESSAGE_TEAM)->EnableWindow(enable);
	UpdateData(FALSE);
}

int event_editor::handler(int code, int node, char *str)
{
	int i, index;

	switch (code) {
		case ROOT_DELETED:
			for (i=0; i<m_num_events; i++)
				if (m_events[i].formula == node)
					break;

			Assert(i < m_num_events);
			index = i;
			while (i < m_num_events - 1) {
				m_events[i] = m_events[i + 1];
				m_sig[i] = m_sig[i + 1];
				i++;
			}

			m_num_events--;
			GetDlgItem(IDC_BUTTON_NEW_EVENT)->EnableWindow(TRUE);

			cur_event = index;
			update_cur_event();

			return node;

		case ROOT_RENAMED:
			for (i=0; i<m_num_events; i++)
				if (m_events[i].formula == node)
					break;

			Assert(i < m_num_events);
			Assert(strlen(str) < NAME_LENGTH);
			strcpy_s(m_events[i].name, str);
			return node;

		default:
			Int3();
	}

	return -1;
}

void event_editor::OnButtonNewEvent() 
{
	if (m_num_events == MAX_MISSION_EVENTS) {
		MessageBox("You have reached the limit on mission events.\n"
			"Can't add any more.");
		return;
	}

	reset_event(m_num_events++, TVI_LAST);
}

void event_editor::OnInsert() 
{
	int i;

	if (m_num_events == MAX_MISSION_EVENTS) {
		MessageBox("You have reached the limit on mission events.\n"
			"Can't add any more.");
		return;
	}

	if(cur_event < 0 || m_num_events == 0)
	{
		//There are no events yet, so just create one
		reset_event(m_num_events++, TVI_LAST);
	}
	else
	{
		for (i=m_num_events; i>cur_event; i--) {
			m_events[i] = m_events[i - 1];
			m_sig[i] = m_sig[i - 1];
		}

		if (cur_event){
			reset_event(cur_event, get_event_handle(cur_event - 1));
		} else {
			reset_event(cur_event, TVI_FIRST);
		}

		m_num_events++;
	}
}

HTREEITEM event_editor::get_event_handle(int num)
{
	HTREEITEM h;

	h = m_event_tree.GetRootItem();
	while (h) {
		if ((int) m_event_tree.GetItemData(h) == m_events[num].formula){
			return h;
		}

		h = m_event_tree.GetNextSiblingItem(h);
	}

	return 0;
}

void event_editor::reset_event(int num, HTREEITEM after)
{
	int index;
	HTREEITEM h;

	strcpy_s(m_events[num].name, "Event name");
	h = m_event_tree.insert(m_events[num].name, BITMAP_ROOT, BITMAP_ROOT, TVI_ROOT, after);

	m_events[num].repeat_count = 1;
	m_events[num].trigger_count = 1;
	m_events[num].interval = 1;
	m_events[num].score = 0;
	m_events[num].chain_delay = -1;
	m_events[num].objective_text = NULL;
	m_events[num].objective_key_text = NULL;
	m_events[num].team = -1;
	m_events[num].mission_log_flags = 0;
	m_sig[num] = -1;

	m_event_tree.item_index = -1;
	m_event_tree.add_operator("when", h);
	index = m_events[num].formula = m_event_tree.item_index;
	m_event_tree.SetItemData(h, index);
	m_event_tree.add_operator("true");
	m_event_tree.item_index = index;
	m_event_tree.add_operator("do-nothing");

	m_event_tree.SelectItem(h);
//	GetDlgItem(IDC_CHAIN_DELAY) -> EnableWindow(FALSE);
	if (num >= MAX_MISSION_EVENTS){
		GetDlgItem(IDC_BUTTON_NEW_EVENT)->EnableWindow(FALSE);
	}
}

void event_editor::OnDelete() 
{
	HTREEITEM h;

	// call update_cur_event to clean up local class variables so that we can correctly
	// set up the newly selected item.
	cur_event = -1;
	update_cur_event();

	h = m_event_tree.GetSelectedItem();
	if (h) {
		while (m_event_tree.GetParentItem(h))
			h = m_event_tree.GetParentItem(h);

		m_event_tree.setup_selected(h);
		m_event_tree.OnCommand(ID_DELETE, 0);
	}
}

// this is called when you hit the escape key..
void event_editor::OnCancel()
{
}

// this is called the clicking the ID_CANCEL button
void event_editor::On_Cancel()
{
	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	theApp.record_window_data(&Events_wnd_data, this);
	delete Event_editor_dlg;
	Event_editor_dlg = NULL;
}

void event_editor::OnClose() 
{
	int z;

	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	if (query_modified()) {
		z = MessageBox("Do you want to keep your changes?", "Close", MB_ICONQUESTION | MB_YESNOCANCEL);
		if (z == IDCANCEL){
			return;
		}

		if (z == IDYES) {
			OnOk();
			return;
		}
	}
	
	theApp.record_window_data(&Events_wnd_data, this);
	delete Event_editor_dlg;
	Event_editor_dlg = NULL;
}

void event_editor::insert_handler(int old, int node)
{
	int i;

	for (i=0; i<m_num_events; i++){
		if (m_events[i].formula == old){
			break;
		}
	}

	Assert(i < m_num_events);
	m_events[i].formula = node;
	return;
}

void event_editor::save()
{
	int m = m_cur_msg;

	save_event(cur_event);
	save_message(m);
}

void event_editor::save_event(int e)
{
	if (e < 0) {
		return;
	}

	UpdateData(TRUE);
	m_events[e].repeat_count = m_repeat_count;
	m_events[e].trigger_count = m_trigger_count;
	m_events[e].interval = m_interval;
	m_events[e].score = m_event_score;

	// handle chain
	if (m_chained) {
		m_events[e].chain_delay = m_chain_delay;
	} else {
		m_events[e].chain_delay = -1;
	}

	// handle objective text
	if (m_events[e].objective_text) {
		free(m_events[e].objective_text);
	}

	if (m_obj_text.IsEmpty()) {
		m_events[e].objective_text = NULL;
	} else {
		m_events[e].objective_text = strdup(m_obj_text);
	}

	// handle objective key text
	if (m_events[e].objective_key_text) {
		free(m_events[e].objective_key_text);
	}

	if (m_obj_key_text.IsEmpty()) {
		m_events[e].objective_key_text = NULL;
	} else {
		m_events[e].objective_key_text = strdup(m_obj_key_text);
	}

	// update bitmap
	int bitmap;
	if (m_chained) {
		if (m_obj_text.IsEmpty()) {
			bitmap = BITMAP_CHAIN;
		} else {
			bitmap = BITMAP_CHAIN_DIRECTIVE;
		}
	} else {
		// not chained
		if (m_obj_text.IsEmpty()) {
			bitmap = BITMAP_ROOT;
		} else {
			bitmap = BITMAP_ROOT_DIRECTIVE;
		}
	}

	// handle event log flags
	m_events[e].mission_log_flags = 0;
	if (m_log_true) 
		m_events[e].mission_log_flags |= MLF_SEXP_TRUE;
	if (m_log_false) 
		m_events[e].mission_log_flags |= MLF_SEXP_FALSE;
	if (m_log_always_true) 
		m_events[e].mission_log_flags |= MLF_SEXP_KNOWN_TRUE;
	if (m_log_always_false) 
		m_events[e].mission_log_flags |= MLF_SEXP_KNOWN_FALSE;
	if (m_log_1st_repeat) 
		m_events[e].mission_log_flags |= MLF_FIRST_REPEAT_ONLY;
	if (m_log_last_repeat) 
		m_events[e].mission_log_flags |= MLF_LAST_REPEAT_ONLY;
	if (m_log_1st_trigger) 
		m_events[e].mission_log_flags |= MLF_FIRST_TRIGGER_ONLY;
	if (m_log_last_trigger) 
		m_events[e].mission_log_flags |= MLF_LAST_TRIGGER_ONLY;


	// Search for item to update
	HTREEITEM h = m_event_tree.GetRootItem();
	while (h) {
		if ((int) m_event_tree.GetItemData(h) == m_events[e].formula) {
			m_event_tree.SetItemImage(h, bitmap, bitmap);
			return;
		}

		h = m_event_tree.GetNextSiblingItem(h);
	}

}

// this function deals with the left click on an event in the tree view.  We get into this
// function so that we may update the other data on the screen (i.e repeat count and interval
// count)
void event_editor::OnSelchangedEventTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int i, z;
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM h, h2;

	// before we do anything, we must check and save off any data from the current event (i.e.
	// the repeat count and interval count)
	save();
	h = pNMTreeView->itemNew.hItem;
	if (!h){
		return;
	}

	m_event_tree.update_help(h);
	while ((h2 = m_event_tree.GetParentItem(h))>0){
		h = h2;
	}

	z = m_event_tree.GetItemData(h);
	for (i=0; i<m_num_events; i++){
		if (m_events[i].formula == z){
			break;
		}
	}

	Assert(i < m_num_events);
	cur_event = i;
	update_cur_event();
	
	*pResult = 0;
}

void event_editor::update_cur_event()
{
	if (cur_event < 0) {
		m_repeat_count = 1;
		m_trigger_count = 1;
		m_interval = 1;
		m_chain_delay = 0;
		m_team = -1;
		m_obj_text.Empty();
		m_obj_key_text.Empty();
		GetDlgItem(IDC_INTERVAL_TIME) -> EnableWindow(FALSE);
		GetDlgItem(IDC_REPEAT_COUNT) -> EnableWindow(FALSE);
		GetDlgItem(IDC_TRIGGER_COUNT) -> EnableWindow(FALSE);
		GetDlgItem(IDC_EVENT_SCORE) -> EnableWindow(FALSE);
		GetDlgItem(IDC_CHAINED) -> EnableWindow(FALSE);
		GetDlgItem(IDC_CHAIN_DELAY) -> EnableWindow(FALSE);
		GetDlgItem(IDC_OBJ_TEXT) -> EnableWindow(FALSE);
		GetDlgItem(IDC_OBJ_KEY_TEXT) -> EnableWindow(FALSE);
		GetDlgItem(IDC_EVENT_TEAM)->EnableWindow(FALSE);

		UpdateData(FALSE);
		return;
	}

	m_team = m_events[cur_event].team;

	m_repeat_count = m_events[cur_event].repeat_count;
	m_trigger_count = m_events[cur_event].trigger_count;
	m_interval = m_events[cur_event].interval;
	m_event_score = m_events[cur_event].score;
	if (m_events[cur_event].chain_delay >= 0) {
		m_chained = TRUE;
		m_chain_delay = m_events[cur_event].chain_delay;
		GetDlgItem(IDC_CHAIN_DELAY) -> EnableWindow(TRUE);

	} else {
		m_chained = FALSE;
		m_chain_delay = 0;
		GetDlgItem(IDC_CHAIN_DELAY) -> EnableWindow(FALSE);
	}

	if (m_events[cur_event].objective_text){
		m_obj_text = m_events[cur_event].objective_text;
	} else {
		m_obj_text.Empty();
	}

	if (m_events[cur_event].objective_key_text){
		m_obj_key_text = m_events[cur_event].objective_key_text;
	} else {
		m_obj_key_text.Empty();
	}

	GetDlgItem(IDC_REPEAT_COUNT)->EnableWindow(TRUE);
	GetDlgItem(IDC_TRIGGER_COUNT)->EnableWindow(TRUE);

	if (( m_repeat_count <= 1) && (m_trigger_count <= 1)) {
		m_interval = 1;
		GetDlgItem(IDC_INTERVAL_TIME) -> EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_INTERVAL_TIME) -> EnableWindow(TRUE);
	}

	GetDlgItem(IDC_EVENT_SCORE) -> EnableWindow(TRUE);
	GetDlgItem(IDC_CHAINED) -> EnableWindow(TRUE);
	GetDlgItem(IDC_OBJ_TEXT) -> EnableWindow(TRUE);
	GetDlgItem(IDC_OBJ_KEY_TEXT) -> EnableWindow(TRUE);
	GetDlgItem(IDC_EVENT_TEAM)->EnableWindow(FALSE);
	if ( The_mission.game_type & MISSION_TYPE_MULTI_TEAMS ){
		GetDlgItem(IDC_EVENT_TEAM)->EnableWindow(TRUE);
	}

	// handle event log flags
	if (m_events[cur_event].mission_log_flags & MLF_SEXP_TRUE) {
		m_log_true  = TRUE;
	}else {
		m_log_true  = FALSE;
	}
	if (m_events[cur_event].mission_log_flags & MLF_SEXP_FALSE) {
		m_log_false  = TRUE;
	}else {
		m_log_false  = FALSE;
	}
	if (m_events[cur_event].mission_log_flags & MLF_SEXP_KNOWN_TRUE) {
		m_log_always_true  = TRUE;
	}else {
		m_log_always_true  = FALSE;
	}
	if (m_events[cur_event].mission_log_flags & MLF_SEXP_KNOWN_FALSE) {
		m_log_always_false  = TRUE;
	}else {
		m_log_always_false  = FALSE;
	}
	if (m_events[cur_event].mission_log_flags & MLF_FIRST_REPEAT_ONLY) {
		m_log_1st_repeat  = TRUE;
	}else {
		m_log_1st_repeat  = FALSE;
	}
	if (m_events[cur_event].mission_log_flags & MLF_LAST_REPEAT_ONLY) {
		m_log_last_repeat  = TRUE;
	}else {
		m_log_last_repeat  = FALSE;
	}
	if (m_events[cur_event].mission_log_flags & MLF_FIRST_TRIGGER_ONLY) {
		m_log_1st_trigger  = TRUE;
	}else {
		m_log_1st_trigger  = FALSE;
	}
	if (m_events[cur_event].mission_log_flags & MLF_LAST_TRIGGER_ONLY) {
		m_log_last_trigger  = TRUE;
	}else {
		m_log_last_trigger  = FALSE;
	}

	UpdateData(FALSE);
}

void event_editor::OnUpdateRepeatCount()
{
	char buf[128];
	int count = 128; 
	GetDlgItem(IDC_REPEAT_COUNT)->GetWindowText(buf, count);
	m_repeat_count = atoi(buf);

	if ( ( m_repeat_count <= 1) && (m_trigger_count <= 1) ){
		GetDlgItem(IDC_INTERVAL_TIME)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_INTERVAL_TIME)->EnableWindow(TRUE);
	}
}

void event_editor::OnUpdateTriggerCount()
{
	char buf[128];
	int count = 128;

	GetDlgItem(IDC_TRIGGER_COUNT)->GetWindowText(buf, count);
	m_trigger_count = atoi(buf);

	if ( ( m_repeat_count <= 1) && (m_trigger_count <= 1) ){
		GetDlgItem(IDC_INTERVAL_TIME)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_INTERVAL_TIME)->EnableWindow(TRUE);
	}
}
void event_editor::swap_handler(int node1, int node2)
{
	int index1, index2;
	mission_event m;

	save();
	for (index1=0; index1<m_num_events; index1++){
		if (m_events[index1].formula == node1){
			break;
		}
	}

	Assert(index1 < m_num_events);
	for (index2=0; index2<m_num_events; index2++){
		if (m_events[index2].formula == node2){
			break;
		}
	}

	Assert(index2 < m_num_events);
	m = m_events[index1];
//	m_events[index1] = m_events[index2];
	while (index1 < index2) {
		m_events[index1] = m_events[index1 + 1];
		m_sig[index1] = m_sig[index1 + 1];
		index1++;
	}

	while (index1 > index2 + 1) {
		m_events[index1] = m_events[index1 - 1];
		m_sig[index1] = m_sig[index1 - 1];
		index1--;
	}

	m_events[index1] = m;
	cur_event = index1;
	update_cur_event();
}

void event_editor::OnChained() 
{
	int image;
	HTREEITEM h;

	UpdateData(TRUE);
	if (m_chained) {
		GetDlgItem(IDC_CHAIN_DELAY) -> EnableWindow(TRUE);
		if (m_obj_text.IsEmpty()) {
			image = BITMAP_CHAIN;
		} else {
			image = BITMAP_CHAIN_DIRECTIVE;
		}
	} else {
		GetDlgItem(IDC_CHAIN_DELAY) -> EnableWindow(FALSE);
		if (m_obj_text.IsEmpty()) {
			image = BITMAP_ROOT;
		} else {
			image = BITMAP_ROOT_DIRECTIVE;
		}
	}

	h = m_event_tree.GetRootItem();
	while (h) {
		if ((int) m_event_tree.GetItemData(h) == m_events[cur_event].formula) {
			m_event_tree.SetItemImage(h, image, image);
			return;
		}

		h = m_event_tree.GetNextSiblingItem(h);
	}
}

void event_editor::OnSelchangeMessageList() 
{	
	static int flag = 0;

	if (flag)
		return;
/*
	if (save_message(m_cur_msg)) {
		flag = 1;
		((CListBox *) GetDlgItem(IDC_MESSAGE_LIST)) -> SetCurSel(old);
		m_cur_msg = old;
		flag = 0;
		return;
	}*/

	save();
	update_cur_message();
}

int event_editor::save_message(int num)
{
	char *ptr;
	int i, conflict = 0;
	CListBox *list;

	UpdateData(TRUE);
	if (num >= 0) {
		ptr = (char *) (LPCTSTR) m_message_name;
		for (i=0; i<Num_builtin_messages; i++){
			if (!stricmp(m_message_name, Messages[i].name)) {
				conflict = 1;
				break;
			}
		}

		for (i=0; i<m_num_messages; i++){
			if ((i != num) && (!stricmp(m_message_name, m_messages[i].name))) {
				conflict = 1;
				break;
			}
		}

		if (!conflict) {  // update name if no conflicts, otherwise keep old name
			string_copy(m_messages[num].name, m_message_name, NAME_LENGTH - 1);

			list = (CListBox *) GetDlgItem(IDC_MESSAGE_LIST);
			list->DeleteString(num);
			list->InsertString(num, m_message_name);
		}

		string_copy(m_messages[num].message, m_message_text, MESSAGE_LENGTH - 1);
		lcl_fred_replace_stuff(m_messages[num].message, MESSAGE_LENGTH);
		if (m_messages[num].avi_info.name){
			free(m_messages[num].avi_info.name);
		}

		ptr = (char *) (LPCTSTR) m_avi_filename;
		if ( !ptr || !VALID_FNAME(ptr) ) {
			m_messages[num].avi_info.name = NULL;
		} else {
			m_messages[num].avi_info.name = strdup(ptr);
		}

		if (m_messages[num].wave_info.name){
			free(m_messages[num].wave_info.name);
		}

		ptr = (char *) (LPCTSTR) m_wave_filename;
		if ( !ptr || !VALID_FNAME(ptr) ) {
			m_messages[num].wave_info.name = NULL;
		} else {
			m_messages[num].wave_info.name = strdup(ptr);
		}

		// update the persona to the message.  We subtract 1 for the "None" at the beginning of the combo
		// box list.
		m_messages[num].persona_index = m_persona - 1;

		if(m_message_team >= MAX_TVT_TEAMS){
			m_messages[num].multi_team = -1;
			m_message_team = -1;
		} else {
			m_messages[num].multi_team = m_message_team;
		}

		// possible TODO: auto-update event tree references to this message if we renamed it.
	}

	return 0;
}

void event_editor::OnNewMsg() 
{
	MMessage msg; 

	save();
	strcpy_s(msg.name, "<new message>");
	((CListBox *) GetDlgItem(IDC_MESSAGE_LIST))->AddString("<new message>");

	strcpy_s(msg.message, "<put description here>");
	msg.avi_info.name = NULL;
	msg.wave_info.name = NULL;
	msg.persona_index = -1;
	msg.multi_team = -1;
	m_messages.push_back(msg);
	m_cur_msg = m_num_messages++;

	modified = 1;
	update_cur_message();
}

void event_editor::OnDeleteMsg() 
{
	char buf[256];

	// handle this case somewhat gracefully
	Assert((m_cur_msg >= 0) && (m_cur_msg < m_num_messages));
	if((m_cur_msg < 0) || (m_cur_msg >= m_num_messages)){
		return;
	}

	if (m_messages[m_cur_msg].avi_info.name){
		free(m_messages[m_cur_msg].avi_info.name);
	}
	if (m_messages[m_cur_msg].wave_info.name){
		free(m_messages[m_cur_msg].wave_info.name);
	}

	((CListBox *) GetDlgItem(IDC_MESSAGE_LIST))->DeleteString(m_cur_msg);
	sprintf(buf, "<%s>", m_messages[m_cur_msg].name);
	update_sexp_references(m_messages[m_cur_msg].name, buf, OPF_MESSAGE);
	update_sexp_references(m_messages[m_cur_msg].name, buf, OPF_MESSAGE_OR_STRING);

	m_messages.erase(m_messages.begin() + m_cur_msg); 

	m_num_messages--;
	if (m_cur_msg >= m_num_messages){
		m_cur_msg = m_num_messages - 1;
	}

	GetDlgItem(IDC_NEW_MSG)->EnableWindow(TRUE);
	modified = 1;
	update_cur_message();
}

void event_editor::OnBrowseAvi() 
{
	int z;
	CString name;	

	UpdateData(TRUE);
	if (!stricmp(m_avi_filename, "<None>"))
		m_avi_filename = _T("");

	z = cfile_push_chdir(CF_TYPE_INTERFACE);
	CFileDialog dlg(TRUE, "ani", m_avi_filename, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR,
		"Ani Files (*.ani)|*.ani|Eff Files (*.eff)|*.eff|Avi Files (*.avi)|*.avi|Both (*.ani, *.avi)|*.ani;*.avi||");

	if (dlg.DoModal() == IDOK) {
		m_avi_filename = dlg.GetFileName();
		UpdateData(FALSE);
		modified = 1;
	}

	if (!z)
		cfile_pop_dir();
}

void event_editor::OnBrowseWave() 
{
	int z;
	CString name;

	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	UpdateData(TRUE);
	if (!stricmp(m_wave_filename, "<None>"))
		m_wave_filename = _T("");

	if (The_mission.game_type & MISSION_TYPE_TRAINING)
		z = cfile_push_chdir(CF_TYPE_VOICE_TRAINING);
	else
		z = cfile_push_chdir(CF_TYPE_VOICE_SPECIAL);

	CFileDialog dlg(TRUE, "wav", m_wave_filename, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR,
		"Voice Files (*.ogg, *.wav)|*.ogg;*.wav|Ogg Vorbis Files (*.ogg)|*.ogg|Wave Files (*.wav)|*.wav||");

	if (dlg.DoModal() == IDOK) {
		m_wave_filename = dlg.GetFileName();
		update_persona();
	}

	if (!z){
		cfile_pop_dir();
	}
}

char *event_editor::current_message_name(int i)
{
	if ( (i < 0) || (i >= m_num_messages) ){
		return NULL;
	}

	return m_messages[i].name;
}

char *event_editor::get_message_list_item(int i)
{
	return m_messages[i].name;
}

void event_editor::update_persona()
{
	int i, mask;

	if ((m_wave_filename[0] >= '1') && (m_wave_filename[0] <= '9') && (m_wave_filename[1] == '_') ) {
		i = m_wave_filename[0] - '1';
		if ( (i < Num_personas) && (Personas[i].flags & PERSONA_FLAG_WINGMAN) )	{
			m_persona = i + 1;
			if ((m_persona==1) || (m_persona==2)) 
				m_avi_filename = "HEAD-TP1";
			else if ((m_persona==3) || (m_persona==4))
				m_avi_filename = "HEAD-TP2";
			else if ((m_persona==5))
				m_avi_filename = "HEAD-TP3";
			else if ((m_persona==6))
				m_avi_filename = "HEAD-VP1";
		}
	} else {
		mask = 0;
		if (!strnicmp(m_wave_filename, "S_", 2)) {
			mask = PERSONA_FLAG_SUPPORT;
			m_avi_filename = "HEAD-CM1";
		}
		else if (!strnicmp(m_wave_filename, "L_", 2)) {
			mask = PERSONA_FLAG_LARGE;
			m_avi_filename = "HEAD-CM1";
		}
		else if (!strnicmp(m_wave_filename, "TC_", 3)) {
			mask = PERSONA_FLAG_COMMAND;
			m_avi_filename = "HEAD-CM1";
		}

		for (i=0; i<Num_personas; i++)
			if (Personas[i].flags & mask)
				m_persona = i + 1;
	}
	//GetDlgItem(IDC_ANI_FILENAME)->SetWindowText(m_avi_filename);
	UpdateData(FALSE);
	modified = 1;
}

void event_editor::OnSelchangeWaveFilename() 
{
	int z;
	CComboBox *box;

	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	box = (CComboBox *) GetDlgItem(IDC_WAVE_FILENAME);
	z = box -> GetCurSel();
	UpdateData(TRUE);
	UpdateData(TRUE);

	box -> GetLBText(z, m_wave_filename);
	UpdateData(FALSE);
	update_persona();
}

BOOL event_editor::DestroyWindow() 
{
	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	m_play_bm.DeleteObject();
	return CDialog::DestroyWindow();
}

void event_editor::OnPlay() 
{
	GetDlgItem(IDC_WAVE_FILENAME)->GetWindowText(m_wave_filename);

	if (m_wave_id >= 0) {
		audiostream_close_file(m_wave_id, 0);
		m_wave_id = -1;
		return;
	}

	// we use ASF_EVENTMUSIC here so that it will keep the extension in place
	m_wave_id = audiostream_open((char *)(LPCSTR) m_wave_filename, ASF_EVENTMUSIC);

	if (m_wave_id >= 0) {
		audiostream_play(m_wave_id, 1.0f, 0);
	}
}

void event_editor::OnUpdate() 
{
//	GetDlgItem(IDC_WAVE_FILENAME)->GetWindowText(m_wave_filename);
	UpdateData(TRUE);
	update_persona();
}

// code when the "team" selection in the combo box changes
void event_editor::OnSelchangeTeam() 
{
	if ( cur_event < 0 ){
		return;
	}

	UpdateData(TRUE);

	// If the team isn't valid mark it as such. 
	if((m_team >= MAX_TVT_TEAMS) || (m_team < -1) ){
		m_team = -1;
	}

	m_events[cur_event].team = m_team;
}

// code when the "team" selection in the combo box changes
void event_editor::OnSelchangeMessageTeam() 
{
	if ( m_cur_msg < 0 ){
		return;
	}

	UpdateData(TRUE);

	// If the team isn't valid mark it as such. 
	if((m_message_team>= MAX_TVT_TEAMS) || (m_message_team < -1) ) {
		m_message_team = -1;
	}

	m_messages[m_cur_msg].multi_team = m_message_team;
}

// Cycles among sexp nodes with message text
void event_editor::OnDblclkMessageList() 
{
	CListBox *list = (CListBox*) GetDlgItem(IDC_MESSAGE_LIST);
	int num_messages;
	int message_nodes[MAX_SEARCH_MESSAGE_DEPTH];

	// get current message index and message name
	int cur_index = list->GetCurSel();

	// check if message name is in event tree
	char buffer[256];
	list->GetText(cur_index, buffer);


	num_messages = m_event_tree.find_text(buffer, message_nodes);

	if (num_messages == 0) {
		char message[256];
		sprintf(message, "No events using message '%s'", buffer);
		MessageBox(message);
	} else {
		// find last message_node
		if (m_last_message_node == -1) {
			m_last_message_node = message_nodes[0];
		} else {

			if (num_messages == 1) {
				// only 1 message
				m_last_message_node = message_nodes[0];
			} else {
				// find which message and go to next message
				int found_pos = -1;
				for (int i=0; i<num_messages; i++) {
					if (message_nodes[i] == m_last_message_node) {
						found_pos = i;
						break;
					}
				}

				if (found_pos == -1) {
					// no previous message
					m_last_message_node = message_nodes[0];
				} else if (found_pos == num_messages-1) {
					// cycle back to start
					m_last_message_node = message_nodes[0];
				} else {
					// go to next
					m_last_message_node = message_nodes[found_pos+1];
				}
			}
		}

		// highlight next
		m_event_tree.hilite_item(m_last_message_node);
	}
}
