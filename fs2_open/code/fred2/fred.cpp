/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Fred2/FRED.cpp $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * FRED.cpp : Defines the class behaviors for the application.
 * Global editor dialog box classes are instantiated here, initializes the
 * application (MFC level at least), processes the INI file.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2004/02/13 04:25:37  randomtiger
 * Added messagebox to offer startup choice between htl, non htl and quitting.
 * Tidied up the background dialog box a bit and added some disabled controls for future ambient light feature.
 * Set non htl to use a lower level of model and texture detail to boost poor performance on this path.
 *
 * Revision 1.7  2004/02/05 23:23:19  randomtiger
 * Merging OGL Fred into normal Fred, no conflicts, should be perfect
 *
 * Revision 1.6.2.4  2004/01/24 13:00:43  randomtiger
 * Implemented basic OGL HT&L but some drawing routines missing from cobe.lib so keeping turned off for now.
 * Added lighting menu option and made about dialog more useful.
 * Ship menu now defaults to the first entry.
 * Loads of debug to try and trap crashes.
 *
 * Revision 1.6.2.3  2004/01/20 19:11:29  randomtiger
 * Cut unneeded code, text now works in graphics window.
 *
 * Revision 1.6.2.2  2004/01/18 14:53:17  randomtiger
 * Textures work now, initialisation runs from cobe.lib not internally.
 *
 * Revision 1.6.2.1  2004/01/17 22:11:25  randomtiger
 * Large series of changes to make fred run though OGL. Requires code.lib to be up to date to compile.
 * Port still in progress, fred currently unusable, note this has been committed to fred_ogl branch.
 *
 * Revision 1.6  2004/01/14 05:51:51  Goober5000
 * fixed the file association annoyance
 * --Goober5000
 *
 * Revision 1.5  2003/10/15 22:10:34  Kazan
 * Da Species Update :D
 *
 * Revision 1.4  2003/08/26 20:32:41  Goober5000
 * added support for saving in FS2 retail format
 * --Goober5000
 *
 * Revision 1.3  2003/08/21 08:56:34  Goober5000
 * fixed two linker errors inadvertently added by people who changed stuff in freespace.cpp
 * --Goober5000
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:56  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 6     5/19/99 4:07p Dave
 * Moved versioning code into a nice isolated common place. Fixed up
 * updating code on the pxo screen. Fixed several stub problems.
 * 
 * 5     3/26/99 4:49p Dave
 * Made cruisers able to dock with stuff. Made docking points and paths
 * visible in fred.
 * 
 * 4     3/20/99 5:09p Dave
 * Fixed release build fred warnings and unhandled exception.
 * 
 * 3     10/29/98 10:41a Dave
 * Change the way cfile initializes exe directory.
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
 * 71    7/02/98 12:17p Hoffoss
 * Load and save options 'anti-aliased gridlines' and 'double fine
 * gridlines'.
 * 
 * 70    5/22/98 11:02a Allender
 * Added error conditions on compile and new MAX_SHIPS limits for when
 * FRED is defined when building
 * 
 * 69    5/22/98 1:06a Hoffoss
 * Made Fred not use OLE.
 * 
 * 68    4/17/98 1:41p Allender
 * took out function calls in NDEBUG mode
 * 
 * 67    3/25/98 4:14p Hoffoss
 * Split ship editor up into ship editor and a misc dialog, which tracks
 * flags and such.
 * 
 * 66    3/23/98 4:04p Hoffoss
 * Fixed dialog window initialization so it looks better at startup (they
 * don't flash on for a second).
 * 
 * 65    3/19/98 11:41a Hoffoss
 * Fixed problems with rendering and reading of flying controls in Fred.
 * 
 * 64    3/19/98 11:11a Allender
 * made Fred actually be able to move camera again -- Hoffos is going to
 * fix the code right.
 * 
 * 63    3/18/98 3:10p Allender
 * fixed update problems in OnIdle routine
 * 
 * 62    2/06/98 4:39p Hoffoss
 * Added better checking for whether Fred is in the foreground or
 * background.
 * 
 * 61    8/19/97 5:46p Hoffoss
 * Changed font used in Fred, and added display to show current eye
 * position.
 * 
 * 60    8/17/97 10:22p Hoffoss
 * Fixed several bugs in Fred with Undo feature.  In the process, recoded
 * a lot of CFile.cpp.
 * 
 * 59    8/14/97 2:32p Hoffoss
 * fixed bug where controlling an object doesn't cause screen updates, and
 * added a number of cool features to viewpoint/control object code.
 * 
 * 58    8/13/97 1:38p Hoffoss
 * Added ability to update multiple times, which in needed in one case to
 * brute force redraw so deleted ships actually do get removed from the
 * display.
 * 
 * 57    8/12/97 6:32p Hoffoss
 * Added code to allow hiding of arrival and departure cues in editors.
 * 
 * 56    8/10/97 4:52p Hoffoss
 * Made Fred startup with main window having focus.
 * 
 * 55    8/06/97 7:55p Hoffoss
 * Fixed bug with objects not seeming to be where they are drawn (due to
 * new briefing clip render stuff).  This caused rendering problems,
 * though.  So I fixed those next.
 * 
 * 54    8/05/97 1:29p Hoffoss
 * show sexp help saved between sessions.
 * 
 * 53    8/01/97 3:10p Hoffoss
 * Made Sexp help hidable.
 * 
 * 52    8/01/97 12:49p Hoffoss
 * Changed recently used files total to 9 instead of 4.
 * 
 * 51    6/18/97 11:46a Hoffoss
 * Fixed initial order object reference updating and added briefing dialog
 * window tracking data.
 * 
 * 50    6/03/97 10:18a Allender
 * make showCmd default to maximized since 0 will make the window hidden
 * by default
 * 
 * 49    6/02/97 5:18p Hoffoss
 * Fixed bug in error checker and improved startup code.
 * 
 * 48    6/02/97 11:52a Hoffoss
 * Custom cursors displayed when over objects in different modes.
 * 
 * 47    5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 46    5/09/97 9:45a Hoffoss
 * Added OLE initialization. (needed for drag and drop functionality)
 * 
 * 45    5/06/97 10:34a Hoffoss
 * Fixed bug where dialog boxes that change size (in the code) still
 * retain their old size from the INI stored values.  They are resized now
 * automatically.
 * 
 * 44    5/05/97 9:41a Hoffoss
 * Campaign editor begun.
 * 
 * 43    5/02/97 8:39a Allender
 * turn of heap allocation debuging
 * 
 * 42    4/29/97 1:58p Hoffoss
 * Added some debugging to Fred to try and track down sexp corruption
 * causes.
 * 
 * 41    4/28/97 3:43p Hoffoss
 * Show_waypoints defaults to on for Fred, and is saved as a preference
 * now.
 * 
 * 40    4/17/97 2:01p Hoffoss
 * All dialog box window states are saved between sessions now.
 * 
 * 39    4/03/97 11:35a Hoffoss
 * Fixed bugs: viewpoint didn't reset, initial orders not updated when
 * referenced ship is renamed or deleted.
 * 
 * 38    4/01/97 11:43p Mike
 * Resolve link errors due to addition of SystemVars.cpp.
 * 
 * 37    3/26/97 12:44p Hoffoss
 * Took out the debugging 3d compass on/off toggling I put in to test
 * rerendering with.
 * 
 * 36    3/20/97 3:55p Hoffoss
 * Major changes to how dialog boxes initialize (load) and update (save)
 * their internal data.  This should simplify things and create less
 * problems.
 * 
 * 35    3/13/97 12:08p Hoffoss
 * Waypoint path editor finished (apparently I didn't get around to
 * completing it before).
 * 
 * 34    3/12/97 12:39p Hoffoss
 * Fixed bugs in wing object management functions, several small additions
 * and rearrangements.
 * 
 * 33    3/06/97 3:35p Hoffoss
 * Added Show_outline stuff, moved show options to the view menu, fixed a
 * bug in message dialog editor.
 * 
 * 32    2/28/97 11:31a Hoffoss
 * Implemented modeless dialog saving and restoring, and changed some
 * variables names.
 * 
 * 31    2/27/97 5:54p Hoffoss
 * Implemented support for saving and restoring window positions.
 * 
 * 30    2/25/97 6:10p Hoffoss
 * Fixed bug with modeless dialog box errors on update.
 * 
 * 29    2/24/97 12:50p Hoffoss
 * First attempt at non-continuous redrawing.
 * 
 * 28    2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 * 
 * 27    2/12/97 12:25p Hoffoss
 * Expanded on global error checker, added initial orders conflict
 * checking and warning, added waypoint editor dialog and code.
 * 
 * 26    1/30/97 2:24p Hoffoss
 * Added remaining mission file structures and implemented load/save of
 * them.
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"

#include "MainFrm.h"
#include "FREDDoc.h"
#include "FREDView.h"
#include "FredRender.h"
#include "Management.h"

#include "graphics/2d.h"
#include "io/key.h"
#include "object/object.h"
#include "editor.h"
#include "CampaignTreeWnd.h"
#include "CampaignTreeView.h"
#include "CampaignEditorDlg.h"
#include "debugconsole/dbugfile.h"

#ifdef NDEBUG
#ifndef FRED
#error macro FRED is not defined when trying to build release Fred.  Please define FRED macro in build settings in all Fred projects
#endif
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


int	Fred_running = 1;
int	User_interface = HOFFOSS_INTERFACE;
int	FrameCount = 0;
int	Fred_active = 1;
int	Update_window = 1;
HCURSOR h_cursor_move, h_cursor_rotate;

// Goober5000 - needed to restore compatibility with the changes in fs2_open
float VIEWER_ZOOM_DEFAULT = 0.75f;
int Show_cpu = 0;

CWnd *Prev_window;
CShipEditorDlg	Ship_editor_dialog;
wing_editor		Wing_editor_dialog;
waypoint_path_dlg	Waypoint_editor_dialog;
bg_bitmap_dlg	*Bg_bitmap_dialog = NULL;
briefing_editor_dlg	*Briefing_dialog = NULL;

window_data Main_wnd_data;
window_data Ship_wnd_data;
window_data Wing_wnd_data;
window_data Object_wnd_data;
window_data Mission_goals_wnd_data;
window_data Messages_wnd_data;
window_data Player_wnd_data;
window_data Events_wnd_data;
window_data Bg_wnd_data;
window_data Briefing_wnd_data;
window_data Reinforcement_wnd_data;
window_data Waypoint_wnd_data;
window_data Starfield_wnd_data;
window_data Asteroid_wnd_data;
window_data Mission_notes_wnd_data;

/////////////////////////////////////////////////////////////////////////////
// CFREDApp

BEGIN_MESSAGE_MAP(CFREDApp, CWinApp)
	//{{AFX_MSG_MAP(CFREDApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFREDApp construction

CFREDApp::CFREDApp()
{
	app_init = 0;

	DBUGFILE_INIT();

	#ifndef NDEBUG
	outwnd_init();
	#endif
}

CFREDApp::~CFREDApp()
{
	DBUGFILE_DEINIT();
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CFREDApp object

CFREDApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CFREDApp initialization
char *c;
char *tok = "whee";
BOOL CFREDApp::InitInstance()
{	


	// disable the debug memory stuff
	_CrtSetDbgFlag(~(_CRTDBG_ALLOC_MEM_DF) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	LoadStdProfileSettings(9);  // Load standard INI file options (including MRU)
	User_interface = GetProfileInt("Preferences", "User interface", User_interface);
	Show_stars = GetProfileInt("Preferences", "Show stars", Show_stars);
	Show_grid_positions = GetProfileInt("Preferences", "Show grid positions", Show_grid_positions);
	Show_coordinates = GetProfileInt("Preferences", "Show coordinates", Show_coordinates);
	Show_compass = GetProfileInt("Preferences", "Show compass", Show_compass);
	Show_ship_models = GetProfileInt("Preferences", "Show ship models", Show_ship_models);
	Show_ship_info = GetProfileInt("Preferences", "Show ship info", Show_ship_info);
	Show_outlines = GetProfileInt("Preferences", "Show outlines", Show_outlines);
	Show_waypoints = GetProfileInt("Preferences", "Show waypoints", Show_waypoints);
	Show_sexp_help = GetProfileInt("Preferences", "Show sexp help", Show_sexp_help);
	physics_speed = GetProfileInt("Preferences", "Physics speed", physics_speed);
	physics_rot = GetProfileInt("Preferences", "Physics rotation", physics_rot);
	Hide_ship_cues = GetProfileInt("Preferences", "Hide ship cues", Hide_ship_cues);
	Hide_wing_cues = GetProfileInt("Preferences", "Hide wing cues", Hide_wing_cues);
	Autosave_disabled = GetProfileInt("Preferences", "Autosave disabled", Autosave_disabled);
	double_fine_gridlines = GetProfileInt("Preferences", "Double fine gridlines", double_fine_gridlines);
	Aa_gridlines = GetProfileInt("Preferences", "Anti aliased gridlines", Aa_gridlines);
	Show_dock_points = GetProfileInt("Preferences", "Show dock points", Show_dock_points);
	Show_paths_fred = GetProfileInt("Preferences", "Show paths", Show_paths_fred);
	Lighting_on = GetProfileInt("Preferences", "Lighting On", Lighting_on);

	// Goober5000
	Format_fs2_open = GetProfileInt("Preferences", "FS2 open format", Format_fs2_open);
	Format_fs2_retail = GetProfileInt("Preferences", "FS2 retail format", Format_fs2_retail);
	Format_fs1_retail = GetProfileInt("Preferences", "FS1 retail format", Format_fs1_retail);

	read_window("Main window", &Main_wnd_data);
	read_window("Ship window", &Ship_wnd_data);
	read_window("Wing window", &Wing_wnd_data);
	read_window("Waypoint window", &Waypoint_wnd_data);
	read_window("Object window", &Object_wnd_data);
	read_window("Mission goals window", &Mission_goals_wnd_data);
	read_window("Messages window", &Messages_wnd_data);
	read_window("Player window", &Player_wnd_data);
	read_window("Events window", &Events_wnd_data);
	read_window("Bg window", &Bg_wnd_data);
	read_window("Briefing window", &Briefing_wnd_data);
	read_window("Reinforcement window", &Reinforcement_wnd_data);
	read_window("Starfield window", &Starfield_wnd_data);
	read_window("Asteroid window", &Asteroid_wnd_data);
	read_window("Mission notes window", &Mission_notes_wnd_data);
	write_ini_file(1);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CFREDDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CFREDView));
	AddDocTemplate(pDocTemplate);

	/* Goober5000: um, no, because this is extremely annoying
	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);	*/

	// setup the fred exe directory so CFILE can init properly
	/*
	c = GetCommandLine();
	Assert(c != NULL);
	if(c == NULL){
		return FALSE;
	} 
	tok = strtok(c, " \n");
	Assert(tok != NULL);
	if(tok == NULL){
		return FALSE;		
	}
	// Fred_exe_dir = strdup(c);		
	strcpy(Fred_exe_dir, tok);
	*/
	strcpy(Fred_exe_dir, __argv[0]);
	strcpy(Fred_base_dir, Fred_exe_dir);

	char *str_end = Fred_base_dir + strlen(Fred_base_dir) - 1; // last char

	while (*str_end != '//' && *str_end != '\\')
		str_end--;
	*str_end = '\0';


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);	

	m_nCmdShow = Main_wnd_data.p.showCmd;

	OnFileNew();

	if(m_pMainWnd == NULL) return FALSE;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	h_cursor_move = LoadCursor(IDC_CURSOR1);
	h_cursor_rotate = LoadCursor(IDC_CURSOR2);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnBug();
	afx_msg void OnForums();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_BUG, OnBug)
	ON_BN_CLICKED(IDC_FORUMS, OnForums)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CFREDApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CFREDApp commands

char *edit_mode_text[] = {
	"Ships",
	"Waypoints",
};
/*	"Grid",
	"Wing",
	"Object Relative"
};
*/

char *control_mode_text[] = {
	"Camera",
	"Object"
};

void show_control_mode(void)
{
	CString	str;

	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	CStatusBar* pStatus = &pFrame->m_wndStatusBar;
	//CStatusBarCtrl pStatusBarCtrl;

	if (pStatus) {
//		pStatus->GetStatusBarCtrl().SetParts(NUM_STATUS_PARTS, parts);

		if (Marked)
			str.Format("Marked: %d", Marked);
		else
			str = _T("");
		pStatus->SetPaneText(1, str);

		if (viewpoint)
			str.Format("Viewpoint: %s", object_name(view_obj));
		else
			str.Format("Viewpoint: Camera");

		pStatus->SetPaneText(2, str);
		
		if (FREDDoc_ptr->IsModified())
			pStatus->SetPaneText(3, "MODIFIED");
		else
			pStatus->SetPaneText(3, "");

		str.Format("Units = %.1f Meters", The_grid->square_size);
		pStatus->SetPaneText(4, str);

//		pStatus->SetPaneText(4, "abcdefg");
//		pStatus->SetPaneText(4, "1234567890!");
	}
	
}

#define	MAX_PENDING_MESSAGES	16

typedef struct {
	int	frame_to_process, hwnd, id, wparam, lparam;
} pending_message;

pending_message Pending_messages[MAX_PENDING_MESSAGES];

//	Process messages that needed to wait until a frame had gone by.
void process_pending_messages(void)
{
	int	i;

	for (i=0; i<MAX_PENDING_MESSAGES; i++)
		if (Pending_messages[i].frame_to_process != -1)
			if (Pending_messages[i].frame_to_process <= FrameCount) {
				pending_message	*pmp = &Pending_messages[i];
				PostMessage((HWND) pmp->hwnd, pmp->id, pmp->wparam, pmp->lparam);
				Pending_messages[i].frame_to_process = -1;
			}
}

//	Add a message to be processed to a buffer.
//	Wait skip_count frames before processing.
void add_pending_message(HWND hwnd, int id, int wparam, int lparam, int skip_count)
{
	int	i;

	for (i=0; i<MAX_PENDING_MESSAGES; i++)
		if (Pending_messages[i].frame_to_process == -1) {
			Pending_messages[i].hwnd = (int) hwnd;
			Pending_messages[i].id = id;
			Pending_messages[i].wparam = wparam;
			Pending_messages[i].lparam = lparam;
			Pending_messages[i].frame_to_process = FrameCount + skip_count;
		}
}

void init_pending_messages(void)
{
	int	i;

	for (i=0; i<MAX_PENDING_MESSAGES; i++)
		Pending_messages[i].frame_to_process = -1;
}

// void win32_blit(HDC hSrcDC, HPALETTE hPalette, int x, int y, int w, int h )
#if 0
void win32_blit(void *xx, void *yy, int x, int y, int w, int h )
{
	HPALETTE hOldPalette = NULL;
	HDC hdc = GetDC(hwndApp);

	if ( !hdc )	return;
	if ( !fAppActive ) return;

	if (hPalette)	{
		hOldPalette = SelectPalette(hdc, hPalette, FALSE);
		RealizePalette( hdc );
	}

	BitBlt(hdc, 0, 0, w, h, hSrcDC, x, y, SRCCOPY);
	
	if ( hOldPalette )	
		SelectPalette(hdc, hOldPalette, FALSE);

	ReleaseDC( hwndApp, hdc );
}
#endif

BOOL CFREDApp::OnIdle(LONG lCount)
{
	int adjust = 0;
	CWnd *top, *wnd;

	if (!Show_sexp_help)
		adjust = -SEXP_HELP_BOX_SIZE;

	if (!app_init) {
		app_init = 1;
		theApp.init_window(&Ship_wnd_data, &Ship_editor_dialog, adjust, 1);
		theApp.init_window(&Wing_wnd_data, &Wing_editor_dialog, adjust, 1);
		theApp.init_window(&Waypoint_wnd_data, &Waypoint_editor_dialog, 0, 1);
		init_window(&Main_wnd_data, Fred_main_wnd);
		Fred_main_wnd->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		Ship_editor_dialog.calc_cue_height();
		Wing_editor_dialog.calc_cue_height();
	}

	CWinApp::OnIdle(lCount);
	internal_integrity_check();
	if (Update_ship) {
		Ship_editor_dialog.initialize_data(1);
		Update_ship = 0;
	}

	if (Update_wing) {
		Wing_editor_dialog.initialize_data(1);
		Update_wing = 0;
	}

	Prev_window = CFREDView::GetActiveWindow();

	// Find the root window of the active window
	wnd = top = Prev_window;
	while (wnd) {
		top = wnd;
		wnd = wnd->GetParent();
	}

	// See if the active window is a child of Fred
	if (Prev_window)
		Fred_active = ( (top == Fred_main_wnd) || (top == Campaign_wnd) );
	else
		Fred_active = 0;

	if (!Fred_active)
		return FALSE;  // if fred isn't active, don't waste any time with it.

	game_do_frame();  // do stuff that needs doing, whether we render or not.
	show_control_mode();

  	if (!Update_window)
  		return FALSE;

	render_frame();	// "do the rendering!"  Renders image to offscreen buffer
	process_pending_messages();

	FrameCount++;
	return TRUE;
}

void update_map_window()
{
	if (Fred_active) {
		Update_window++;  // on idle will handle the drawing already.
		return;
	}

	if (!Fred_main_wnd)
		return;

	render_frame();	// "do the rendering!"

  	gr_flip();

	show_control_mode();
	process_pending_messages();

	FrameCount++;
}

void CFREDApp::write_ini_file(int degree)
{
	WriteProfileInt("Preferences", "User interface", User_interface);
	WriteProfileInt("Preferences", "Show stars", Show_stars);
	WriteProfileInt("Preferences", "Show grid positions", Show_grid_positions);
	WriteProfileInt("Preferences", "Show coordinates", Show_coordinates);
	WriteProfileInt("Preferences", "Show compass", Show_compass);
	WriteProfileInt("Preferences", "Show ship models", Show_ship_models);
	WriteProfileInt("Preferences", "Show ship info", Show_ship_info);
	WriteProfileInt("Preferences", "Show outlines", Show_outlines);
	WriteProfileInt("Preferences", "Physics speed", physics_speed);
	WriteProfileInt("Preferences", "Physics rotation", physics_rot);
	WriteProfileInt("Preferences", "Show waypoints", Show_waypoints);
	WriteProfileInt("Preferences", "Show sexp help", Show_sexp_help);
	WriteProfileInt("Preferences", "Hide ship cues", Hide_ship_cues);
	WriteProfileInt("Preferences", "Hide wing cues", Hide_wing_cues);
	WriteProfileInt("Preferences", "Autosave disabled", Autosave_disabled);
	WriteProfileInt("Preferences", "Double fine gridlines", double_fine_gridlines);
	WriteProfileInt("Preferences", "Anti aliased gridlines", Aa_gridlines);
	WriteProfileInt("Preferences", "Show dock points",	Show_dock_points);
	WriteProfileInt("Preferences", "Show paths",	Show_paths_fred);
	WriteProfileInt("Preferences", "Lighting On",	Lighting_on);

	// Goober5000
	WriteProfileInt("Preferences", "FS2 open format", Format_fs2_open);
	WriteProfileInt("Preferences", "FS2 retail format", Format_fs2_retail);
	WriteProfileInt("Preferences", "FS1 retail format", Format_fs1_retail);

	if (!degree) {
		record_window_data(&Waypoint_wnd_data, &Waypoint_editor_dialog);
		record_window_data(&Wing_wnd_data, &Wing_editor_dialog);
		record_window_data(&Ship_wnd_data, &Ship_editor_dialog);
		record_window_data(&Main_wnd_data, Fred_main_wnd);

		write_window("Main window", &Main_wnd_data);
		write_window("Ship window", &Ship_wnd_data);
		write_window("Wing window", &Wing_wnd_data);
		write_window("Waypoint window", &Waypoint_wnd_data);
		write_window("Object window", &Object_wnd_data);
		write_window("Mission goals window", &Mission_goals_wnd_data);
		write_window("Messages window", &Messages_wnd_data);
		write_window("Player window", &Player_wnd_data);
		write_window("Events window", &Events_wnd_data);
		write_window("Bg window", &Bg_wnd_data);
		write_window("Briefing window", &Briefing_wnd_data);
		write_window("Reinforcement window", &Reinforcement_wnd_data);
		write_window("Starfield window", &Starfield_wnd_data);
		write_window("Asteroid window", &Asteroid_wnd_data);
		write_window("Mission notes window", &Mission_notes_wnd_data);
	}
}

void CFREDApp::write_window(char *name, window_data *wndd)
{
	WriteProfileInt(name, "valid", wndd->valid);
	WriteProfileInt(name, "length", wndd->p.length);
	WriteProfileInt(name, "flags", wndd->p.flags);
	WriteProfileInt(name, "showCmd", wndd->p.showCmd);
	WriteProfileInt(name, "ptMinPosition.x", wndd->p.ptMinPosition.x);
	WriteProfileInt(name, "ptMinPosition.y", wndd->p.ptMinPosition.y);
	WriteProfileInt(name, "ptMaxPosition.x", wndd->p.ptMaxPosition.x);
	WriteProfileInt(name, "ptMaxPosition.y", wndd->p.ptMaxPosition.y);
	WriteProfileInt(name, "rcNormalPosition.left", wndd->p.rcNormalPosition.left);
	WriteProfileInt(name, "rcNormalPosition.top", wndd->p.rcNormalPosition.top);
	WriteProfileInt(name, "rcNormalPosition.right", wndd->p.rcNormalPosition.right);
	WriteProfileInt(name, "rcNormalPosition.bottom", wndd->p.rcNormalPosition.bottom);
	WriteProfileInt(name, "Visible", wndd->visible);
}

void CFREDApp::read_window(char *name, window_data *wndd)
{
	wndd->processed = 0;
	wndd->valid = GetProfileInt(name, "valid", FALSE);
	wndd->p.length = GetProfileInt(name, "length", 0);
	wndd->p.flags = GetProfileInt(name, "flags", 0);
	wndd->p.showCmd = GetProfileInt(name, "showCmd", SW_SHOWMAXIMIZED);
	wndd->p.ptMinPosition.x = GetProfileInt(name, "ptMinPosition.x", 0);
	wndd->p.ptMinPosition.y = GetProfileInt(name, "ptMinPosition.y", 0);
	wndd->p.ptMaxPosition.x = GetProfileInt(name, "ptMaxPosition.x", 0);
	wndd->p.ptMaxPosition.y = GetProfileInt(name, "ptMaxPosition.y", 0);
	wndd->p.rcNormalPosition.left = GetProfileInt(name, "rcNormalPosition.left", 0);
	wndd->p.rcNormalPosition.top = GetProfileInt(name, "rcNormalPosition.top", 0);
	wndd->p.rcNormalPosition.right = GetProfileInt(name, "rcNormalPosition.right", 0);
	wndd->p.rcNormalPosition.bottom = GetProfileInt(name, "rcNormalPosition.bottom", 0);
	wndd->visible = GetProfileInt(name, "Visible", 1);
}

int CFREDApp::init_window(window_data *wndd, CWnd *wnd, int adjust, int pre)
{
	int width, height;
	WINDOWPLACEMENT p;

	if (pre && !wndd->visible)
		return -1;

	if (wndd->processed)
		return -2;

	Assert(wnd->GetSafeHwnd());
	wnd->GetWindowPlacement(&p);
	width = p.rcNormalPosition.right - p.rcNormalPosition.left;
	height = p.rcNormalPosition.bottom - p.rcNormalPosition.top + adjust;
	wndd->p.rcNormalPosition.right = wndd->p.rcNormalPosition.left + width;
	wndd->p.rcNormalPosition.bottom = wndd->p.rcNormalPosition.top + height;

	if (wndd->valid) {
		wnd->SetWindowPlacement(&wndd->p);
//		if (!wndd->visible)
//			wnd->ShowWindow(SW_SHOW);
//		else
//			wnd->ShowWindow(SW_HIDE);
	}

	record_window_data(wndd, wnd);
	wndd->processed = 1;
	return 0;
}

void CFREDApp::record_window_data(window_data *wndd, CWnd *wnd)
{
	wnd->GetWindowPlacement(&wndd->p);
	wndd->visible = wnd->IsWindowVisible();
	wndd->valid = TRUE;
}

void CAboutDlg::OnBug() 
{
	char *path = "http://mgo.maxgaming.net/mantis/login_page.php";

	char buffer[MAX_PATH];
	sprintf(buffer,"explorer.exe \"%s\"", path);

	WinExec(buffer,SW_SHOW);
}

void CAboutDlg::OnForums() 
{
	char *path = "http://dynamic4.gamespy.com/~freespace/forums/";

	char buffer[MAX_PATH];
	sprintf(buffer,"explorer.exe \"%s\"", path);

	WinExec(buffer,SW_SHOW);	
}

int CFREDApp::ExitInstance() 
{

	return CWinApp::ExitInstance();
}
