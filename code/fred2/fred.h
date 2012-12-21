/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "globalincs/systemvars.h"		//	Low level variables, common to FreeSpace and Fred
#include "resource.h"       // main symbols
#include "mission/missionparse.h"
#include "ShipEditorDlg.h"
#include "wing_editor.h"
#include "WaypointPathDlg.h"
#include "BgBitmapDlg.h"
#include "BriefingEditorDlg.h"
#include "globalincs/systemvars.h"

#define MODIFY(a, b) do {	\
	if (a != (b)) {			\
		a = (b);					\
		set_modified();		\
	}								\
} while(0)

#define	F_RENDER_SHIP_MODELS	0x01
#define	F_RENDER_SHIP_ICONS	0x02

// user interface types
#define HOFFOSS_INTERFACE	1
#define ALLENDER_INTERFACE	2

typedef struct window_data {
	WINDOWPLACEMENT p;
	int visible;
	int valid;
	int processed;
} window_data;

/////////////////////////////////////////////////////////////////////////////
// CFREDApp:
// See FRED.cpp for the implementation of this class
//

class CFREDApp : public CWinApp
{
	int app_init;
	CString m_sInitialDir;

public:
	void record_window_data(window_data *wndd, CWnd *wnd);
	int init_window(window_data *wndd, CWnd *wnd, int adjust = 0, int pre = 0);
	void read_window(char *name, window_data *wndd);
	void write_window(char *name, window_data *wndd);
	void write_ini_file(int degree = 0);
	CFREDApp();
	~CFREDApp();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFREDApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CFREDApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//	Add a message to be processed at the end of this frame.
//	This is useful if you need the display to update before it's useful
//	to process the message.  For example, right click brings up a popup menu.
//	But the menu it brings up depends on where you right clicked.  If you
//	right click on a ship, you get a message that pertains to the chosen
//	ship.  It is useful to have a visual indication that you have changed the
//	current ship.
void add_pending_message(HWND hwnd, int id, int wparam, int lparam, int skip_count);
void init_pending_messages(void);
void update_map_window();

extern int User_interface;
extern int Fred_active;
extern int Update_window;
extern HCURSOR h_cursor_move, h_cursor_rotate;

extern CWnd *Prev_window;
extern CShipEditorDlg	Ship_editor_dialog;
extern wing_editor		Wing_editor_dialog;
extern waypoint_path_dlg	Waypoint_editor_dialog;
extern bg_bitmap_dlg		*Bg_bitmap_dialog;
extern briefing_editor_dlg	*Briefing_dialog;

extern CFREDApp theApp;

extern window_data Main_wnd_data;
extern window_data Ship_wnd_data;
extern window_data Wing_wnd_data;
extern window_data Object_wnd_data;
extern window_data Mission_goals_wnd_data;
extern window_data Messages_wnd_data;
extern window_data Player_wnd_data;
extern window_data Events_wnd_data;
extern window_data Bg_wnd_data;
extern window_data Briefing_wnd_data;
extern window_data Reinforcement_wnd_data;
extern window_data Waypoint_wnd_data;
extern window_data Starfield_wnd_data;
extern window_data Asteroid_wnd_data;
extern window_data Mission_notes_wnd_data;
