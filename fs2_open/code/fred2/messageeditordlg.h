/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/MessageEditorDlg.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Old message editor dialog box handling code.  This was designed a LONG time ago
 * and because so much changed, I created a new one from scratch instead.  This is
 * only around just in case it might be useful.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.3  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
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
 * 11    1/07/98 5:58p Hoffoss
 * Combined message editor into event editor.
 * 
 * 10    1/06/98 4:19p Hoffoss
 * Made message editor accept returns instead of closing dialog.
 * 
 * 9     10/13/97 11:37a Allender
 * added personas to message editor in Fred
 * 
 * 8     10/08/97 4:41p Hoffoss
 * Changed the way message editor works.  Each message is updated
 * perminently when you switch messages (as if ok button was pressed).
 * 
 * 7     7/14/97 9:55p Hoffoss
 * Making changes to message editor system.
 * 
 * 6     7/10/97 2:32p Hoffoss
 * Made message editor dialog box modeless.
 * 
 * 5     7/02/97 5:09p Hoffoss
 * Added browse buttons to message editor.
 * 
 * 4     5/20/97 2:28p Hoffoss
 * Added message box queries for close window operation on all modal
 * dialog boxes.
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

#include "mission/missionmessage.h"

/////////////////////////////////////////////////////////////////////////////
// CMessageEditorDlg dialog

class CMessageEditorDlg : public CDialog
{
// Construction
public:
	int find_event();
	int query_modified();
	void OnCancel();
	int update(int num);
	void update_cur_message();
	void OnOK();
	CMessageEditorDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMessageEditorDlg)
	enum { IDD = IDD_MESSAGE_EDITOR };
	sexp_tree	m_tree;
	CString	m_avi_filename;
	CString	m_wave_filename;
	CString	m_message_text;
	CString	m_message_name;
	int		m_cur_msg;
	int		m_priority;
	int		m_sender;
	int		m_persona;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMessageEditorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_event_num;  // event index if existing event is being used for formula
	int modified;

	// Generated message map functions
	//{{AFX_MSG(CMessageEditorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeMessageList();
	afx_msg void OnUpdateName();
	afx_msg void OnDelete();
	afx_msg void OnNew();
	afx_msg void OnClose();
	afx_msg void OnBrowseAvi();
	afx_msg void OnBrowseWave();
	afx_msg void OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CMessageEditorDlg *Message_editor_dlg;
