/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
	int load_mission(char *pathname, int flags = 0);
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
