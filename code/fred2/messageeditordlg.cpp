/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/MessageEditorDlg.cpp $
 * $Revision: 1.4 $
 * $Date: 2007-07-15 04:45:42 $
 * $Author: Goober5000 $
 *
 * Old message editor dialog box handling code.  This was designed a LONG time ago
 * and because so much changed, I created a new one from scratch instead.  This is
 * only around just in case it might be useful.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2007/03/07 22:43:24  karajorma
 * Make .ogg voice files selectable in FRED.
 *
 * Revision 1.2  2007/02/15 02:03:56  phreak
 * Apparently Visual C++ 2005 follows standard syntax rules.
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.4  2005/01/29 05:40:27  Goober5000
 * regular docking should work again in FRED now (multiple docking still isn't done yet)
 * --Goober5000
 *
 * Revision 1.3  2004/09/22 06:57:34  Goober5000
 * FRED framework for when-argument
 * --Goober5000
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:59  inquisitor
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
 * 26    4/07/98 10:51a Allender
 * remove any allied from message senders.  Make heads for mission
 * specific messages play appropriately
 * 
 * 25    1/07/98 5:58p Hoffoss
 * Combined message editor into event editor.
 * 
 * 24    1/06/98 4:19p Hoffoss
 * Made message editor accept returns instead of closing dialog.
 * 
 * 23    12/30/97 9:39a Allender
 * minor fixups with message editing
 * 
 * 22    11/17/97 4:57p Allender
 * added persona support in FreeSpace.  A couple of new messages for
 * message.tbl which Dan didn't have
 * 
 * 21    10/16/97 8:54p Duncan
 * Fixed bug in adding new message.
 * 
 * 20    10/14/97 10:31a Allender
 * disable persona field for builtin messages
 * 
 * 19    10/13/97 11:37a Allender
 * added personas to message editor in Fred
 * 
 * 18    10/10/97 6:21p Hoffoss
 * Put in Fred support for training object list editing.
 * 
 * 17    10/08/97 7:52p Hoffoss
 * Made renaming events update all sexp references to it (added a
 * generalized function to replace all references to a name by argument
 * type).
 * 
 * 16    10/08/97 4:41p Hoffoss
 * Changed the way message editor works.  Each message is updated
 * perminently when you switch messages (as if ok button was pressed).
 * 
 * 15    9/09/97 9:19a Hoffoss
 * Fixed bug in Fred: messages shouldn't be send from the player to the
 * player.
 * 
 * 14    8/08/97 3:29p Duncan
 * Fixed bug with strings exceeding their limits.
 * 
 * 13    8/05/97 5:12p Jasen
 * Added advanced_stricmp() function to handle NULL pointers gracefully,
 * and utilized it in message editor query update function.
 * 
 * 12    8/01/97 3:47p Hoffoss
 * Fixed bug where message editor believes modifications occured when
 * nothing actually changed.
 * 
 * 11    7/24/97 4:34p Hoffoss
 * Fixed omission bug.
 * 
 * 10    7/16/97 6:30p Hoffoss
 * Added icons to sexp trees, mainly because I think they will be required
 * for drag n drop.
 * 
 * 9     7/14/97 9:55p Hoffoss
 * Making changes to message editor system.
 * 
 * 8     7/10/97 2:32p Hoffoss
 * Made message editor dialog box modeless.
 * 
 * 7     7/02/97 5:09p Hoffoss
 * Added browse buttons to message editor.
 * 
 * 6     5/20/97 2:28p Hoffoss
 * Added message box queries for close window operation on all modal
 * dialog boxes.
 * 
 * 5     4/17/97 2:01p Hoffoss
 * All dialog box window states are saved between sessions now.
 * 
 * 4     4/01/97 5:15p Hoffoss
 * Fixed errors in max length checks, renaming a wing now renames the
 * ships in the wing as well, as it should.
 * 
 * 3     3/11/97 2:19p Hoffoss
 * New message structure support for Fred.
 * 
 * 2     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"
#include "MessageEditorDlg.h"
#include "FREDDoc.h"
#include "Management.h"
#include "Sexp_tree.h"
#include "EventEditor.h"
#include "mission/missionmessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMessageEditorDlg *Message_editor_dlg = NULL;

/////////////////////////////////////////////////////////////////////////////
// CMessageEditorDlg dialog

CMessageEditorDlg::CMessageEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMessageEditorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMessageEditorDlg)
	m_avi_filename = _T("");
	m_wave_filename = _T("");
	m_message_text = _T("");
	m_message_name = _T("");
	m_cur_msg = -1;
	m_priority = -1;
	m_sender = -1;
	m_persona = -1;
	//}}AFX_DATA_INIT

	m_tree.link_modified(&modified);
	modified = 0;
	m_event_num = -1;
}

void CMessageEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessageEditorDlg)
	DDX_Control(pDX, IDC_TREE, m_tree);
	DDX_CBString(pDX, IDC_AVI_FILENAME, m_avi_filename);
	DDX_CBString(pDX, IDC_WAVE_FILENAME, m_wave_filename);
	DDX_Text(pDX, IDC_MESSAGE_TEXT, m_message_text);
	DDX_Text(pDX, IDC_NAME, m_message_name);
	DDX_LBIndex(pDX, IDC_MESSAGE_LIST, m_cur_msg);
	DDX_CBIndex(pDX, IDC_PRIORITY, m_priority);
	DDX_CBIndex(pDX, IDC_SENDER, m_sender);
	DDX_CBIndex(pDX, IDC_PERSONA_NAME, m_persona);
	//}}AFX_DATA_MAP
	DDV_MaxChars(pDX, m_message_name, NAME_LENGTH - 1);
	DDV_MaxChars(pDX, m_message_text, MESSAGE_LENGTH - 1);
	DDV_MaxChars(pDX, m_avi_filename, MAX_FILENAME_LEN - 1);
	DDV_MaxChars(pDX, m_wave_filename, MAX_FILENAME_LEN - 1);
}

BEGIN_MESSAGE_MAP(CMessageEditorDlg, CDialog)
	//{{AFX_MSG_MAP(CMessageEditorDlg)
	ON_LBN_SELCHANGE(IDC_MESSAGE_LIST, OnSelchangeMessageList)
	ON_EN_UPDATE(IDC_NAME, OnUpdateName)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BROWSE_AVI, OnBrowseAvi)
	ON_BN_CLICKED(IDC_BROWSE_WAVE, OnBrowseWave)
	ON_NOTIFY(NM_RCLICK, IDC_TREE, OnRclickTree)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE, OnBeginlabeleditTree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE, OnEndlabeleditTree)
	ON_BN_CLICKED(ID_OK, OnOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageEditorDlg message handlers

BOOL CMessageEditorDlg::OnInitDialog() 
{
	int i;
	CListBox *list;
	CComboBox *box;

	CDialog::OnInitDialog();
	theApp.init_window(&Messages_wnd_data, this);
	m_tree.setup();

	((CEdit *) GetDlgItem(IDC_NAME))->LimitText(NAME_LENGTH - 1);
	((CEdit *) GetDlgItem(IDC_MESSAGE_TEXT))->LimitText(MESSAGE_LENGTH - 1);
	((CComboBox *) GetDlgItem(IDC_AVI_FILENAME))->LimitText(MAX_FILENAME_LEN - 1);
	((CComboBox *) GetDlgItem(IDC_WAVE_FILENAME))->LimitText(MAX_FILENAME_LEN - 1);

	list = (CListBox *) GetDlgItem(IDC_MESSAGE_LIST);
	list->ResetContent();
	for (i=0; i<Num_messages; i++) {
		//Assert(list->FindStringExact(-1, Messages[i].name) == CB_ERR);
		// mwa we should probably not include builtin messages into this list!
		list->AddString(Messages[i].name);
	}

	box = (CComboBox *) GetDlgItem(IDC_AVI_FILENAME);
	for (i=0; i<Num_messages; i++)
		if (Messages[i].avi_info.name)
			if (box->FindStringExact(-1, Messages[i].avi_info.name) == CB_ERR)
				box->AddString(Messages[i].avi_info.name);

	box = (CComboBox *) GetDlgItem(IDC_WAVE_FILENAME);
	for (i=0; i<Num_messages; i++)
		if (Messages[i].wave_info.name)
			if (box->FindStringExact(i, Messages[i].wave_info.name) == CB_ERR)
				box->AddString(Messages[i].wave_info.name);

	// add the persona names into the combo box
	box = (CComboBox *)GetDlgItem(IDC_PERSONA_NAME);
	box->ResetContent();
	box->AddString("<None>");
	for (i = 0; i < Num_personas; i++ )
		box->AddString( Personas[i].name );

	box = (CComboBox *) GetDlgItem(IDC_SENDER);
	for (i=0; i<MAX_SHIPS; i++)
		if ((Ships[i].objnum >= 0) && (Objects[Ships[i].objnum].type == OBJ_SHIP))
			box->AddString(Ships[i].ship_name);

	for (i=0; i<MAX_WINGS; i++)
		if (Wings[i].wave_count)
			box->AddString(Wings[i].name);

	box->AddString("<Any wingman>");

	// MWA 4/7/98 -- removed any allied
	//box->AddString("<Any allied>");

	// set the first message to be the first non-builtin message (if it exists)
	if ( Num_messages > Num_builtin_messages )
		m_cur_msg = Num_builtin_messages;
	else if (Num_messages)
		m_cur_msg = 0;
	else
		m_cur_msg = -1;

	if (Num_messages >= MAX_MISSION_MESSAGES)
		GetDlgItem(IDC_NEW)->EnableWindow(FALSE);

	update_cur_message();
	return TRUE;
}

int CMessageEditorDlg::query_modified()
{
	char *ptr, buf[MESSAGE_LENGTH];
	int i;

	UpdateData(TRUE);
	if (modified)
		return 1;

	if (m_cur_msg < 0)
		return 0;

	ptr = (char *) (LPCTSTR) m_message_name;
	for (i=0; i<Num_messages; i++)
		if ((i != m_cur_msg) && (!stricmp(ptr, Messages[i].name)))
			break;

	if (i < Num_messages)
		if (stricmp(ptr, Messages[m_cur_msg].name))
			return 1;  // name is different and allowed to update

	string_copy(buf, m_message_text, MESSAGE_LENGTH - 1);
	if (stricmp(buf, Messages[m_cur_msg].message))
		return 1;

	ptr = (char *) (LPCTSTR) m_avi_filename;
	if (advanced_stricmp(ptr, Messages[m_cur_msg].avi_info.name))
		return 1;

	ptr = (char *) (LPCTSTR) m_wave_filename;
	if (advanced_stricmp(ptr, Messages[m_cur_msg].wave_info.name))
		return 1;

	// check to see if persona changed.  use -1 since we stuck a "None" for persona
	// at the beginning of the list.
	if ( (m_persona - 1 ) != Messages[m_cur_msg].persona_index )
		return 1;

	if (m_tree.query_false()) {
		if (m_event_num >= 0)
			return 1;

	} else {
		if (m_event_num < 0)
			return 1;
	}

	return 0;
}

void CMessageEditorDlg::OnOK()
{
}

void CMessageEditorDlg::OnOk()
{
	update(m_cur_msg);
	theApp.record_window_data(&Messages_wnd_data, this);
	delete Message_editor_dlg;
	Message_editor_dlg = NULL;
}

void CMessageEditorDlg::OnCancel()
{
	theApp.record_window_data(&Messages_wnd_data, this);
	delete Message_editor_dlg;
	Message_editor_dlg = NULL;
}

// load controls with structure data
void CMessageEditorDlg::update_cur_message()
{
	int node, enable = TRUE, enable2 = TRUE;

	if (m_cur_msg < 0)
	{
		enable = enable2 = FALSE;
		m_message_name = _T("");
		m_message_text = _T("");
		m_avi_filename = _T("");
		m_wave_filename = _T("");
		m_tree.clear_tree();
		m_persona = 0;
		m_sender = m_priority = -1;

	} else {
		m_message_name = Messages[m_cur_msg].name;
		m_message_text = Messages[m_cur_msg].message;
		if (Messages[m_cur_msg].avi_info.name)
			m_avi_filename = _T(Messages[m_cur_msg].avi_info.name);
		else
			m_avi_filename = _T("");

		if (Messages[m_cur_msg].wave_info.name)
			m_wave_filename = _T(Messages[m_cur_msg].wave_info.name);
		else
			m_wave_filename = _T("");

		// add persona id
		if ( Messages[m_cur_msg].persona_index != -1 )
			m_persona = Messages[m_cur_msg].persona_index + 1;		// add one for the "none" at the beginning of the list
		else
			m_persona = 0;

		m_event_num = find_event();
		if (m_event_num < 0) {
			node = -1;
			m_sender = m_priority = 0;

		} else
			node = CADR(Mission_events[m_event_num].formula);

		m_tree.load_tree(node, "false");
	}

	if (m_cur_msg < Num_builtin_messages)
		enable = FALSE;

	GetDlgItem(IDC_NAME)->EnableWindow(enable);
	GetDlgItem(IDC_MESSAGE_TEXT)->EnableWindow(enable);
	GetDlgItem(IDC_AVI_FILENAME)->EnableWindow(enable);
	GetDlgItem(IDC_WAVE_FILENAME)->EnableWindow(enable);
	GetDlgItem(IDC_DELETE)->EnableWindow(enable);
	GetDlgItem(IDC_TREE)->EnableWindow(enable2);
	GetDlgItem(IDC_SENDER)->EnableWindow(enable2);
	GetDlgItem(IDC_PRIORITY)->EnableWindow(enable2);
	GetDlgItem(IDC_PERSONA_NAME)->EnableWindow(enable);
	UpdateData(FALSE);
}

int CMessageEditorDlg::find_event()
{
	char *str;
	int i, formula, node;
	CComboBox *box;

	for (i=0; i<Num_mission_events; i++) {
		node = Mission_events[i].formula;
		if ( get_operator_const(CTEXT(node)) == OP_WHEN || get_operator_const(CTEXT(node)) == OP_EVERY_TIME
			|| get_operator_const(CTEXT(node)) == OP_WHEN_ARGUMENT || get_operator_const(CTEXT(node)) == OP_EVERY_TIME_ARGUMENT )
		{
			// Goober5000 - the bool part of the when-argument conditional starts at the second, not first, argument
			if (get_operator_const(CTEXT(node)) == OP_WHEN_ARGUMENT || get_operator_const(CTEXT(node)) == OP_EVERY_TIME_ARGUMENT)
				node = CDR(node);

			node = CDR(node);
			formula = CAR(node);  // bool conditional
			if (CDR(CDR(node)) == -1) {  // only 1 action
				node = CADR(node);
				if ((get_operator_const(CTEXT(node)) == OP_SEND_MESSAGE) && !stricmp(CTEXT(CDR(CDR(CDR(node)))), m_message_name)) {
					box = (CComboBox *) GetDlgItem(IDC_SENDER);
					str = CTEXT(CDR(node));
					m_sender = box->FindStringExact(-1, str);
					if (m_sender == CB_ERR)
						m_sender = 0;

					box = (CComboBox *) GetDlgItem(IDC_PRIORITY);
					str = CTEXT(CDR(CDR(node)));
					m_priority = box->FindStringExact(-1, str);
					if (m_priority == CB_ERR)
						m_priority = 0;

					return i;
				}
			}
		}
	}

	m_sender = m_priority = 0;
	return -1;
}

void CMessageEditorDlg::OnSelchangeMessageList() 
{
	int old = m_cur_msg;
	static int flag = 0;

	if (flag)
		return;

	if (update(m_cur_msg)) {
		flag = 1;
		((CListBox *) GetDlgItem(IDC_MESSAGE_LIST)) -> SetCurSel(old);
		m_cur_msg = old;
		flag = 0;
		return;
	}

	update_cur_message();
}

void CMessageEditorDlg::OnUpdateName() 
{
}

int CMessageEditorDlg::update(int num)
{
	char *ptr, buf[4096];
	int i, node, fnode;
	CListBox *list;

	UpdateData(TRUE);
	if (num >= 0)
	{
		ptr = (char *) (LPCTSTR) m_message_name;
		for (i=0; i<Num_messages; i++)
			if ((i != num) && (!stricmp(m_message_name, Messages[i].name)))
				break;

		if (i == Num_messages) {  // update name if no conflicts, otherwise keep old name
			update_sexp_references(Messages[num].name, ptr, OPF_MESSAGE);
			string_copy(Messages[num].name, m_message_name, NAME_LENGTH - 1);

			list = (CListBox *) GetDlgItem(IDC_MESSAGE_LIST);
			list->DeleteString(num);
			list->InsertString(num, m_message_name);
		}

		string_copy(Messages[num].message, m_message_text, MESSAGE_LENGTH - 1);
		if (Messages[num].avi_info.name)
			free(Messages[num].avi_info.name);

		ptr = (char *) (LPCTSTR) m_avi_filename;
		if (!ptr || !strlen(ptr))
			Messages[num].avi_info.name = NULL;
		else
			Messages[num].avi_info.name = strdup(ptr);

		if (Messages[num].wave_info.name)
			free(Messages[num].wave_info.name);

		ptr = (char *) (LPCTSTR) m_wave_filename;
		if (!ptr || !strlen(ptr))
			Messages[num].wave_info.name = NULL;
		else
			Messages[num].wave_info.name = strdup(ptr);

		// update the persona to the message.  We subtract 1 for the "None" at the beginning of the combo
		// box list.
		Messages[num].persona_index = m_persona - 1;

		if (m_tree.query_false()) {
			if (m_event_num >= 0) {  // need to delete event
				i = m_event_num;
				free_sexp2(Mission_events[i].formula);
				Assert(i < Num_mission_events);
				while (i < Num_mission_events - 1) {
					Mission_events[i] = Mission_events[i + 1];
					i++;
				}

				Num_mission_events--;
				m_event_num = -1;
			}

		} else {
			if (m_event_num >= 0)
				free_sexp2(Mission_events[m_event_num].formula);
			
			else {
				if (Num_mission_events == MAX_MISSION_EVENTS) {
					MessageBox("You have reached the limit on mission events.\n"
						"Can't add an event to send this message.");

					goto exit;
				}

				Assert(Num_mission_events < MAX_MISSION_EVENTS);
				m_event_num = Num_mission_events++;
				string_copy(Mission_events[m_event_num].name, m_message_name, NAME_LENGTH - 1);
				Mission_events[m_event_num].repeat_count = 1;
				Mission_events[m_event_num].interval = 1;
				Mission_events[m_event_num].score = 0;
				Mission_events[m_event_num].chain_delay = -1;
				Mission_events[m_event_num].objective_text = NULL;
				Mission_events[m_event_num].objective_key_text = NULL;
			}

			fnode = m_tree.save_tree();
			ptr = (char *) (LPCTSTR) m_message_name;
			node = alloc_sexp(ptr, SEXP_ATOM, SEXP_ATOM_STRING, -1, -1);
			((CComboBox *) GetDlgItem(IDC_PRIORITY))->GetLBText(m_priority, buf);
			node = alloc_sexp(buf, SEXP_ATOM, SEXP_ATOM_STRING, -1, node);
			((CComboBox *) GetDlgItem(IDC_SENDER))->GetLBText(m_sender, buf);
			node = alloc_sexp(buf, SEXP_ATOM, SEXP_ATOM_STRING, -1, node);
			node = alloc_sexp("send-message", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, node);
			node = alloc_sexp("", SEXP_LIST, SEXP_ATOM_LIST, node, -1);
			node = alloc_sexp("", SEXP_LIST, SEXP_ATOM_LIST, fnode, node);
			node = alloc_sexp("when", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, node);
			Mission_events[m_event_num].formula = node;
		}
	}

exit:
	if (query_modified())
		set_modified();

	modified = 0;
	return 0;
}

void CMessageEditorDlg::OnDelete() 
{
	char buf[256];
	int i;

	Assert((m_cur_msg >= 0) && (m_cur_msg < Num_messages));
	if (Messages[m_cur_msg].avi_info.name)
		free(Messages[m_cur_msg].avi_info.name);
	if (Messages[m_cur_msg].wave_info.name)
		free(Messages[m_cur_msg].wave_info.name);

	((CListBox *) GetDlgItem(IDC_MESSAGE_LIST))->DeleteString(m_cur_msg);
	sprintf(buf, "<%s>", Messages[m_cur_msg].name);
	update_sexp_references(Messages[m_cur_msg].name, buf, OPF_MESSAGE);

	for (i=m_cur_msg; i<Num_messages-1; i++)
		Messages[i] = Messages[i + 1];

	Num_messages--;
	if (m_cur_msg >= Num_messages)
		m_cur_msg = Num_messages - 1;

	GetDlgItem(IDC_NEW)->EnableWindow(TRUE);
	modified = 1;
	update_cur_message();
}

void CMessageEditorDlg::OnNew() 
{
	if (update(m_cur_msg))
		return;

	Assert(Num_messages < MAX_MISSION_MESSAGES);
	strcpy(Messages[Num_messages].name, "<new message>");
	((CListBox *) GetDlgItem(IDC_MESSAGE_LIST))->AddString("<new message>");

	strcpy(Messages[Num_messages].message, "<put description here>");
	Messages[Num_messages].avi_info.name = NULL;
	Messages[Num_messages].wave_info.name = NULL;
	Messages[Num_messages].persona_index = -1;
	m_cur_msg = Num_messages++;
	if (Num_messages >= MAX_MISSION_MESSAGES)
		GetDlgItem(IDC_NEW)->EnableWindow(FALSE);

	modified = 1;
	update_cur_message();
}

void CMessageEditorDlg::OnClose() 
{
	int z;

	modified = query_modified();
	if (modified) {
		z = MessageBox("Do you want to keep your changes?", "Close", MB_ICONQUESTION | MB_YESNOCANCEL);
		if (z == IDCANCEL)
			return;

		if (z == IDYES) {
			OnOK();
			return;
		}
	}
	
	theApp.record_window_data(&Messages_wnd_data, this);
	delete Message_editor_dlg;
	Message_editor_dlg = NULL;
}

void CMessageEditorDlg::OnBrowseAvi() 
{
	CString name;	

	UpdateData(TRUE);
	CFileDialog dlg(TRUE, "ani", m_avi_filename, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR,
		"Ani Files (*.ani)|*.ani|Avi Files (*.avi)|*.avi|Both (*.ani, *.avi)|*.ani;*.avi||");

	if (dlg.DoModal() == IDOK)
	{
		m_avi_filename = dlg.GetFileName();
		UpdateData(FALSE);
		modified = 1;
	}
}

void CMessageEditorDlg::OnBrowseWave() 
{
	CString name;	

	UpdateData(TRUE);
	CFileDialog dlg(TRUE, "wav", m_wave_filename, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR,
		"Voice Files (*.ogg, *.wav)|*.ogg;*.wav|Ogg Vorbis Files (*.ogg)|*.ogg|Wave Files (*.wav)|*.wav||");

	if (dlg.DoModal() == IDOK)
	{
		m_wave_filename = dlg.GetFileName();
		UpdateData(FALSE);
		modified = 1;
	}
}

void CMessageEditorDlg::OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_tree.right_clicked();
	*pResult = 0;
}

void CMessageEditorDlg::OnBeginlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	CEdit *edit;

	if (m_tree.edit_label(pTVDispInfo->item.hItem) == 1)	{
		*pResult = 0;
		modified = 1;
		edit = m_tree.GetEditControl();
		Assert(edit);
		edit->SetLimitText(NAME_LENGTH - 1);

	} else
		*pResult = 1;
}

void CMessageEditorDlg::OnEndlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_tree.end_label_edit(pTVDispInfo->item.hItem, pTVDispInfo->item.pszText);
}
