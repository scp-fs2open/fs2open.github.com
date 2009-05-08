/* 3DFXDisp.cpp : implementation file
 *
 * Subtab of the Video tab, handles glide 3dfx mode selection.
 * Does not initialise glide, options are avaliable purely on the basis of what
 * 'should' be assuming system supports glide
 *
 * Very similar to the DX5 tab but kept seperate for easy of expansion
 */


#include "stdafx.h"
#include "launcher.h"
#include "3DFXDisp.h"

#include "win32func.h"
#include "launcher_settings.h"

typedef struct
{
	int xres;
	int yres;
	char *text_desc;

} Mode3DFX;

// Glide only allows 16 bit so only two modes possible
const int NUM_3DFX_MODES = 2;

Mode3DFX glide_modes[NUM_3DFX_MODES] =
{
	1024, 768, "3DFX Glide (1024x768)",
	 640, 480, "3DFX Glide (640x480)",
};

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// C3DFXDisp dialog


C3DFXDisp::C3DFXDisp(CWnd* pParent /*=NULL*/)
	: CDialog(C3DFXDisp::IDD, pParent)
{
	//{{AFX_DATA_INIT(C3DFXDisp)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void C3DFXDisp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(C3DFXDisp)
	DDX_Control(pDX, IDC_GLIDE_RES, m_res_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(C3DFXDisp, CDialog)
	//{{AFX_MSG_MAP(C3DFXDisp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// C3DFXDisp message handlers

/** 
 * Initialise the dialog ready for viewing
 */
BOOL C3DFXDisp::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateResList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/**
 * Applies setting cahnges to registry
 *
 * @param char *reg_path - Registry path to apply changes to
 */
void C3DFXDisp::OnApply()
{
	// Now apply video mode
	int index = m_res_list.GetCurSel();

	if( index == CB_ERR)
	{
		MessageBox("Failed to set graphic mode", "Error", MB_ICONERROR);
		return;
	}

	int current = m_res_list.GetItemData(index);

	if(reg_set_sz(LauncherSettings::get_reg_path(), "Videocard", glide_modes[index].text_desc) == false)
	{
		MessageBox("Failed to set graphic mode", "Error", MB_ICONERROR);
	}

	if(reg_set_sz(LauncherSettings::get_reg_path(), "VideocardFs2open", glide_modes[index].text_desc) == false)
	{
		MessageBox("Failed to set graphic mode", "Error", MB_ICONERROR);
	}
}

/**
 * Refreshes the list of modes avaliable, this depends on variables that have already 
 * been set using other functions of this class.
 */
void C3DFXDisp::UpdateResList()
{
	int count = 0;
	char res_text[20];
 	m_res_list.ResetContent();

	for(int i = 0; i < NUM_3DFX_MODES; i++)
	{
		sprintf(res_text, "%d x %d", glide_modes[i].xres, glide_modes[i].yres);

		// Store the index
		int index = m_res_list.InsertString(count, res_text);
		m_res_list.SetItemData(index, count);
		count++;
	}

	m_res_list.SetCurSel(0);
}

/**
 * Determines the current settings in the registry and selects them in the list if avaliable
 *
 * @param char *reg_path - Registry path to obtain info from
 */
void C3DFXDisp::LoadSettings()
{
	char videocard_string[MAX_PATH];

	// Lets get those video card settings
	if(reg_get_sz(LauncherSettings::get_reg_path(), "VideocardFs2open", videocard_string, MAX_PATH) == false)
	{
		return;
	}

	int count = 0;
	for(int i = 0; i < NUM_3DFX_MODES; i++)
	{
		if(strcmp(videocard_string, glide_modes[i].text_desc) == 0)
		{
			m_res_list.SetCurSel(count);
			return;
		}

		count++;
	}

	m_res_list.SetCurSel(0);
}			 
