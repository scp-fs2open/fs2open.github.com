/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/FREDDoc.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * FREDDoc.h : interface of the CFREDDoc class
 * Document class for document/view architechure, which we don't really use in
 * Fred, but MFC forces you do use like it or not.  Handles loading/saving
 * mainly.  Most of the MFC related stuff is handled in FredView.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.7  2005/04/13 20:02:08  chhogg
 * Changed vector to vec3d for Fred.
 *
 * Revision 1.6  2005/03/29 03:43:11  phreak
 * ai directory fixes as well as fixes for the new jump node code
 *
 * Revision 1.5  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.4  2003/09/30 04:05:08  Goober5000
 * updated FRED to import FS1 default weapons loadouts as well as missions
 * --Goober5000
 *
 * Revision 1.3  2003/09/28 21:22:58  Goober5000
 * added the option to import FSM missions, added a replace function, spruced
 * up my $player, $rank, etc. code, and fixed encrypt being misspelled as 'encrpyt'
 * --Goober5000
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:57  inquisitor
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
 * 15    8/17/97 10:22p Hoffoss
 * Fixed several bugs in Fred with Undo feature.  In the process, recoded
 * a lot of CFile.cpp.
 * 
 * 14    8/13/97 5:49p Hoffoss
 * Fixed bugs, made additions.
 * 
 * 13    8/13/97 12:46p Hoffoss
 * Added campaign error checker, accelerator table, and mission goal data
 * listings to sexp tree right click menu.
 * 
 * 12    6/09/97 4:57p Hoffoss
 * Added autosave and undo to Fred.
 * 
 * 11    6/05/97 6:10p Hoffoss
 * Added features: Autosaving, object hiding.  Also fixed some minor bugs.
 * 
 * 10    5/21/97 5:42p Hoffoss
 * Added features requested on Todo list.
 * 
 * 9     2/20/97 4:03p Hoffoss
 * Several ToDo items: new reinforcement clears arrival cue, reinforcement
 * control from ship and wing dialogs, show grid toggle.
 * 
 * 8     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 * 
 * 14    2/12/97 12:25p Hoffoss
 * Expanded on global error checker, added initial orders conflict
 * checking and warning, added waypoint editor dialog and code.
 * 
 * 13    1/30/97 2:24p Hoffoss
 * Added remaining mission file structures and implemented load/save of
 * them.
 *
 * $NoKeywords: $
 */

#ifndef _FREDDOC_H
#define _FREDDOC_H

#include "ai/ailocal.h"
#include "MissionSave.h"

#define MISSION_BACKUP_NAME	"Backup"

#define	US_WORLD_CHANGED	0x01
#define	US_VIEW_CHANGED		0x02

class CFREDDoc : public CDocument
{
protected: // create from serialization only
	CFREDDoc();
	DECLARE_DYNCREATE(CFREDDoc)

// Attributes
public:
	int check_undo();
	int autoload();
	int load_mission(char *pathname, int importFSM = 0);
	int autosave(char *desc);
	int save_matrix(matrix &m, FILE *fp);
	int save_vector(vec3d &v, FILE *fp);
	BOOL confirm_deleting;
	BOOL show_capital_ships;
	BOOL show_elevations;
	BOOL show_fighters;
	BOOL show_grid;
	BOOL show_misc_objects;
	BOOL show_planets;
	BOOL show_waypoints;
	BOOL show_starfield;
	char mission_pathname[256];

// Operations
public:
	CString undo_desc[BACKUP_DEPTH + 1];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFREDDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void OnEditClearAll();
	virtual void DeleteContents();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFREDDoc();
	static void UpdateStatus(int flags = US_WORLD_CHANGED);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext &dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFREDDoc)
	afx_msg void OnEditDelete();
	afx_msg void OnDuplicate();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditHold();
	afx_msg void OnEditFetch();
	afx_msg void OnEditPaste();
	afx_msg void OnEditUndo();
	afx_msg void OnFilePreferences();
	afx_msg void OnFileSave();
	afx_msg void OnFileNew();
	afx_msg void editor_init_mission();
	afx_msg void OnFileImportFSM();
	afx_msg void OnFileImportWeapons();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int save_waypoint_list(waypoint_list &w, FILE *fp);
	int save_waypoints(FILE *fp);
	int save_goals(FILE *fp);
	int save_wings(FILE *fp);
	int save_objects(FILE *fp);
	int save_players(FILE *fp);
	int save_briefing_info(FILE *fp);
	int save_plot_info(FILE *fp);
	int save_mission_info(FILE *FP);
};

extern int Local_modified;
extern int Undo_available;
extern int Undo_count;
extern CFREDDoc *FREDDoc_ptr;

void set_modified(BOOL arg = TRUE);

/////////////////////////////////////////////////////////////////////////////

#endif
