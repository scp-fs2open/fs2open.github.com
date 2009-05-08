// OGLDisp.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "OGLDisp.h"
#include <vector>

#include "win32func.h"
#include "launcher_settings.h"

typedef struct
{
	int cdepth;
	char text[10];
} CDepth;

CDepth color_depth[2];

typedef struct
{
	int xres;
	int yres;
	int cdepth;
} ModeOGL;

using std::vector;
vector<ModeOGL> ogl_modes;

static int aniso_value[6] = { 0, 1, 2, 4, 8, 16 };
static int fsaa_value[5] = { 0, 2, 4, 8, 16 };

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COGLDisp dialog


COGLDisp::COGLDisp(CWnd* pParent /*=NULL*/)
	: CDialog(COGLDisp::IDD, pParent)
{
	//{{AFX_DATA_INIT(COGLDisp)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COGLDisp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COGLDisp)
	DDX_Control(pDX, IDC_ANISOFILTER_SLIDER, m_anisofilter_slider);
	DDX_Control(pDX, IDC_FSAA_SLIDER, m_fsaa_slider);
	DDX_Control(pDX, IDC_OGL_RES, m_res_list);
	DDX_Control(pDX, IDC_OGL_TEXFILTER_LIST, m_texfilter_list);
	DDX_Control(pDX, IDC_CDEPTH_LIST, m_cdepth_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COGLDisp, CDialog)
	//{{AFX_MSG_MAP(COGLDisp)
	ON_BN_CLICKED(IDC_GEN_CAPS, OnGenCaps)
	ON_CBN_SELCHANGE(IDC_CDEPTH_LIST, OnChangeCDepth)
	ON_CBN_SELCHANGE(IDC_OGL_RES, OnResChange)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COGLDisp message handlers

BOOL COGLDisp::OnInitDialog() 
{
	ModeOGL mogl;
	DEVMODE dm;
	DWORD n = 0;
	unsigned int i;
	bool found = false;

	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;

	while ( EnumDisplaySettings(NULL, n, &dm) ) {
		mogl.cdepth = dm.dmBitsPerPel;
		mogl.xres = dm.dmPelsWidth;
		mogl.yres = dm.dmPelsHeight;
		n++;

		if (mogl.xres < 640)
			continue;
		if (mogl.yres < 480)
			continue;
		if (mogl.cdepth < 16)
			continue;

		found = false;

		for (i = 0; i < ogl_modes.size(); i++) {
			if ( (mogl.cdepth == ogl_modes[i].cdepth) && (mogl.yres == ogl_modes[i].yres)
				&& (mogl.xres == ogl_modes[i].xres) )
			{
				found = true;
			}
		}

		if (found)
			continue;

		ogl_modes.push_back(mogl);
	}

	
	color_depth[0].cdepth = 16;
	strcpy(color_depth[0].text, "16-bit");
	color_depth[1].cdepth = 32;
	strcpy(color_depth[1].text, "32-bit");

	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/**
 * The user has chosen to accept these settings
 *
 * @param char *reg_path - Registry path that any settings should be saved to
 */
void COGLDisp::OnApply()
{
	int index = m_res_list.GetCurSel();

	if (index == CB_ERR) {
		MessageBox("Failed to set graphic mode");
		return;
	}

	int current = m_res_list.GetItemData(index);

	char videocard[100];
	sprintf(videocard, "OGL -(%dx%d)x%d bit",
		ogl_modes[current].xres,
		ogl_modes[current].yres,
		ogl_modes[current].cdepth);

	if ( reg_set_sz(LauncherSettings::get_reg_path(), "VideocardFs2open", videocard) == false )
		MessageBox("Failed to set graphic mode");

	current = m_anisofilter_slider.GetPos();

	char aniso_setting[10] = "";
	sprintf(aniso_setting, "%i.0", aniso_value[current]);

	if ( reg_set_sz(LauncherSettings::get_reg_path(), "OGL_AnisotropicFilter", aniso_setting) == false )
		MessageBox("Failed to set Anisotropic Filter setting.");

	current = m_fsaa_slider.GetPos();
	if ( reg_set_dword(LauncherSettings::get_reg_path(), "OGL_FSAA", fsaa_value[current]) == false )
		MessageBox("Failed to set FSAA setting.");

	current = m_texfilter_list.GetCurSel();

	if (current != CB_ERR) {
		if ( reg_set_dword(LauncherSettings::get_reg_path(), "TextureFilter", current) == false )
			MessageBox("Failed to set Texture Filter setting.");
	}
}

int COGLDisp::GetCDepth(int cdepth)
{
	int idx = m_cdepth_list.GetCurSel();

	if (cdepth == -1) {
		if (idx < 0) {
			m_cdepth_list.SetCurSel(0);
			idx = 0;
		}

		return color_depth[idx].cdepth;
	}

	for (idx = 0; idx < 2; idx++) {
		if (cdepth == color_depth[idx].cdepth) {
			m_cdepth_list.SetCurSel(idx);
			return color_depth[idx].cdepth;
		}
	}

	return cdepth;
}

void COGLDisp::UpdateResList(unsigned int requested_width, unsigned int requested_height, int requested_cdepth)
{
	int selected_sel = 0;
	int count = 0;
	unsigned int i;
	bool standard_match = TRUE;

	requested_cdepth = GetCDepth(requested_cdepth);

 	m_res_list.ResetContent();

	for(i = 0; i < ogl_modes.size(); i++)
	{
		if (ogl_modes[i].cdepth != requested_cdepth)
			continue;

		// skip anything below the minimum
		if(	!(ogl_modes[i].xres == 640 && ogl_modes[i].yres == 480) &&
			(ogl_modes[i].xres < 800 || ogl_modes[i].yres < 600))
		{
		  	continue;
		}

		char mode_string[128];
		sprintf(mode_string, "%d x %d", ogl_modes[i].xres, ogl_modes[i].yres);

		// Store the index
		int index = m_res_list.InsertString(count, mode_string);
		m_res_list.SetItemData(index, i);

		if( requested_width  == ogl_modes[i].xres &&
			requested_height == ogl_modes[i].yres &&
			requested_cdepth == ogl_modes[i].cdepth)

		{
	   		selected_sel = count;

			// identify whether this is one of the standard video modes or not
			standard_match = ((ogl_modes[i].xres == 1024 && ogl_modes[i].yres == 768) ||
									(ogl_modes[i].xres == 640 && ogl_modes[i].yres == 480));
		} 

		count++;
	}

	if (selected_sel < 0)
		selected_sel = 0;

	// show or hide the non-standard video mode text if needed
	GetDlgItem(IDC_NSVM_TEXT)->ShowWindow( (standard_match) ? SW_HIDE : SW_SHOW );

	m_res_list.SetCurSel(selected_sel);
}

void COGLDisp::SetFSAA(DWORD new_value)
{
	char value[5] = "";

	m_fsaa_slider.SetRange(0, 4);
	m_fsaa_slider.SetTicFreq(1);

	int cur_setting = 0;

	while (new_value != fsaa_value[cur_setting]) {
		if (new_value < fsaa_value[cur_setting])
			break;

		cur_setting++;

		if (cur_setting > 4) {
			cur_setting = 0;
			break;
		}
	}

	if (cur_setting < 0)
		cur_setting = 0;
	else if (cur_setting > 4)
		cur_setting = 4;

	m_fsaa_slider.SetPos(cur_setting);

	if (fsaa_value[cur_setting] == 0)
		sprintf(value, "Off");
	else
		sprintf(value, "%ix", fsaa_value[cur_setting]);

	GetDlgItem(IDC_FSAA_SETTING)->SetWindowText( value );
}

void COGLDisp::SetAnisoFilter(char *filter_set)
{
	char value[5] = "";

	m_anisofilter_slider.SetRange(0, 5);
	m_anisofilter_slider.SetTicFreq(1);

	int cur_setting = 0;

	if (filter_set) {
		int setting = atoi(filter_set);

		while (setting != aniso_value[cur_setting]) {
			if (setting < aniso_value[cur_setting])
				break;

			cur_setting++;

			if (cur_setting > 5) {
				cur_setting = 0;
				break;
			}
		}
	}

	if (cur_setting < 0)
		cur_setting = 0;
	else if (cur_setting > 5)
		cur_setting = 5;

	m_anisofilter_slider.SetPos(cur_setting);

	if (aniso_value[cur_setting] == 0)
		sprintf(value, "Off");
	else
		sprintf(value, "%ix", aniso_value[cur_setting]);

	GetDlgItem(IDC_ANISO_SETTING)->SetWindowText( value );
}

void COGLDisp::LoadSettings()
{
	unsigned int width, height;
	int cdepth;
	char videocard_string[MAX_PATH];
	char anisofilter_string[10] = "";

	// Lets get those video card settings
	if(reg_get_sz(LauncherSettings::get_reg_path(), "VideocardFs2open", videocard_string, MAX_PATH) == false)
		return;

	if(sscanf(videocard_string, "OGL -(%dx%d)x%d bit", &width, &height, &cdepth)  != 3) 
		return;

	UpdateResList(width, height, cdepth);

	if (reg_get_sz(LauncherSettings::get_reg_path(), "OGL_AnisotropicFilter", anisofilter_string, 10) == false)
		SetAnisoFilter("0.0");
	else
		SetAnisoFilter(anisofilter_string);

	DWORD fsaa = 0;
	if (reg_get_dword(LauncherSettings::get_reg_path(), "OGL_FSAA", &fsaa) == false)
		SetFSAA(0);
	else
		SetFSAA(fsaa);
	
	DWORD texfilter = 1;
	if (reg_get_dword(LauncherSettings::get_reg_path(), "TextureFilter", &texfilter) == false)
		m_texfilter_list.SetCurSel(1);
	else
		m_texfilter_list.SetCurSel(texfilter);
}

void COGLDisp::UpdateLists()
{
	// set color depth defaults
	m_cdepth_list.ResetContent();
	m_cdepth_list.AddString(color_depth[0].text);
	m_cdepth_list.AddString(color_depth[1].text);

	// set texture filter default
	m_texfilter_list.ResetContent();
	m_texfilter_list.AddString( TEXT("Bilinear") );
	m_texfilter_list.AddString( TEXT("Trilinear") );

	// set aniso filter based on reg setting if possible
	char anisofilter_string[10] = "";
	if (reg_get_sz(LauncherSettings::get_reg_path(), "OGL_AnisotropicFilter", anisofilter_string, 10) == false)
		SetAnisoFilter("0.0");
	else
		SetAnisoFilter(anisofilter_string);

	DWORD fsaa = 0;
	if (reg_get_dword(LauncherSettings::get_reg_path(), "OGL_FSAA", &fsaa) == false)
		SetFSAA(0);
	else
		SetFSAA(fsaa);

	// set tex filter based on reg setting if possible
	DWORD texfilter = 1;
	if (reg_get_dword(LauncherSettings::get_reg_path(), "TextureFilter", &texfilter) == false)
		m_texfilter_list.SetCurSel(1);
	else
		m_texfilter_list.SetCurSel(texfilter);


	// now the resolution list
	UpdateResList();
}


void COGLDisp::OnChangeCDepth()
{
	int cur_pos = m_res_list.GetCurSel();

	if (cur_pos == CB_ERR)
		return;

	int offset = m_res_list.GetItemData(cur_pos);

	if (offset < 0)
		offset = 0;

	UpdateResList(ogl_modes[offset].xres, ogl_modes[offset].yres);
}

void COGLDisp::OnGenCaps() 
{
	char filename[MAX_PATH] = {"OGL CAPS "};

	unsigned long len = MAX_PATH - strlen(filename); 
	GetUserName(filename + strlen(filename), &len); 

	FILE *fp = browse_for_and_open_save_file(this->GetSafeHwnd(), filename, "txt", "Save OGL CAPS textfile"); 

	if (fp == NULL) {
		MessageBox("Cancelled OGL cap file save.", "Warning", MB_ICONWARNING);
		return;
	}

	char os_string[100];
  	os_get_type_text(os_string);

 	fprintf(fp, "Operating System: %s\n", os_string); 
 	fprintf(fp, "Memory: %d MB\n", memory_get_ammount() / 1048576);

	// TODO:  more caps

	fclose(fp);
}

void COGLDisp::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	char value[5] = "";
	int pos;

	if (*pScrollBar == m_anisofilter_slider) {
		pos = m_anisofilter_slider.GetPos();

		if (aniso_value[pos] == 0)
			sprintf(value, "Off");
		else
			sprintf(value, "%ix", aniso_value[pos]);

		GetDlgItem(IDC_ANISO_SETTING)->SetWindowText( value );
	} else if (*pScrollBar == m_fsaa_slider) {
		pos = m_fsaa_slider.GetPos();

		if (fsaa_value[pos] == 0)
			sprintf(value, "Off");
		else
			sprintf(value, "%ix", fsaa_value[pos]);

		GetDlgItem(IDC_FSAA_SETTING)->SetWindowText( value );
	}
}

void COGLDisp::OnResChange()
{
	int index = m_res_list.GetCurSel();

	// if we don't have a current selection then it should default to position 0
	// which would be 640x480
	if (index == CB_ERR) {
		GetDlgItem(IDC_NSVM_TEXT)->ShowWindow( SW_HIDE );
		return;
	}

	int i = m_res_list.GetItemData(index);

	// identify whether this is one of the standard video modes or not
	bool standard_match = ((ogl_modes[i].xres == 1024 && ogl_modes[i].yres == 768) ||
						(ogl_modes[i].xres == 640 && ogl_modes[i].yres == 480));

	// show or hide the non-standard video mode text if needed
	GetDlgItem(IDC_NSVM_TEXT)->ShowWindow( (standard_match) ? SW_HIDE : SW_SHOW );
}
