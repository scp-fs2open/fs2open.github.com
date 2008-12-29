/*
 * DX5Disp.cpp : implementation file
 *
 * Subtab of the Video tab, handles DX5 mode selection.
 * Does not initialise DX5, options are avaliable purely on the basis of what
 * 'should' be assuming system is setup correctly with DX5
 *
 * Very similar to the Glide tab but kept seperate for easy of expansion
 */

#include "stdafx.h"
#include "launcher.h"
#include "DX5Disp.h"

#include "win32func.h"
#include "settings.h"

typedef struct
{
	int xres;
	int yres;
	int cdepth;
	char *text_desc;

} ModeDX5;

const int NUM_DX5_MODES = 4;

ModeDX5 dx5_modes[NUM_DX5_MODES] =
{
	1024, 768, 32, "Direct 3D - Primary Display Driver (32 bit) (1024x768)",
	1024, 768, 16, "Direct 3D - Primary Display Driver (16 bit) (1024x768)",
	 640, 480, 32, "Direct 3D - Primary Display Driver (32 bit) (640x480)",
	 640, 480, 16, "Direct 3D - Primary Display Driver (16 bit) (640x480)"
};

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDX5Disp dialog


CDX5Disp::CDX5Disp(CWnd* pParent /*=NULL*/)
	: CDialog(CDX5Disp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDX5Disp)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

}


void CDX5Disp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDX5Disp)
	DDX_Control(pDX, IDC_DX5_RES, m_res_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDX5Disp, CDialog)
	//{{AFX_MSG_MAP(CDX5Disp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDX5Disp message handlers

/** 
 * Initialise the dialog ready for viewing
 */
BOOL CDX5Disp::OnInitDialog() 
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
void CDX5Disp::OnApply()
{
	// Now apply video mode
	int index = m_res_list.GetCurSel();

	if( index == CB_ERR)
	{
		MessageBox("Failed to set graphic mode", "Error", MB_ICONERROR);
		return;
	}

	int current = m_res_list.GetItemData(index);

	if(reg_set_sz(Settings::reg_path, "Videocard", dx5_modes[index].text_desc) == false)
	{
		MessageBox("Failed to set graphic mode", "Error", MB_ICONERROR);
	}
}

/**
 * Refreshes the list of modes avaliable, this depends on variables that have already 
 * been set using other functions of this class.
 */
void CDX5Disp::UpdateResList()
{
	int count = 0;
	char res_text[20];
 	m_res_list.ResetContent();

	for(int i = 0; i < NUM_DX5_MODES; i++)
	{
		sprintf(res_text, "%d x %d x %d", 
			dx5_modes[i].xres, 
			dx5_modes[i].yres,
			dx5_modes[i].cdepth);

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
void CDX5Disp::LoadSettings()
{	
	char videocard_string[MAX_PATH];

	// Lets get those video card settings
	if(reg_get_sz(Settings::reg_path, "VideocardFs2open", videocard_string, MAX_PATH) == false)
	{
		return;
	}

	int count = 0;
	for(int i = 0; i < NUM_DX5_MODES; i++)
	{
		if(strcmp(videocard_string, dx5_modes[i].text_desc) == 0)
		{
			m_res_list.SetCurSel(count);
			return;
		}

		count++;
	}

	m_res_list.SetCurSel(0);
}
