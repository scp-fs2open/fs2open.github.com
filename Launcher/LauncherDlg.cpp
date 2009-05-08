// LauncherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Launcher.h"
#include "LauncherDlg.h"

#include "win32func.h"
#include "misc.h"
#include "launcher_settings.h"

#include "iniparser/iniparser.h"
#include "iniparser/dictionary.h"

CLauncherDlg *CLauncherDlg::pthis = NULL;


CTabCommLine   tab_comm_line;
CTabHelp	   tab_help;
CTabMOD	       tab_mod;
CTabRegOptions tab_reg_options;
CTabVideo	   tab_video;
CTabSound	   tab_sound;
CTabSpeech	   tab_speech;
CTabNetwork	   tab_network;
int reg_sheet_index = -1;

const int NUM_TABS = 8;
CDialog *tab_dialogs[NUM_TABS];

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLauncherDlg dialog

CLauncherDlg::CLauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLauncherDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLauncherDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	currently_selected_tab = -1;

	pthis = this;
}

// Force the whole app to redraw
void CLauncherDlg::Redraw()
{
	if(pthis)
	{
		pthis->RedrawWindow();
	}
}


void CLauncherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLauncherDlg)
	DDX_Control(pDX, IDC_HOLDER, m_tabctrl);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLauncherDlg, CDialog)
	//{{AFX_MSG_MAP(CLauncherDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	ON_NOTIFY(TCN_SELCHANGE, IDC_HOLDER, OnSelchangeHolder)
	ON_BN_CLICKED(IDC_RUN, OnRun)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLauncherDlg message handlers

// Global variables 
 
/**
 * Initialises the tab control system including:
 *
 * Taking tab strings from the 'String Table' in resources
 * Create all dialogs ready for display
 *
 */
BOOL CLauncherDlg::InitTabControl()
{
	// Prepare the inferface
    TCITEM tab; 
	// Add tabs for each string 
    tab.mask = TCIF_TEXT | TCIF_IMAGE; 
    tab.iImage = -1; 

    for (int i = 0; i < NUM_TABS; i++) 
	{ 
        CString tab_name;
		tab_name.LoadString(IDS_1ST_TAB + i);

		tab.pszText = (LPTSTR) (LPCTSTR) tab_name;
	   	m_tabctrl.InsertItem(i, &tab); 

		tab_dialogs[i] = NULL;
	}

	// Prepare the dialog sheets
	tab_comm_line.Create(IDD_COMM_LINE,	GetDlgItem(IDC_DLG_HOLDER));
  	tab_help.Create(IDD_HELP, GetDlgItem(IDC_DLG_HOLDER));
  	tab_mod.Create(IDD_MOD, GetDlgItem(IDC_DLG_HOLDER));
	tab_reg_options.Create(IDD_REG_OPTIONS,	GetDlgItem(IDC_DLG_HOLDER));
	tab_video.Create(IDD_VIDEO, GetDlgItem(IDC_DLG_HOLDER));
	tab_sound.Create(IDD_SOUND, GetDlgItem(IDC_DLG_HOLDER));
	tab_speech.Create(IDD_SPEECH, GetDlgItem(IDC_DLG_HOLDER));
	tab_network.Create(IDD_NETWORK, GetDlgItem(IDC_DLG_HOLDER));

	int t = 0;
  	tab_dialogs[t++] = &tab_help;
	tab_dialogs[t++] = &tab_comm_line;
	tab_dialogs[t++] = &tab_mod;	
	tab_dialogs[t++] = &tab_video;
	tab_dialogs[t++] = &tab_sound;
	tab_dialogs[t++] = &tab_speech;
	tab_dialogs[t++] = &tab_network;
	reg_sheet_index  = t;
	tab_dialogs[t++] = &tab_reg_options;

	return TRUE;
}

BOOL CLauncherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	InitCommonControls();
	InitTabControl();

	LauncherSettings::load();

	char alt_path[MAX_PATH];
	char *exe_path = LauncherSettings::get_exe_filepath();

	// If no settings file exists use default settings
	if(!*LauncherSettings::get_exe_filepath())
	{
		const int MAX_FILENAME_TRIES = 6;
		char *filename_tries[MAX_FILENAME_TRIES] =
		{
			"fs2_open_r.exe",
			"fs2_open_trunk_r.exe",
			"fs2_open.exe",
			"fs2_open_trunk.exe",
			"fs2_open_d.exe",
			"fs2_open_trunk_d.exe",
		};

		bool found = false;
		for(int i = 0; i < MAX_FILENAME_TRIES; i++)
		{
			GetCurrentDirectory(MAX_PATH, alt_path);
			int len = strlen(alt_path);
			alt_path[len] = '\\';
			
			strcpy(alt_path + len + 1, filename_tries[i]); 
			
			if(file_exists(alt_path)) 
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			*alt_path = '\0';
		}

		exe_path = alt_path;
	}

	NewExeSet(exe_path);
	SelectTab();
	tab_comm_line.UpdateFields();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLauncherDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

/**
 * If you add a minimize button to your dialog, you will need the code below
 *  to draw the icon.  For MFC applications using the document/view model,
 *  this is automatically done for you by the framework.
 */
void CLauncherDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLauncherDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

/**
 * Leave the launcher but save any setting changes first
 */
void CLauncherDlg::OnOK() 
{
	OnApply();
	CDialog::OnOK();
}

/**
 * Leave the launcher without saving any settings
 */
void CLauncherDlg::OnCancel() 
{
	CDialog::OnCancel();
}

/**
 * The user has chosen to accept these settings
 */
void CLauncherDlg::OnApply() 
{
	int flags = tab_comm_line.GetFlags(); 

	// Save Command line settings
	tab_comm_line.SaveSettings();

	// Save tab settings
	if(LauncherSettings::get_exe_type() == EXE_TYPE_CUSTOM || flags & FLAG_FS2)
	{
		tab_video.OnApply(flags);
		tab_sound.OnApply();
		tab_speech.OnApply();
		tab_network.OnApply();
		// Call mod apply from inside comm line	
	}

	LauncherSettings::save();
}

void CLauncherDlg::OnSelchangeHolder(NMHDR* pNMHDR, LRESULT* pResult) 
{
	SelectTab(m_tabctrl.GetCurSel());
}

void CLauncherDlg::SelectTab(int selected_tab)
{
	if(selected_tab >= NUM_TABS)
	{
		return;
	}

	// Hide all windows not in use
	for(int i = 0; i < NUM_TABS; i++)
	{
		if(tab_dialogs[i])
		{
			tab_dialogs[i]->ShowWindow(SW_HIDE);
		}
	}

 	if(selected_tab == reg_sheet_index)
	{			   
		tab_reg_options.FillRegList();
	}					 

	// If tab control exists show it
	if(tab_dialogs[selected_tab] != NULL)
	{
		GetDlgItem(IDC_DLG_HOLDER)->ShowWindow(SW_SHOW);
		tab_dialogs[selected_tab]->ShowWindow(SW_SHOW);
		currently_selected_tab = selected_tab;
	}
	// Otherwise hide the whole tab
	else if(currently_selected_tab != -1)
	{
		GetDlgItem(IDC_DLG_HOLDER)->ShowWindow(SW_HIDE);
	}
}

/**
 * Try to run the selected exe with the selected command line options	
 */
void CLauncherDlg::OnRun() 
{
	OnApply();

	bool result = run_file( 
		(LPTSTR) LauncherSettings::get_exe_nameonly(), 
		(LPTSTR) LauncherSettings::get_exe_pathonly(),
		// our cmdline can be *really* long, but we can only safely pass MAX_PATH
		// worth of it due to OS issues.  instead just pass NULL and let cmdline_fso.cfg
		// handle actually making use of all the options
		/*(LPTSTR) (LPCTSTR) tab_comm_line.GetCommLine()*/NULL);

	if(!result)
	{
		char buffer[1000];
		sprintf(buffer,"Failed to run exe: %s %s %s",
			(LPTSTR) LauncherSettings::get_exe_nameonly(), 
			(LPTSTR) LauncherSettings::get_exe_pathonly(),
			(LPTSTR) (LPCTSTR) tab_comm_line.GetCommLine());
		MessageBox(buffer);
	}
}

/**
 * Allows user to browse for a new exe
 */
void CLauncherDlg::OnBrowse() 
{
	char new_file[MAX_PATH];
	char new_path[MAX_PATH] = "";
	char current_path[MAX_PATH];
	OPENFILENAME details;

	// save our current settings, since they will get overwritten otherwise
	tab_comm_line.SaveSettings();

	GetCurrentDirectory(MAX_PATH, current_path);
	memset(&details, 0, sizeof(OPENFILENAME));
	details.lStructSize       = sizeof(OPENFILENAME); 
    details.hwndOwner         = this->GetSafeHwnd(); 
    details.lpstrFilter       = 
		"FreeSpace2 Open exe\0*fs2_open*.exe\0"
		"Official retail exe\0FS2.exe;FS2Demo.exe\0"
		"Any exe\0*.exe\0"; 

    details.lpstrFileTitle    = new_file;
    details.nMaxFileTitle 	  = MAX_PATH;
    details.lpstrInitialDir   = current_path;
    details.lpstrTitle        = "Open FreeSpace exe"; 
    details.Flags			  = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR; 

	details.lpstrFile         = new_path;
	details.nMaxFile          = MAX_PATH;

	if(GetOpenFileName(&details) == TRUE)
	{
		NewExeSet(new_path);
	}
}

/**
 * Use this to change selected exe to a new file
 *
 */
void CLauncherDlg::NewExeSet(char *exe_path)
{
	// Check dir and set exe
	if (strlen(exe_path) > 0) {
		bool good_path = false;
		char current_dir[MAX_PATH], just_path[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, current_dir);

		strcpy(just_path, exe_path);
		char *last_slash = strrchr(just_path, '\\');

		if (last_slash)
			*last_slash = '\0';


		if ( !last_slash || !stricmp(current_dir, just_path) ) {
			if ( !last_slash ) {
				sprintf(just_path, "%s\\%s", current_dir, exe_path);

				if ( file_exists(just_path) ) {
					LauncherSettings::set_exe_filepath(just_path);
					good_path = true;
				}
			} else {
				LauncherSettings::set_exe_filepath(exe_path);
				good_path = true;
			}
		}

		if (!good_path) {
			if ( MessageBox("The launcher must be in the same directory as the binary you are trying to use.",
					"Error", MB_OK | MB_ICONERROR) == IDOK )
			{
				if (last_slash) {
					sprintf( just_path, "%s\\%s", current_dir, exe_path + (strlen(just_path) + 1) );

					if ( file_exists(just_path) ) {
						MessageBox("Using binary in the launcher directory instead.", "Notice", MB_OK | MB_ICONINFORMATION);
						LauncherSettings::set_exe_filepath(just_path);
					}
				} else {
					LauncherSettings::set_reg_path("", EXE_TYPE_NONE);
				}
			} else {
				LauncherSettings::set_reg_path("", EXE_TYPE_NONE);
			}
		}
	} else {
		LauncherSettings::set_reg_path("", EXE_TYPE_NONE);
	}

	// Setup reg dir
	tab_comm_line.SelectRegPathAndExeType();				

	// Update the options
	tab_comm_line.UpdateFields();
	tab_comm_line.ConstructFlagList();

	GetDlgItem(IDC_RUN)->EnableWindow( LauncherSettings::is_exe_path_valid() );
	GetDlgItem(IDC_PATH)->SetWindowText(LauncherSettings::get_exe_filepath());	

	tab_video.Update(LauncherSettings::get_exe_type(), tab_comm_line.GetFlags());

	// is this necessary? - G5K
	// tab_comm_line.LoadSettings(LauncherSettings::get_reg_path());

	for(int i = 2; i < NUM_TABS-1; i++)
	{
		tab_dialogs[i] = NULL;
	}

	int flags = tab_comm_line.GetFlags();
	if(LauncherSettings::get_exe_type() == EXE_TYPE_CUSTOM || flags & FLAG_FS2)
	{
		tab_video.LoadSettings(flags);	
		tab_sound.LoadSettings();
		tab_speech.LoadSettings();
		tab_network.LoadSettings();

		int t = 2;
		tab_dialogs[t++] = &tab_mod;
		tab_dialogs[t++] = &tab_video;
		tab_dialogs[t++] = &tab_sound;
		tab_dialogs[t++] = &tab_speech;
		tab_dialogs[t++] = &tab_network;
	}
	
	m_tabctrl.SetCurSel(0);
	SelectTab(0);
}
