/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/CampaignTreeWnd.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Campaign display tree window code.  Works very closely with the Campaign editor dialog box.
 *
 * $Log: not supported by cvs2svn $
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
 * 12    12/18/97 5:11p Allender
 * initial support for ship/weapon persistence
 * 
 * 11    8/14/97 11:54p Hoffoss
 * Added more error checking to Campaign editor, and made exit from
 * Campaign editor reload last mission in Fred (unless specifically
 * loading another mission).
 * 
 * 10    8/13/97 5:49p Hoffoss
 * Fixed bugs, made additions.
 * 
 * 9     8/13/97 12:46p Hoffoss
 * Added campaign error checker, accelerator table, and mission goal data
 * listings to sexp tree right click menu.
 * 
 * 8     5/15/97 12:45p Hoffoss
 * Extensive changes to fix many little bugs.
 * 
 * 7     5/14/97 12:54p Hoffoss
 * Added sexp tree for campaign branches, branch hilighting, and branch
 * reordering.
 * 
 * 6     5/13/97 12:46p Hoffoss
 * Added close campaign editor functions, changed global pointer to have
 * capped first letter.
 * 
 * 5     5/13/97 11:13a Hoffoss
 * Added remaining file menu options to campaign editor.
 * 
 * 4     5/13/97 10:52a Hoffoss
 * Added campaign saving code.
 * 
 * 3     5/09/97 9:50a Hoffoss
 * Routine code check in.
 * 
 * 2     5/01/97 4:11p Hoffoss
 * Started on Campaign editor stuff, being sidetracked with fixing bugs
 * now, though, so checking it for now.
 *
 * $NoKeywords: $
 */

/////////////////////////////////////////////////////////////////////////////
// campaign_tree_wnd window

class campaign_tree_wnd : public CFrameWnd
{
	DECLARE_DYNCREATE(campaign_tree_wnd)

// Construction
public:
	campaign_tree_wnd();

// Attributes
public:

// Operations
public:
	int error_checker();
	int fred_check_sexp(int sexp, int type, char *msg, ...);
	int error(char *msg, ...);
	int internal_error(char *msg, ...);
	int save_modified();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(campaign_tree_wnd)
	//}}AFX_VIRTUAL

private:
	int g_err;

// Implementation
public:
	virtual ~campaign_tree_wnd();
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	// Generated message map functions
	CSplitterWnd m_splitter;

	//{{AFX_MSG(campaign_tree_wnd)
	afx_msg void OnUpdateCpgnFileOpen(CCmdUI* pCmdUI);
	afx_msg void OnCpgnFileOpen();
	afx_msg void OnDestroy();
	afx_msg void OnCpgnFileSave();
	afx_msg void OnCpgnFileSaveAs();
	afx_msg void OnCpgnFileNew();
	afx_msg void OnClose2();
	afx_msg void OnErrorChecker();
	afx_msg void OnClose();
	afx_msg void OnInitialShips();
	afx_msg void OnInitialWeapons();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

extern int Mission_filename_cb_format;
extern int Campaign_modified;
extern int Bypass_clear_mission;
extern campaign_tree_wnd *Campaign_wnd;