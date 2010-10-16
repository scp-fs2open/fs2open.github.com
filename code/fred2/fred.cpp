/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
#include "globalincs/mspdb_callstack.h"

#include "AFXADV.H"

#ifdef NDEBUG
#ifndef FRED
#error macro FRED is not defined when trying to build release Fred.  Please define FRED macro in build settings in all Fred projects
#endif
#endif

#ifdef _DEBUG
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
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFREDApp construction

CFREDApp::CFREDApp()
{
	app_init = 0;

	DBUGFILE_INIT();
	SCP_mspdbcs_Initialise( );

#ifndef NDEBUG
	outwnd_init();
#endif
}

CFREDApp::~CFREDApp()
{
	SCP_mspdbcs_Cleanup( );
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
//	_CrtSetDbgFlag(~(_CRTDBG_ALLOC_MEM_DF) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));

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
	strcpy_s(Fred_exe_dir, tok);
	*/

	// we need a full path, and if run from a shell that may not happen, so work that case out...

	Assert( strlen(__argv[0]) > 2 );

	// see if we have a ':', and if not then assume that we don't have a full path
	if (__argv[0][1] != ':') {
		GetCurrentDirectory( sizeof(Fred_exe_dir)-1, Fred_exe_dir );
		strcat_s(Fred_exe_dir, "\\");
		strcat_s(Fred_exe_dir, __argv[0]);
	} else {
		strcpy_s(Fred_exe_dir, __argv[0]);
	}

	strcpy_s(Fred_base_dir, Fred_exe_dir);

	char *str_end = Fred_base_dir + strlen(Fred_base_dir) - 1; // last char

	while (*str_end != '//' && *str_end != '\\')
		str_end--;
	*str_end = '\0';


	// Goober5000 - figure out where the FRED file dialog should go
	if ((m_pRecentFileList != NULL) && (m_pRecentFileList->GetSize() > 0))
	{
		// use the most recently opened file to supply the default folder
		m_sInitialDir = m_pRecentFileList->operator[](0);	// lol syntax
	}
	else
	{
		// use FRED's own directory to search for missions
		m_sInitialDir = Fred_base_dir;
		m_sInitialDir += "\\data\\missions";
	}


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

// G5K - from http://www.experts-exchange.com/Programming/System/Windows__Programming/MFC/Q_20155824.html
void CFREDApp::OnFileOpen()
{
    // use the initial dir the first time we prompt for a file
    // name. after a successful open, the file name will be
    // changed to be the empty string and the default dir
    // will be supplied by the CFileDialog internal memory
    if(!DoPromptFileName(m_sInitialDir, AFX_IDS_OPENFILE, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, NULL))
	{
        // open cancelled
        return;
    }
    AfxGetApp()->OpenDocumentFile(m_sInitialDir);
    // if returns NULL, the user has already been alerted

    // now erase the initial dir since we only want to use it once
    m_sInitialDir.Empty();
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
//	internal_integrity_check();
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
	char *path = "http://www.hard-light.net/forums/";

	char buffer[MAX_PATH];
	sprintf(buffer,"explorer.exe \"%s\"", path);

	WinExec(buffer,SW_SHOW);	
}

int CFREDApp::ExitInstance() 
{

	return CWinApp::ExitInstance();
}

// Empty functions to make fred link with the sexp_mission_set_subspace
void game_start_subspace_ambient_sound() {}
void game_stop_subspace_ambient_sound() {} 
