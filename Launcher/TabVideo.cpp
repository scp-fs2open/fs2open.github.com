// TabVideo.cpp : implementation file
//

#include "stdafx.h"
#include "Launcher.h"
#include "TabCommLine.h"
#include "TabVideo.h"
#include "DX9Disp.h"
#include "DX8Disp.h"
#include "DX5Disp.h"
#include "OGLDisp.h"
#include "3DFXDisp.h"

#include "win32func.h"
#include "misc.h"
#include "settings.h"

CDX9Disp  tab_dx9_disp;
CDX8Disp  tab_dx8_disp;
CDX5Disp  tab_dx5_disp;
COGLDisp  tab_ogl_disp;
C3DFXDisp tab_3dfx_disp;

const int NUM_LEVELS = 4;

// These are the modes recognised (or not) by the launcher
// note values to not corrispond to those in fs2 code
enum
{
//	GR_DIRECT3D9,		// Use Direct3d hardware renderer
	GR_OPENGL,			// Use OpenGl hardware renderer
	GR_DIRECT3D8,		// Use Direct3d hardware renderer
	GR_DIRECT3D5,		// Use Direct3d hardware renderer
	GR_GLIDE,			// Use Glide hardware renderer
	GR_DIRECTDRAW,		// Software renderer using DirectDraw fullscreen.
	GR_SOFTWARE,		// Software renderer using standard Win32 functions in a window.
	MAX_GR_TYPES
};

char *api_text_desc[MAX_GR_TYPES] = 
{
//	"Direct3D9",
	"OpenGL",
	"Direct3D8",
	"Direct3D5",
	"Glide",
	"DirectDraw",
	"Software",
};

char *reg_api_name[MAX_GR_TYPES] = 
{
//	"D3D9",
	"OGL ",
	"D3D8",
	"Direct 3D",
	"3DFX Glide",
	"DirectDraw",
	"Software",
};

bool api_available[MAX_GR_TYPES];

CDialog *api_dialogs[MAX_GR_TYPES];

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabVideo dialog

// Constructor 
CTabVideo::CTabVideo(CWnd* pParent /*=NULL*/)
	: CDialog(CTabVideo::IDD, pParent)
{
	m_last_tab = -1;
	//{{AFX_DATA_INIT(CTabVideo)
	//}}AFX_DATA_INIT
}


void CTabVideo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabVideo)
	DDX_Control(pDX, IDC_GF4_FFIX, m_gf4_fix_button);
	DDX_Control(pDX, IDC_API_HOLDER, m_api_list);
	DDX_Control(pDX, IDC_LTXT_CHECKBOX, m_largetxt_checkbox);
	DDX_Control(pDX, IDC_HI_SPARKY_CHECKBOX, m_hi_sparky_checkbox);
	DDX_Control(pDX, IDC_SETTINGS_LIST, m_gfx_level_droplist);
	DDX_Control(pDX, IDC_GFXAPI_LIST, m_api_droplist);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabVideo, CDialog)
	//{{AFX_MSG_MAP(CTabVideo)
	ON_CBN_SELCHANGE(IDC_GFXAPI_LIST, OnSelchangeGfxapiList)
	ON_BN_CLICKED(IDC_GF4_FFIX, OnGf4Ffix)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabVideo message handlers

/**
 * Called when a graphics API is selected, should display the options relevent to that system
 */
void CTabVideo::OnSelchangeGfxapiList() 
{
	CString api_choice;
	m_api_droplist.GetLBText(m_api_droplist.GetCurSel(), api_choice);

	for(int i = 0; i < MAX_GR_TYPES; i++)
	{
		if(	api_choice.CompareNoCase(api_text_desc[i]) == 0)
		{
			SelectTab(i);
			return;
		}
	}

	MessageBox("API selection error!");
}

/**
 * Called once when the dialog is initialised
 */
BOOL CTabVideo::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	((CComboBox *) GetDlgItem(IDC_GFXAPI_LIST))->SetCurSel(0);
	((CComboBox *) GetDlgItem(IDC_SETTINGS_LIST))->SetCurSel(0);

	// Check the validity of this option
	if(m_gfx_level_droplist.GetCount() != NUM_LEVELS)
	{
		MessageBox("Coder error: Level of detail code has not been updated with options");
		return FALSE;
	}

	// Prepare the dialog sheets
	tab_dx5_disp.Create(IDD_DX5, GetDlgItem(IDC_API_HOLDER));
	tab_ogl_disp.Create(IDD_OGL, GetDlgItem(IDC_API_HOLDER));
	tab_3dfx_disp.Create(IDD_3DFX, GetDlgItem(IDC_API_HOLDER));
	tab_dx8_disp.Create(IDD_DX8, GetDlgItem(IDC_API_HOLDER));
//	tab_dx9_disp.Create(IDD_DX9, GetDlgItem(IDC_API_HOLDER));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/**
 * Prepare the inner tab control
 *
 * @return BOOL - TRUE or FALSE
 */
BOOL CTabVideo::InitTabControl(int type)
{
	int count = 0;

	m_api_droplist.ResetContent();
	for(int i = 0; i < MAX_GR_TYPES; i++)
	{
		api_dialogs[i]   = NULL;
		api_available[i] = (type & (1 << i)) > 0;

	 	if(api_available[i])
		{
			m_api_droplist.InsertString(count, api_text_desc[i]);
			count++;
		}
	}

    count = 0;
//	api_dialogs[count] = &tab_dx9_disp;
//	count++;
	api_dialogs[count] = &tab_ogl_disp;
	count++;
	api_dialogs[count] = &tab_dx8_disp;
	count++;
  	api_dialogs[count] = &tab_dx5_disp;
	count++;
	api_dialogs[count] = &tab_3dfx_disp;

	SelectTab(SelectAPI(0));

	return TRUE;
}

/**
 * Set inner video tab
 *
 * @param int select - Tab to select and display
 */
void CTabVideo::SelectTab(int select)
{
	// If the last tab is still shown hide it
	if(m_last_tab >= 0)
	{
		api_dialogs[m_last_tab]->ShowWindow(SW_HIDE);
	}

	// Bail here if we don't want to display any new tab
	if(select < 0 || select > MAX_GR_TYPES || api_dialogs[select] == NULL)
	{
		m_last_tab = -1;
		return;
	}

	// If valid tab number and details are filled in
	if((select < MAX_GR_TYPES) && (api_dialogs[select] != NULL))
	{
		api_dialogs[select]->ShowWindow(SW_SHOW);
	}

	m_last_tab = select;
}

/**
 * The user has chosen to accept these settings
 *
 * @param char *reg_path - Registry path that any settings should be saved to
 */
void CTabVideo::OnApply(int flags)
{
	// Take note of checkbox options
	int use_gf4_fix = (m_gf4_fix_button.GetCheck() == 0) ? 0 : 1;
		
	reg_set_dword(Settings::reg_path, "D3DTextureOrigin", use_gf4_fix);

	reg_set_dword(Settings::reg_path, "D3DFast", 0);
	reg_set_dword(Settings::reg_path, "ComputerSpeed", NUM_LEVELS - m_gfx_level_droplist.GetCurSel());

	int use_large_textures = (m_largetxt_checkbox.GetCheck() == 0) ? 0 : 2;
	reg_set_dword(Settings::reg_path, "D3DUseLargeTextures", use_large_textures);

	switch(m_last_tab)
	{
		case -1:			
			MessageBox("This API is not currently supported by the launcher"); 
			return;
		case GR_OPENGL:		tab_ogl_disp.OnApply();  break;	  
//		case GR_DIRECT3D9:	tab_dx9_disp.OnApply(flags);  break;
		case GR_DIRECT3D8:	tab_dx8_disp.OnApply(flags);  break;
		case GR_DIRECT3D5:	tab_dx5_disp.OnApply();  break;
		case GR_GLIDE:		tab_3dfx_disp.OnApply(); break;
	}
}

/**
 *
 * @param int type
 * @param char *path
 */
void CTabVideo::Update(int type, int flags)
{
	if(Settings::exe_path_valid == false)
	{
		SelectTab(-1);
		return;
	}

	char vp_path[MAX_PATH];
	strcpy(vp_path, Settings::exe_pathonly);
	strcat(vp_path, "\\sparky_hi_fs2.vp");

	InitTabControl(flags);

	tab_dx8_disp.UpdateAdapterList();
	tab_dx9_disp.UpdateAdapterList();
	tab_ogl_disp.UpdateLists();

	// Notify tabs that need to know if hi res pack is installed
	m_hi_sparky_checkbox.SetCheck((flags & FLAG_FS1) || Settings::exe_path_valid == true);
}

/**
 *
 * @param char *reg_path - Current registry path the function should look for data at
 */
void CTabVideo::LoadSettings(int flags)
{
	DWORD gf4_fix;
	if(reg_get_dword(Settings::reg_path, "D3DTextureOrigin", &gf4_fix) == true)
	{
		m_gf4_fix_button.SetCheck(gf4_fix ? 1 : 0);
	}	

	DWORD large_textures;
	if(reg_get_dword(Settings::reg_path, "D3DUseLargeTextures", &large_textures) == true)
	{
		m_largetxt_checkbox.SetCheck((large_textures == 2) ? 1 : 0);
	}

	// Determine general speed settings
	DWORD level_value;
	reg_get_dword(Settings::reg_path, "ComputerSpeed", &level_value);

	level_value = NUM_LEVELS - level_value;	
	m_gfx_level_droplist.SetCurSel(level_value);

	int api = 0;
	{
		char api_name[1024];
		// If fs2_open videocard is not setup
		if(reg_get_sz(Settings::reg_path, "VideocardFs2open", api_name, 1024) == false)
		{
			// Use default videocard entry
			if(reg_get_sz(Settings::reg_path, "Videocard", api_name, 1024) == false)
			{
				api = SelectAPI(GR_OPENGL);
				SelectTab(api);
				return;
			}
		}

		// Try and match the mode
		for(int i = 0; i < MAX_GR_TYPES; i++)
		{
			if(strstr(api_name, reg_api_name[i]) != NULL)
			{
				api = i;
				break;
			}
		}
	}

	api = SelectAPI(api);

	switch(api)
	{
		case GR_OPENGL:		tab_ogl_disp.LoadSettings(); break;	  
		case GR_GLIDE:		tab_3dfx_disp.LoadSettings(); break;
		case GR_DIRECT3D5:	tab_dx5_disp.LoadSettings(); break;
		case GR_DIRECT3D8:	tab_dx8_disp.LoadSettings(flags); break;
//		case GR_DIRECT3D9:	tab_dx9_disp.LoadSettings(flags); break;
	}

	SelectTab(api);
}


int CTabVideo::SelectAPI(int api)
{
	int i = 0;

	// This api may not be avaliable
	if(api_available[api] == false)
	{
		while(api_available[i] == false)
			i++;

		api = i;
	}

	int menu_value = api;

	for(i = 0; i < api; i++) {
		if(api_available[i] == false)
			menu_value--;
	}

	if(menu_value < 0)
		menu_value = 0;

	m_api_droplist.SetCurSel(menu_value);

	return api;
}

void CTabVideo::OnGf4Ffix() 
{
	if(((CButton*)GetDlgItem(IDC_GF4_FFIX))->GetCheck())
	{
		MessageBox(
			"Only use this if you actually have the font distortion problem that happens to some cards in D3D modes.", 
			"Info", MB_ICONINFORMATION);	
	}
}
