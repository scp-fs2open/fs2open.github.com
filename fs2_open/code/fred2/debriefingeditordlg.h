/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/DebriefingEditorDlg.h $
 * $Revision: 1.2 $
 * $Date: 2006-01-26 04:01:58 $
 * $Author: Goober5000 $
 *
 * Debriefing editor dialog.  Used to edit mission debriefings of course.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.3  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.2  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
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
 * 9     7/07/98 2:11p Hoffoss
 * 
 * 8     4/20/98 4:40p Hoffoss
 * Added a button to 4 editors to play the chosen wave file.
 * 
 * 7     3/17/98 2:06p Hoffoss
 * Made enter key not close the dialog box (default windows behavior, even
 * when no ok button.  Talk about stupid. :)
 * 
 * 6     2/09/98 9:25p Allender
 * team v team support.  multiple pools and briefings
 * 
 * 5     2/04/98 4:32p Allender
 * support for multiple briefings and debriefings.  Changes to mission
 * type (now a bitfield).  Bitfield defs for multiplayer modes
 * 
 * 4     11/10/97 11:58a Johnson
 * Added support to debriefing editor for "press cancel to go to reference
 * of sexp".
 * 
 * 3     10/14/97 12:06p Hoffoss
 * Recoded debriefing editor to utilize new format.
 * 
 * 2     7/08/97 2:03p Hoffoss
 * Debriefing editor coded and implemented.
 *
 * $NoKeywords: $
 */

/////////////////////////////////////////////////////////////////////////////
// debriefing_editor_dlg dialog

class debriefing_editor_dlg : public CDialog
{
// Construction
public:
	void OnOK();
	void update_data(int update = 1);
	debriefing_editor_dlg(CWnd* pParent = NULL);   // standard constructor
	int select_sexp_node;

// Dialog Data
	//{{AFX_DATA(debriefing_editor_dlg)
	enum { IDD = IDD_DEBRIEFING_EDITOR };
	sexp_tree	m_tree;
	CString	m_text;
	CString	m_voice;
	CString	m_stage_title;
	CString	m_rec_text;
	int		m_current_debriefing;
	//}}AFX_DATA

	CBitmap m_play_bm;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(debriefing_editor_dlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_cur_stage;
	int m_last_stage;
	int modified;

	void copy_stage(int from, int to, int clear_formula = 0);

	// Generated message map functions
	//{{AFX_MSG(debriefing_editor_dlg)
	afx_msg void OnNext();
	afx_msg void OnPrev();
	afx_msg void OnBrowse();
	afx_msg void OnAddStage();
	afx_msg void OnDeleteStage();
	afx_msg void OnInsertStage();
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnPlay();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};
