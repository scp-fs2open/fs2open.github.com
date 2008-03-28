/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/CmdBrief.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Command Briefing Editor
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
 * 4     4/20/98 4:40p Hoffoss
 * Added a button to 4 editors to play the chosen wave file.
 * 
 * 3     3/19/98 4:24p Hoffoss
 * Added remaining support for command brief screen (ANI and WAVE file
 * playing).
 * 
 * 2     3/05/98 3:59p Hoffoss
 * Added a bunch of new command brief stuff, and asteroid initialization
 * to Fred.
 *
 * $NoKeywords: $
 */

#include "missionui/missioncmdbrief.h"

/////////////////////////////////////////////////////////////////////////////
// cmd_brief_dlg dialog

class cmd_brief_dlg : public CDialog
{
// Construction
public:
	cmd_brief_dlg(CWnd* pParent = NULL);   // standard constructor
	void update_data(int update = 1);
	void OnOK();

// Dialog Data
	//{{AFX_DATA(cmd_brief_dlg)
	enum { IDD = IDD_CMD_BRIEF };
	CString	m_ani_filename;
	CString	m_text;
	CString	m_stage_title;
	CString	m_wave_filename;
	//}}AFX_DATA

	CBitmap m_play_bm;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cmd_brief_dlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_cur_stage;
	int m_last_stage;
	int m_wave_id;
	cmd_brief *last_cmd_brief;

	void copy_stage(int from, int to);

	// Generated message map functions
	//{{AFX_MSG(cmd_brief_dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnNext();
	afx_msg void OnPrev();
	afx_msg void OnAddStage();
	afx_msg void OnInsertStage();
	afx_msg void OnDeleteStage();
	afx_msg void OnBrowseAni();
	afx_msg void OnBrowseWave();
	afx_msg void OnPlay();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
