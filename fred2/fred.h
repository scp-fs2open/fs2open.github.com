#ifndef _FRED_H
#define _FRED_H
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

#include "BgBitmapDlg.h"
#include "BriefingEditorDlg.h"
#include "resource.h"
#include "ShipEditorDlg.h"
#include "WaypointPathDlg.h"
#include "wing_editor.h"
#include "musicplayerdlg.h"

#include "globalincs/systemvars.h"
#include "globalincs/systemvars.h"
#include "mission/missionparse.h"

#define MODIFY(a, b) do {   \
	if (a != (b)) {         \
		a = (b);            \
		set_modified();     \
	}                       \
} while(false)

#define F_RENDER_SHIP_MODELS 0x01
#define F_RENDER_SHIP_ICONS  0x02

// user interface types
#define HOFFOSS_INTERFACE  1
#define ALLENDER_INTERFACE 2

typedef struct window_data
{
	WINDOWPLACEMENT p;
	int visible;
	int valid;
	int processed;
} window_data;

/**
 * @class CFREDApp
 *
 * @breif The FRED Application class
 */
class CFREDApp : public CWinApp
{
public:
	/**
	 * @brief Record the relavent window data for the given window
	 *
	 * @param[out] wndd struct associated with the window
	 * @param[in]  wnd  Window to record data from
	 */
	void record_window_data(window_data *wndd, CWnd *wnd);

	/**
	 * @brief Initializes the given window
	 *
	 * @param[in,out] wndd   The input window's data
	 * @param[in,out] wnd    The window to init
	 * @param[in]     adjust Height, in pixels, to adjust the window by
	 * @param[in]     pre    Pre-placement?
	 *
	 * @returns  0 If sucessful, or
	 * @returns -1 If pre is nonzero and the window is not visible, or
	 * @returns -2 If the window has been already initialized
	 */
	int init_window(window_data *wndd, CWnd *wnd, int adjust = 0, int pre = 0);
	
	/**
	 * @brief Read window data from the .ini file
	 *
	 * @param[in]  Name of the window to read
	 * @param[out] wndd Window data to read to
	 */
	void read_window(char *name, window_data *wndd);

	/**
	 * @brief Write window data to the .ini file
	 * 
	 * @param[in] name Name of the window to write
	 * @param[in] wndd Window data to write from
	 */
	void write_window(char *name, window_data *wndd);

	/**
	 * @brief Writes preferences the to .ini file.
	 *
	 * @param[in] degree If nonzero, also write window data
	 */
	void write_ini_file(int degree = 0);

	/**
	 * @brief Constructor
	 */
	CFREDApp();

	/**
	 * @brief Deconstcutor
	 */
	~CFREDApp();

	//{{AFX_VIRTUAL(CFREDApp)
	/**
	 * @brief Handler for window initialization
	 */
	virtual BOOL InitInstance();
	
	/**
	 * @brief Idle handler
	 */
	virtual BOOL OnIdle(LONG lCount);
	
	/**
	 * @brief Exit handler
	 */
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CFREDApp)
	/**
	 * @brief Help->About handler
	 */
	afx_msg void OnAppAbout();

	/**
	 * @brief Handler for opening a file
	 */
	afx_msg void OnFileOpen();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int app_init;
	CString m_sInitialDir;
};

/**
 * @brief Add a message to be processed at the end of this frame.
 *
 * @details This is useful if you need the display to update before it's useful to process the message.  For example,
 * right click brings up a popup menu, but the menu it brings up depends on where you right clicked.  If you right
 * click on a ship, you get a message that pertains to the chosen ship.  It is useful to have a visual indication that
 * you have changed the current ship.
 *
 * @note z64: I get this feeling that this is a useful function.
 */
void add_pending_message(HWND hwnd, int id, WPARAM wparam, LPARAM lparam, int skip_count);

/**
 * @brief Initializes the message vector
 */
void init_pending_messages(void);

/**
 * @brief Refreshes the viewport
 */
void update_map_window();

extern bool Fred_active;    //!< True, if the main window is in-focus
extern int Update_window;   //!< Number of times we need to redraw the window. Usually just 1 or 0
extern HCURSOR h_cursor_move;   //!< Cursor resource (icon) used for translational movement of selected items
extern HCURSOR h_cursor_rotate; //!< Cursor resource (icon) used for rotational movement of selected items

extern CWnd*                Prev_window;            //!< The currently active window
extern CShipEditorDlg       Ship_editor_dialog;     //!< The ship editor instance
extern wing_editor          Wing_editor_dialog;     //!< The wing editor instance
extern waypoint_path_dlg    Waypoint_editor_dialog; //!< The waypoint editor instance
extern music_player_dlg		Music_player_dialog;    //!< The waypoint editor instance
extern bg_bitmap_dlg*       Bg_bitmap_dialog;       //!< The bitmap dialog instance
extern briefing_editor_dlg* Briefing_dialog;        //!< The briefing editor instance

extern CFREDApp theApp; //!< The application instance

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
extern window_data MusPlayer_wnd_data;
extern window_data Starfield_wnd_data;
extern window_data Asteroid_wnd_data;
extern window_data Mission_notes_wnd_data;

#endif // _FRED_H
