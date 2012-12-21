/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
