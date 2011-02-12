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
#include "BgBitmapDlg.h"
#include "backgroundchooser.h"
#include "starfield/starfield.h"
#include "bmpman/bmpman.h"
#include "FREDView.h"
#include "FREDDoc.h"
#include "palman/palman.h"
#include "starfield/nebula.h"
#include "nebula/neb.h"
#include "nebula/neblightning.h"
#include "parse/parselo.h"
#include "mission/missionparse.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// bg_bitmap_dlg dialog

bg_bitmap_dlg::bg_bitmap_dlg(CWnd* pParent) : CDialog(bg_bitmap_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(bg_bitmap_dlg)		
	m_neb_intensity = _T("");
	m_skybox_model = _T("");
	m_envmap = _T("");
	m_nebula_color = -1;
	m_nebula_index = -1;
	m_bank = 0;
	m_heading = 0;
	m_pitch = 0;
	m_neb2_texture = 0;
	m_subspace = FALSE;
	m_fullneb = FALSE;
	m_toggle_trails = FALSE;
	m_poof_0 = Neb2_poof_flags & (1<<0) ? 1 : 0;
	m_poof_1 = Neb2_poof_flags & (1<<1) ? 1 : 0;
	m_poof_2 = Neb2_poof_flags & (1<<2) ? 1 : 0;
	m_poof_3 = Neb2_poof_flags & (1<<3) ? 1 : 0;
	m_poof_4 = Neb2_poof_flags & (1<<4) ? 1 : 0;
	m_poof_5 = Neb2_poof_flags & (1<<5) ? 1 : 0;
	s_pitch = 0;
	s_bank = 0;
	s_heading = 0;
	s_scale = 1.0f;	
	s_index = -1;
	b_pitch = 0;
	b_bank = 0;
	b_heading = 0;
	b_scale_x = 1.0f; b_scale_y = 1.0f;
	b_div_x = 1; b_div_y = 1;
	b_index = -1;
	m_sky_flag_1 = The_mission.skybox_flags & MR_NO_LIGHTING ? 1 : 0;
	m_sky_flag_2 = The_mission.skybox_flags & MR_ALL_XPARENT ? 1 : 0;
	m_sky_flag_3 = The_mission.skybox_flags & MR_NO_ZBUFFER ? 1 : 0;
	m_sky_flag_4 = The_mission.skybox_flags & MR_NO_CULL ? 1 : 0;
	m_sky_flag_5 = The_mission.skybox_flags & MR_NO_GLOWMAPS ? 1 : 0;
	m_sky_flag_6 = The_mission.skybox_flags & MR_FORCE_CLAMP ? 1 : 0;
	//}}AFX_DATA_INIT
}

void bg_bitmap_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(bg_bitmap_dlg)			
	DDX_Control(pDX, IDC_AMBIENT_B_SLIDER, m_amb_blue);
	DDX_Control(pDX, IDC_AMBIENT_G_SLIDER, m_amb_green);
	DDX_Control(pDX, IDC_AMBIENT_R_SLIDER, m_amb_red);
	DDX_Text(pDX, IDC_NEB2_INTENSITY, m_neb_intensity);
	DDX_CBIndex(pDX, IDC_NEBCOLOR, m_nebula_color);
	DDX_CBIndex(pDX, IDC_NEBPATTERN, m_nebula_index);
	DDX_Text(pDX, IDC_BANK, m_bank);
	DDX_Text(pDX, IDC_HEADING, m_heading);
	DDX_Text(pDX, IDC_PITCH, m_pitch);
	DDX_Control(pDX, IDC_SLIDER1, m_slider);
	DDX_CBIndex(pDX, IDC_NEB2_TEXTURE, m_neb2_texture);
	DDX_Check(pDX, IDC_SUBSPACE, m_subspace);
	DDX_Check(pDX, IDC_FULLNEB, m_fullneb);
	DDX_Check(pDX, IDC_POOF0, m_poof_0);
	DDX_Check(pDX, IDC_POOF1, m_poof_1);
	DDX_Check(pDX, IDC_POOF2, m_poof_2);
	DDX_Check(pDX, IDC_POOF3, m_poof_3);
	DDX_Check(pDX, IDC_POOF4, m_poof_4);
	DDX_Check(pDX, IDC_POOF5, m_poof_5);
	DDX_Check(pDX, IDC_NEB_TOGGLE_TRAILS, m_toggle_trails);
	DDX_Text(pDX, IDC_SUN1, s_name);
	DDX_Text(pDX, IDC_SUN1_P, s_pitch);
	DDV_MinMaxInt(pDX, s_pitch, 0, 359);
	DDX_Text(pDX, IDC_SUN1_B, s_bank);
	DDV_MinMaxInt(pDX, s_bank, 0, 359);
	DDX_Text(pDX, IDC_SUN1_H, s_heading);
	DDV_MinMaxInt(pDX, s_heading, 0, 359);
	DDX_Text(pDX, IDC_SUN1_SCALE, s_scale);
	DDV_MinMaxFloat(pDX, s_scale, 0.1f, 50.0f);
	DDX_Text(pDX, IDC_SBITMAP, b_name);
	DDX_Text(pDX, IDC_SBITMAP_P, b_pitch);
	DDV_MinMaxInt(pDX, b_pitch, 0, 359);
	DDX_Text(pDX, IDC_SBITMAP_B, b_bank);
	DDV_MinMaxInt(pDX, b_bank, 0, 359);
	DDX_Text(pDX, IDC_SBITMAP_H, b_heading);
	DDV_MinMaxInt(pDX, b_heading, 0, 359);	DDX_Text(pDX, IDC_SBITMAP_SCALE_X, b_scale_x);
	DDV_MinMaxFloat(pDX, b_scale_x, .001f, 18.0f);
	DDX_Text(pDX, IDC_SBITMAP_SCALE_Y, b_scale_y);
	DDV_MinMaxFloat(pDX, b_scale_y, .001f, 18.0f);
	DDX_Text(pDX, IDC_SBITMAP_DIV_X, b_div_x);
	DDV_MinMaxInt(pDX, b_div_x, 1, 5);
	DDX_Text(pDX, IDC_SBITMAP_DIV_Y, b_div_y);
	DDV_MinMaxInt(pDX, b_div_y, 1, 5);
	DDX_Text(pDX, IDC_SKYBOX_FNAME, m_skybox_model);
	DDX_Text(pDX, IDC_ENVMAP, m_envmap);
	DDX_Check(pDX, IDC_SKY_FLAG_NO_LIGHTING, m_sky_flag_1);
	DDX_Check(pDX, IDC_SKY_FLAG_XPARENT, m_sky_flag_2);
	DDX_Check(pDX, IDC_SKY_FLAG_NO_ZBUFF, m_sky_flag_3);
	DDX_Check(pDX, IDC_SKY_FLAG_NO_CULL, m_sky_flag_4);
	DDX_Check(pDX, IDC_SKY_FLAG_NO_GLOW, m_sky_flag_5);
	DDX_Check(pDX, IDC_SKY_FLAG_CLAMP, m_sky_flag_6);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(bg_bitmap_dlg, CDialog)
	//{{AFX_MSG_MAP(bg_bitmap_dlg)
	ON_WM_CLOSE()	
	ON_CBN_SELCHANGE(IDC_NEBCOLOR, OnSelchangeNebcolor)
	ON_CBN_SELCHANGE(IDC_NEBPATTERN, OnSelchangeNebpattern)
	ON_BN_CLICKED(IDC_FULLNEB, OnFullNeb)
	ON_WM_HSCROLL()
	ON_LBN_SELCHANGE(IDC_SUN1_LIST, OnSunChange)
	ON_BN_CLICKED(IDC_ADD_SUN, OnAddSun)
	ON_BN_CLICKED(IDC_DEL_SUN, OnDelSun)
	ON_CBN_SELCHANGE(IDC_SUN1, OnSunDropdownChange)
	ON_LBN_SELCHANGE(IDC_SBITMAP_LIST, OnBitmapChange)
	ON_BN_CLICKED(IDC_ADD_SBITMAP, OnAddBitmap)
	ON_BN_CLICKED(IDC_DEL_SBITMAP, OnDelBitmap)
	ON_CBN_SELCHANGE(IDC_SBITMAP, OnBitmapDropdownChange)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SBITMAP_P_SPIN, OnDeltaposSbitmapPSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SBITMAP_B_SPIN, OnDeltaposSbitmapBSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SBITMAP_H_SPIN, OnDeltaposSbitmapHSpin)
	ON_EN_KILLFOCUS(IDC_SBITMAP_SCALE_X, OnKillfocusSbitmapScaleX)
	ON_EN_KILLFOCUS(IDC_SBITMAP_SCALE_Y, OnKillfocusSbitmapScaleY)
	ON_EN_KILLFOCUS(IDC_SBITMAP_DIV_X, OnKillfocusSbitmapDivX)
	ON_EN_KILLFOCUS(IDC_SBITMAP_DIV_Y, OnKillfocusSbitmapDivY)
	ON_EN_KILLFOCUS(IDC_SBITMAP_P, OnKillfocusSbitmapP)
	ON_EN_KILLFOCUS(IDC_SBITMAP_B, OnKillfocusSbitmapB)
	ON_EN_KILLFOCUS(IDC_SBITMAP_H, OnKillfocusSbitmapH)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SUN1_P_SPIN, OnDeltaposSun1PSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SUN1_H_SPIN, OnDeltaposSun1HSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SUN1_B_SPIN, OnDeltaposSun1BSpin)
	ON_EN_KILLFOCUS(IDC_SUN1_P, OnKillfocusSun1P)
	ON_EN_KILLFOCUS(IDC_SUN1_H, OnKillfocusSun1H)
	ON_EN_KILLFOCUS(IDC_SUN1_B, OnKillfocusSun1B)
	ON_EN_KILLFOCUS(IDC_SUN1_SCALE, OnKillfocusSun1Scale)
	ON_BN_CLICKED(IDC_IMPORT_BACKGROUND, OnImportBackground)
	ON_BN_CLICKED(IDC_SWAP_BACKGROUND, OnSwapBackground)
	ON_CBN_SELCHANGE(IDC_BACKGROUND_NUM, OnBackgroundDropdownChange)
	ON_BN_CLICKED(IDC_SKYBOX_MODEL, OnSkyboxBrowse)
	ON_BN_CLICKED(IDC_ENVMAP_BROWSE, OnEnvmapBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

const static float delta = .00001f;

/////////////////////////////////////////////////////////////////////////////
// bg_bitmap_dlg message handlers

void bg_bitmap_dlg::create()
{
	char buf[40];
	int i;
	CComboBox *box;

	CDialog::Create(bg_bitmap_dlg::IDD);
	theApp.init_window(&Bg_wnd_data, this);
	
	box = (CComboBox *) GetDlgItem(IDC_NEBCOLOR);
	for (i=0; i<NUM_NEBULA_COLORS; i++){
		box->AddString(Nebula_colors[i]);
	}	

	m_slider.SetRange(0, MAX_STARS);
	m_slider.SetPos(Num_stars);
	sprintf(buf, "%d", Num_stars);
	GetDlgItem(IDC_TOTAL)->SetWindowText(buf);

	build_nebfile_list();	

	// setup neb poof names
	GetDlgItem(IDC_POOF0)->SetWindowText(Neb2_poof_filenames[0]);	
	GetDlgItem(IDC_POOF1)->SetWindowText(Neb2_poof_filenames[1]);
	GetDlgItem(IDC_POOF2)->SetWindowText(Neb2_poof_filenames[2]);
	GetDlgItem(IDC_POOF3)->SetWindowText(Neb2_poof_filenames[3]);
	GetDlgItem(IDC_POOF4)->SetWindowText(Neb2_poof_filenames[4]);
	GetDlgItem(IDC_POOF5)->SetWindowText(Neb2_poof_filenames[5]);

	m_skybox_model = _T(The_mission.skybox_model);
	m_envmap = _T(The_mission.envmap_name);

	for(i=0; i<MAX_NEB2_BITMAPS; i++){
		if(strlen(Neb2_bitmap_filenames[i]) > 0){
			((CComboBox*)GetDlgItem(IDC_NEB2_TEXTURE))->AddString(Neb2_bitmap_filenames[i]);
		}
	}
	// if we have a texture selected already
	if(strlen(Neb2_texture_name) > 0){
		m_neb2_texture = ((CComboBox*)GetDlgItem(IDC_NEB2_TEXTURE))->SelectString(-1, Neb2_texture_name);
		if(m_neb2_texture == CB_ERR){
			((CComboBox*)GetDlgItem(IDC_NEB2_TEXTURE))->SetCurSel(0);
			m_neb2_texture = 0;
		}
	} else {
		((CComboBox*)GetDlgItem(IDC_NEB2_TEXTURE))->SetCurSel(0);
	}

	// setup lightning storm names
	((CComboBox*)GetDlgItem(IDC_NEB2_LIGHTNING))->ResetContent();
	((CComboBox*)GetDlgItem(IDC_NEB2_LIGHTNING))->AddString(CString("none"));
	for(size_t j=0; j<Storm_types.size(); j++){
		((CComboBox*)GetDlgItem(IDC_NEB2_LIGHTNING))->AddString(CString(Storm_types[j].name));
	}
	((CComboBox*)GetDlgItem(IDC_NEB2_LIGHTNING))->SelectString(-1, Mission_parse_storm_name);
		
	// if the nebula intensity wasn't set before - set it now
	if(Neb2_awacs < 0.0f){
		m_neb_intensity = CString("3000");
	} else {
		char whee[255] = "";
		m_neb_intensity = CString(itoa((int)Neb2_awacs, whee, 10));
	}
		
	// determine if a full Neb2 is active - load in the full nebula filenames or the partial neb
	// filenames
	m_fullneb = (The_mission.flags & MISSION_FLAG_FULLNEB) ? 1 : 0;
	if(m_fullneb){
		((CButton*)GetDlgItem(IDC_FULLNEB))->SetCheck(1);
	} else {
		// since there is no "none" option for the full nebulas
		m_nebula_index = Nebula_index + 1;		
	
		m_nebula_color = Mission_palette;
		if (Nebula_index < 0){
			GetDlgItem(IDC_NEBCOLOR)->EnableWindow(FALSE);
		}

		m_pitch = Nebula_pitch;
		m_bank = Nebula_bank;
		m_heading = Nebula_heading;
	}

	m_toggle_trails = (The_mission.flags & MISSION_FLAG_TOGGLE_SHIP_TRAILS) ? 1 : 0;
	((CButton*)GetDlgItem(IDC_NEB_TOGGLE_TRAILS))->SetCheck(m_toggle_trails);

	// setup background numbering
	for (i = 0; i < MAX_BACKGROUNDS; i++) 
	{
		char temp[NAME_LENGTH];
		sprintf(temp, "Background %d", i + 1);

		((CComboBox*) GetDlgItem(IDC_BACKGROUND_NUM))->AddString(temp);
		((CComboBox*) GetDlgItem(IDC_BACKGROUND_SWAP_NUM))->AddString(temp);
	}
	((CComboBox*) GetDlgItem(IDC_BACKGROUND_NUM))->SetCurSel(0);
	((CComboBox*) GetDlgItem(IDC_BACKGROUND_SWAP_NUM))->SetCurSel(0);

	// setup sun and sunglow controls
	sun_data_init();	

	// setup bitmap info
	bitmap_data_init();
	
	// determine if subspace is active
	m_subspace = (The_mission.flags & MISSION_FLAG_SUBSPACE) ? 1 : 0;

	m_amb_red.SetRange(1,255);
	m_amb_green.SetRange(1,255);
	m_amb_blue.SetRange(1,255);

	m_amb_red.SetPos(The_mission.ambient_light_level & 0xff);
	m_amb_green.SetPos((The_mission.ambient_light_level >> 8) & 0xff);
	m_amb_blue.SetPos((The_mission.ambient_light_level >> 16) & 0xff);

	sprintf(buf, "Red: %d", m_amb_red.GetPos());
	GetDlgItem(IDC_AMBIENT_R_TEXT)->SetWindowText(buf);
	sprintf(buf, "Green: %d", m_amb_green.GetPos());
	GetDlgItem(IDC_AMBIENT_G_TEXT)->SetWindowText(buf);
	sprintf(buf, "Blue: %d", m_amb_blue.GetPos());
	GetDlgItem(IDC_AMBIENT_B_TEXT)->SetWindowText(buf);
	

	UpdateData(FALSE);
	OnFullNeb();
	update_data();
	update_map_window();
	set_modified();
}

void bg_bitmap_dlg::OnOK()
{
	OnClose();
}

void bg_bitmap_dlg::OnCancel()
{
	OnClose();
}

void bg_bitmap_dlg::OnClose() 
{
	UpdateData(TRUE);
	Mission_palette = m_nebula_color;
	
	if(m_fullneb){		
		The_mission.flags |= MISSION_FLAG_FULLNEB;
		Neb2_awacs = (float)atoi((LPCSTR)m_neb_intensity);

		// override dumb values with reasonable ones
		if(Neb2_awacs <= 0.00000001f){
			Neb2_awacs = 3000.0f;
		}

		// store poof flags
		Neb2_poof_flags = 0;
		if(m_poof_0)
		{
			Neb2_poof_flags |= (1<<0);
		}
		if(m_poof_1)
		{
			Neb2_poof_flags |= (1<<1);
		}
		if(m_poof_2)
		{
			Neb2_poof_flags |= (1<<2);
		}
		if(m_poof_3)
		{
			Neb2_poof_flags |= (1<<3);
		}
		if(m_poof_4)
		{
			Neb2_poof_flags |= (1<<4);
		}
		if(m_poof_5)
		{
			Neb2_poof_flags |= (1<<5);
		}
		
		// get the bitmap name
		strcpy_s(Neb2_texture_name, Neb2_bitmap_filenames[m_neb2_texture]);

		// init the nebula
		neb2_level_init();
	} else {
		The_mission.flags &= ~MISSION_FLAG_FULLNEB;		
		Nebula_index = m_nebula_index - 1;
		Neb2_awacs = -1.0f;
		strcpy_s(Neb2_texture_name, "");
	}

	// check for no ship trails -C
	if( m_toggle_trails ) {
		The_mission.flags |= MISSION_FLAG_TOGGLE_SHIP_TRAILS;
	} else {
		The_mission.flags &= ~MISSION_FLAG_TOGGLE_SHIP_TRAILS;
	}

	// get selected storm
	((CComboBox*)GetDlgItem(IDC_NEB2_LIGHTNING))->GetLBText(((CComboBox*)GetDlgItem(IDC_NEB2_LIGHTNING))->GetCurSel(), Mission_parse_storm_name);

	Nebula_pitch = m_pitch;
	Nebula_bank = m_bank;
	Nebula_heading = m_heading;
	if (Nebula_index >= 0){
		nebula_init(Nebula_filenames[Nebula_index], m_pitch, m_bank, m_heading);
	} else {
		nebula_close();
	}

	if (m_subspace){
		The_mission.flags |= MISSION_FLAG_SUBSPACE;				
	} else {
		The_mission.flags &= ~MISSION_FLAG_SUBSPACE;		
	}

	string_copy(The_mission.skybox_model, m_skybox_model, NAME_LENGTH, 1);
	string_copy(The_mission.envmap_name, m_envmap, NAME_LENGTH, 1);

	//store the skybox flags
	The_mission.skybox_flags = 0;

	if(m_sky_flag_1) {
		The_mission.skybox_flags |= MR_NO_LIGHTING;
	}
	if(m_sky_flag_2) {
		The_mission.skybox_flags |= MR_ALL_XPARENT;
	}
	if(m_sky_flag_3) {
		The_mission.skybox_flags |= MR_NO_ZBUFFER;
	}
	if(m_sky_flag_4) {
		The_mission.skybox_flags |= MR_NO_CULL;
	}
	if(m_sky_flag_5) {
		The_mission.skybox_flags |= MR_NO_GLOWMAPS;
	}
	if(m_sky_flag_6) {
		The_mission.skybox_flags |= MR_FORCE_CLAMP;
	}

	// close sun data
	sun_data_close();

	// close bitmap data
	bitmap_data_close();

	// reset the background
	stars_pack_backgrounds();
	stars_load_first_valid_background();
	
	// close window stuff
	theApp.record_window_data(&Bg_wnd_data, this);
	delete Bg_bitmap_dialog;
	Bg_bitmap_dialog = NULL;
}

void bg_bitmap_dlg::update_data(int update)
{
	if (update){
		UpdateData(TRUE);
	}

	UpdateData(FALSE);	
}

void bg_bitmap_dlg::OnSelchangeNebcolor() 
{
	CWaitCursor wait;

	UpdateData(TRUE);
	Mission_palette = m_nebula_color;


	char palette_filename[1024];

	Assert( Mission_palette >= 0 );
	Assert( Mission_palette <= 98 );

	sprintf( palette_filename, "gamepalette%d-%02d", 1, Mission_palette+1 );

	mprintf(( "Loading palette %s\n", palette_filename ));

	palette_load_table(palette_filename);
	
	Update_window = 1;
}

void bg_bitmap_dlg::OnSelchangeNebpattern() 
{
	CWaitCursor wait;

	UpdateData(TRUE);

	// fullneb indexes differently	
	Nebula_index = m_nebula_index - 1;			

	GetDlgItem(IDC_NEBCOLOR)->EnableWindow(m_nebula_index ? TRUE : FALSE);
	if (Nebula_index >= 0){		
		nebula_init(Nebula_filenames[Nebula_index], m_pitch, m_bank, m_heading);		
	} else {
		nebula_close();
	}

	Update_window = 1;
}

void bg_bitmap_dlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar) 
{
	char buf[40];

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);

	MODIFY(Num_stars, m_slider.GetPos());
	sprintf(buf, "%d", Num_stars);
	GetDlgItem(IDC_TOTAL)->SetWindowText(buf);

	int col = 0;

	col |= m_amb_red.GetPos();
	col |= m_amb_green.GetPos() << 8;
	col |= m_amb_blue.GetPos() << 16;

	sprintf(buf, "Red: %d", m_amb_red.GetPos());
	GetDlgItem(IDC_AMBIENT_R_TEXT)->SetWindowText(buf);
	sprintf(buf, "Green: %d", m_amb_green.GetPos());
	GetDlgItem(IDC_AMBIENT_G_TEXT)->SetWindowText(buf);
	sprintf(buf, "Blue: %d", m_amb_blue.GetPos());
	GetDlgItem(IDC_AMBIENT_B_TEXT)->SetWindowText(buf);

	The_mission.ambient_light_level = col;
	gr_set_ambient_light(m_amb_red.GetPos(), m_amb_green.GetPos(), m_amb_blue.GetPos());
}

// when the user toggled the "Full Nebula" button
void bg_bitmap_dlg::OnFullNeb()
{		
	// determine what state we're in	
	UpdateData(TRUE);
	if(m_fullneb){
		// enable all fullneb controls
		GetDlgItem(IDC_NEB2_INTENSITY)->EnableWindow(TRUE);
		GetDlgItem(IDC_NEB2_TEXTURE)->EnableWindow(TRUE);
		GetDlgItem(IDC_NEB2_LIGHTNING)->EnableWindow(TRUE);
		GetDlgItem(IDC_POOF0)->EnableWindow(TRUE);
		GetDlgItem(IDC_POOF1)->EnableWindow(TRUE);
		GetDlgItem(IDC_POOF2)->EnableWindow(TRUE);
		GetDlgItem(IDC_POOF3)->EnableWindow(TRUE);
		GetDlgItem(IDC_POOF4)->EnableWindow(TRUE);
		GetDlgItem(IDC_POOF5)->EnableWindow(TRUE);
		GetDlgItem(IDC_NEB_TOGGLE_TRAILS)->EnableWindow(TRUE);

		// disable non-fullneb controls
		GetDlgItem(IDC_NEBPATTERN)->EnableWindow(FALSE);
		GetDlgItem(IDC_NEBCOLOR)->EnableWindow(FALSE);
		GetDlgItem(IDC_PITCH)->EnableWindow(FALSE);
		GetDlgItem(IDC_BANK)->EnableWindow(FALSE);
		GetDlgItem(IDC_HEADING)->EnableWindow(FALSE);

		// check all relevant poofs		
		((CButton*)GetDlgItem(IDC_POOF0))->SetCheck(FALSE);
		if(m_poof_0){
			((CButton*)GetDlgItem(IDC_POOF0))->SetCheck(TRUE);
		}
		((CButton*)GetDlgItem(IDC_POOF1))->SetCheck(FALSE);
		if(m_poof_1){
			((CButton*)GetDlgItem(IDC_POOF1))->SetCheck(TRUE);
		}
		((CButton*)GetDlgItem(IDC_POOF2))->SetCheck(FALSE);
		if(m_poof_2){
			((CButton*)GetDlgItem(IDC_POOF2))->SetCheck(TRUE);
		}
		((CButton*)GetDlgItem(IDC_POOF3))->SetCheck(FALSE);
		if(m_poof_3){
			((CButton*)GetDlgItem(IDC_POOF3))->SetCheck(TRUE);
		}
		((CButton*)GetDlgItem(IDC_POOF4))->SetCheck(FALSE);
		if(m_poof_4){
			((CButton*)GetDlgItem(IDC_POOF4))->SetCheck(TRUE);
		}
		((CButton*)GetDlgItem(IDC_POOF5))->SetCheck(FALSE);
		if(m_poof_5){
			((CButton*)GetDlgItem(IDC_POOF5))->SetCheck(TRUE);
		}
	} else {
		// enable all non-fullneb controls
		GetDlgItem(IDC_NEBPATTERN)->EnableWindow(TRUE);
		GetDlgItem(IDC_NEBCOLOR)->EnableWindow(TRUE);
		GetDlgItem(IDC_PITCH)->EnableWindow(TRUE);
		GetDlgItem(IDC_BANK)->EnableWindow(TRUE);
		GetDlgItem(IDC_HEADING)->EnableWindow(TRUE);		

		// disable all fullneb controls
		GetDlgItem(IDC_NEB2_INTENSITY)->EnableWindow(FALSE);
		GetDlgItem(IDC_NEB2_TEXTURE)->EnableWindow(FALSE);
		GetDlgItem(IDC_NEB2_LIGHTNING)->EnableWindow(FALSE);
		GetDlgItem(IDC_POOF0)->EnableWindow(FALSE);
		GetDlgItem(IDC_POOF1)->EnableWindow(FALSE);
		GetDlgItem(IDC_POOF2)->EnableWindow(FALSE);
		GetDlgItem(IDC_POOF3)->EnableWindow(FALSE);
		GetDlgItem(IDC_POOF4)->EnableWindow(FALSE);
		GetDlgItem(IDC_POOF5)->EnableWindow(FALSE);
		GetDlgItem(IDC_NEB_TOGGLE_TRAILS)->EnableWindow(FALSE);
	}		
}

// clear and build the nebula filename list appropriately
void bg_bitmap_dlg::build_nebfile_list()
{
	int i;
	CComboBox *box = (CComboBox *) GetDlgItem(IDC_NEBPATTERN);

	// wacky
	Assert(box != NULL);
	if(box == NULL){
		return;
	}
	
	// clear the box
	box->ResetContent();

	// add all necessary strings		
	box->AddString("None");
	for (i=0; i<NUM_NEBULAS; i++){
		box->AddString(Nebula_filenames[i]);
	}	

	// select the first elementccombobox
	box->SetCurSel(0);
	OnSelchangeNebpattern();
}

void bg_bitmap_dlg::sun_data_init()
{
	int idx;
	CComboBox *ccb = (CComboBox*) GetDlgItem(IDC_SUN1);
	CListBox *clb = (CListBox*) GetDlgItem(IDC_SUN1_LIST);
	background_t *background = &Backgrounds[get_active_background()];
	
	// clear if necessary
	ccb->ResetContent();
	clb->ResetContent();

	// add all suns to the drop down
	for (idx = 0; idx < stars_get_num_entries(true, true); idx++)
	{
		ccb->AddString(stars_get_name_FRED(idx, true));
 	}

	// add all suns by bitmap filename to the list
	for (idx = 0; idx < (int)background->suns.size(); idx++)
	{	
		clb->AddString(background->suns[idx].filename);
	}		

	// if we have at least one item, select it
	if (background->suns.size() > 0)
	{
		clb->SetCurSel(0);
		OnSunChange();
	}
}

void bg_bitmap_dlg::sun_data_close()
{	
	// if there is an active sun, save it
	sun_data_save_current();	
}

void bg_bitmap_dlg::sun_data_save_current()
{
	// if we have an active item
	if (s_index >= 0)
	{
		background_t *background = &Backgrounds[get_active_background()];
		starfield_list_entry *sle = &background->suns[s_index];

		// read out of the controls
		UpdateData(TRUE);

		// store the data
		strcpy_s(sle->filename, s_name);
		sle->ang.p = (float) fl_radians(s_pitch);
		sle->ang.b = (float) fl_radians(s_bank);
		sle->ang.h = (float) fl_radians(s_heading);
		sle->scale_x = (float) s_scale;
		sle->scale_y = 1.0f;
		sle->div_x = 1;
		sle->div_y = 1;
	}
}

void bg_bitmap_dlg::OnSunChange()
{	
	// save the current sun
	sun_data_save_current();

	// select the new one
	s_index = ((CListBox*) GetDlgItem(IDC_SUN1_LIST))->GetCurSel();

	// setup data	
	if (s_index >= 0)
	{
		int drop_index;
		background_t *background = &Backgrounds[get_active_background()];
		starfield_list_entry *sle = &background->suns[s_index];

		s_name = CString(sle->filename);
		s_pitch = (int) (fl_degrees(sle->ang.p) + delta);
		s_bank = (int) (fl_degrees(sle->ang.b) + delta);
		s_heading = (int) (fl_degrees(sle->ang.h) + delta);
		s_scale = sle->scale_x;

		// stuff back into the controls
		UpdateData(FALSE);

		// select the proper item from the dropdown
		drop_index = ((CComboBox*) GetDlgItem(IDC_SUN1))->FindString(-1, sle->filename);
		if(drop_index != CB_ERR)
			((CComboBox*) GetDlgItem(IDC_SUN1))->SetCurSel(drop_index);
	}

	// refresh the background
	stars_load_background(get_active_background());
}

void bg_bitmap_dlg::OnAddSun()
{
	starfield_list_entry sle;

	// save any current
	sun_data_save_current();

	// select the first sun by default
	strcpy_s(sle.filename, stars_get_name_FRED(0, true));

	sle.ang.p = 0;
	sle.ang.b = 0;
	sle.ang.h = 0;
	sle.scale_x = 1.0f;
	sle.scale_y = 1.0f;
	sle.div_x = 1;
	sle.div_y = 1;

	Backgrounds[get_active_background()].suns.push_back(sle);

	// add to the listbox and select it
	int add_index = ((CListBox*) GetDlgItem(IDC_SUN1_LIST))->AddString(sle.filename);
	((CListBox*) GetDlgItem(IDC_SUN1_LIST))->SetCurSel(add_index);

	// call the OnSunChange function to setup all relevant data in the class
	OnSunChange();
}

void bg_bitmap_dlg::OnDelSun()
{
	// if we don't have an active item
	if(s_index < 0)
		return;
	
	// remove the item from the list
	((CListBox*) GetDlgItem(IDC_SUN1_LIST))->DeleteString(s_index);

	// remove it from the list
	background_t *background = &Backgrounds[get_active_background()];
	background->suns.erase(background->suns.begin() + s_index);

	// no item selected, let the message handler assign a new one
	s_index = -1;

	// refresh the background
	stars_load_background(get_active_background());
}

void bg_bitmap_dlg::OnSunDropdownChange()
{	
	// if we have no active sun, do nothing
	if (s_index < 0)
		return;

	int new_index = ((CComboBox*) GetDlgItem(IDC_SUN1))->GetCurSel();
	Assert(new_index != CB_ERR);

	// get the new string
	if(new_index != CB_ERR)
	{
		((CComboBox*) GetDlgItem(IDC_SUN1))->GetLBText(new_index, s_name);

		// change the name of the string in the listbox
		((CListBox*) GetDlgItem(IDC_SUN1_LIST))->DeleteString(s_index);
		((CListBox*) GetDlgItem(IDC_SUN1_LIST))->InsertString(s_index, (const char*) s_name);

		OnSunChange();
	}	
}

void bg_bitmap_dlg::bitmap_data_init()
{
	int idx;
	CComboBox *ccb = (CComboBox*) GetDlgItem(IDC_SBITMAP);
	CListBox *clb = (CListBox*) GetDlgItem(IDC_SBITMAP_LIST);
	background_t *background = &Backgrounds[get_active_background()];
	
	// clear if necessary
	ccb->ResetContent();
	clb->ResetContent();

	// add all bitmaps to the drop down
	for (idx = 0; idx < stars_get_num_entries(false, true); idx++)
	{
		ccb->AddString(stars_get_name_FRED(idx, false));
 	}

	// add all bitmaps by bitmap filename to the list
	for (idx = 0; idx < (int)background->bitmaps.size(); idx++)
	{	
		clb->AddString(background->bitmaps[idx].filename);
	}		

	// if we have at least one item, select it
	if (background->bitmaps.size() > 0)
	{
		clb->SetCurSel(0);
		OnBitmapChange();
	}
}

void bg_bitmap_dlg::bitmap_data_close()
{
	// if there is an active bitmap, save it
	bitmap_data_save_current();	
}

void bg_bitmap_dlg::bitmap_data_save_current()
{
	// if we have an active item
	if (b_index >= 0)
	{
		background_t *background = &Backgrounds[get_active_background()];
		starfield_list_entry *sle = &background->bitmaps[b_index];

		// read out of the controls
		UpdateData(TRUE);

		// store the data
		strcpy_s(sle->filename, b_name);
		sle->ang.p = (float) fl_radians(b_pitch);
		sle->ang.b = (float) fl_radians(b_bank);
		sle->ang.h = (float) fl_radians(b_heading);
		sle->scale_x = (float) b_scale_x;
		sle->scale_y = (float) b_scale_y;
		sle->div_x = b_div_x;
		sle->div_y = b_div_y;
	}
}

void bg_bitmap_dlg::OnBitmapChange()
{
	// save the current bitmap
	bitmap_data_save_current();

	// select the new one
	b_index = ((CListBox*) GetDlgItem(IDC_SBITMAP_LIST))->GetCurSel();

	// setup data	
	if (b_index >= 0)
	{
		int drop_index;
		background_t *background = &Backgrounds[get_active_background()];
		starfield_list_entry *sle = &background->bitmaps[b_index];

		b_name = CString(sle->filename);
		b_pitch = (int) (fl_degrees(sle->ang.p) + delta);
		b_bank = (int) (fl_degrees(sle->ang.b) + delta);
		b_heading = (int) (fl_degrees(sle->ang.h) + delta);
		b_scale_x = sle->scale_x;
		b_scale_y = sle->scale_y;
		b_div_x = sle->div_x;
		b_div_y = sle->div_y;

		// stuff back into the controls
		UpdateData(FALSE);

		// select the proper item from the dropdown
		drop_index = ((CComboBox*) GetDlgItem(IDC_SBITMAP))->FindString(-1, sle->filename);
		if(drop_index != CB_ERR)
			((CComboBox*) GetDlgItem(IDC_SBITMAP))->SetCurSel(drop_index);
	}

	// refresh the background
	stars_load_background(get_active_background());
}

void bg_bitmap_dlg::OnAddBitmap()
{
	starfield_list_entry sle;

	// save any current
	bitmap_data_save_current();

	// select the first bitmap by default
	strcpy_s(sle.filename, stars_get_name_FRED(0, false));

	sle.ang.p = 0;
	sle.ang.b = 0;
	sle.ang.h = 0;
	sle.scale_x = 1.0f;
	sle.scale_y = 1.0f;
	sle.div_x = 1;
	sle.div_y = 1;

	Backgrounds[get_active_background()].bitmaps.push_back(sle);

	// add to the listbox and select it
	int add_index = ((CListBox*) GetDlgItem(IDC_SBITMAP_LIST))->AddString(sle.filename);
	((CListBox*) GetDlgItem(IDC_SBITMAP_LIST))->SetCurSel(add_index);

	// call the OnBitmapChange function to setup all relevant data in the class
	OnBitmapChange();
}

void bg_bitmap_dlg::OnDelBitmap()
{
	// if we don't have an active item
	if(b_index < 0)
		return;
	
	// remove the item from the list
	((CListBox*) GetDlgItem(IDC_SBITMAP_LIST))->DeleteString(b_index);

	// remove it from the list
	background_t *background = &Backgrounds[get_active_background()];
	background->bitmaps.erase(background->bitmaps.begin() + b_index);

	// no item selected, let the message handler assign a new one
	b_index = -1;

	// refresh the background
	stars_load_background(get_active_background());
}

void bg_bitmap_dlg::OnBitmapDropdownChange()
{
	// if we have no active bitmap, do nothing
	if (b_index < 0)
		return;

	int new_index = ((CComboBox*) GetDlgItem(IDC_SBITMAP))->GetCurSel();
	Assert(new_index != CB_ERR);

	// get the new string
	if(new_index != CB_ERR)
	{
		((CComboBox*) GetDlgItem(IDC_SBITMAP))->GetLBText(new_index, b_name);

		// change the name of the string in the listbox
		((CListBox*) GetDlgItem(IDC_SBITMAP_LIST))->DeleteString(b_index);
		((CListBox*) GetDlgItem(IDC_SBITMAP_LIST))->InsertString(b_index, (const char*) b_name);

		OnBitmapChange();
	}	
}

void bg_bitmap_dlg::get_data_spinner(NM_UPDOWN* pUD, int id, int *var, int min, int max)
{
	if (pUD->iDelta > 0)
	{
		(*var)--;

		//go min->max
		if (*var == (min-1))
		{
			*var = max;
		}

		this->SetDlgItemInt(id, *var);
	}
	else 
	{
		(*var)++;

		//go max->min
		if (*var == (max+1))
		{
			*var=min;
		}

		this->SetDlgItemInt(id, *var);
	}
}


void bg_bitmap_dlg::get_data_float(int id, float *var, float min, float max)
{
	char buf[16];
	char max_ch[16];
	char min_ch[16];

	this->GetDlgItemText(id, buf, 16);

	*var = (float)atof(buf);
	sprintf(max_ch,"%.3f",max);
	sprintf(min_ch,"%.3f",min);
	CString error_msg = "Please Enter a number between ";
	error_msg += min_ch;
	error_msg += " and ";
	error_msg += max_ch;

	if (*var < min)
	{
		*var = min;
		this->SetDlgItemText(id, min_ch);
		MessageBox(error_msg, "Error", MB_OK | MB_ICONWARNING);
	}
		
	if (*var > max)
	{
		*var = max;
		this->SetDlgItemText(id, max_ch);
		MessageBox(error_msg, "Error", MB_OK | MB_ICONWARNING);
	}
}


void bg_bitmap_dlg::get_data_int(int id, int *var, int min, int max)
{
	char max_ch[16];
	char min_ch[16];

	*var=this->GetDlgItemInt(id);

	sprintf(max_ch,"%d",max);
	sprintf(min_ch,"%d",min);
	CString error_msg = "Please Enter a number between ";
	error_msg += min_ch;
	error_msg += " and ";
	error_msg += max_ch;

	if (*var < min)
	{
		*var = min;
		SetDlgItemInt(id, min);
		MessageBox(error_msg, "Error", MB_OK | MB_ICONWARNING);
	}
		
	if (*var > max)
	{
		*var = max;
		SetDlgItemInt(id, max);
		MessageBox(error_msg, "Error", MB_OK | MB_ICONWARNING);
	}
}

void bg_bitmap_dlg::OnDeltaposSbitmapPSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	if (b_index < 0) return;
	get_data_spinner(pNMUpDown, IDC_SBITMAP_P, &b_pitch, 0, 359);
	OnBitmapChange();
	*pResult = 0;
}

void bg_bitmap_dlg::OnDeltaposSbitmapBSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	if (b_index < 0) return;
	get_data_spinner(pNMUpDown, IDC_SBITMAP_B, &b_bank, 0, 359);
	OnBitmapChange();
	*pResult = 0;
}

void bg_bitmap_dlg::OnDeltaposSbitmapHSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	if (b_index < 0) return;
	get_data_spinner(pNMUpDown, IDC_SBITMAP_H, &b_heading, 0, 359);
	OnBitmapChange();
	*pResult = 0;
}

void bg_bitmap_dlg::OnKillfocusSbitmapScaleX() 
{
	if (b_index < 0) return;
	get_data_float(IDC_SBITMAP_SCALE_X, &b_scale_x, 0.001f, 18.0f);
	OnBitmapChange();
}

void bg_bitmap_dlg::OnKillfocusSbitmapScaleY() 
{
	if (b_index < 0) return;
	get_data_float(IDC_SBITMAP_SCALE_Y, &b_scale_y, 0.001f, 18.0f);
	OnBitmapChange();
}

void bg_bitmap_dlg::OnKillfocusSbitmapDivX() 
{
	if (b_index < 0) return;
	get_data_int(IDC_SBITMAP_DIV_X, &b_div_x, 1,5);
	OnBitmapChange();
}

void bg_bitmap_dlg::OnKillfocusSbitmapDivY() 
{
	if (b_index < 0) return;
	get_data_int(IDC_SBITMAP_DIV_Y, &b_div_y, 1,5);	
	OnBitmapChange();
}

void bg_bitmap_dlg::OnKillfocusSbitmapP() 
{
	if (b_index < 0) return;
	get_data_int(IDC_SBITMAP_P, &b_pitch, 0, 359);
	OnBitmapChange();
}

void bg_bitmap_dlg::OnKillfocusSbitmapB() 
{
	if (b_index < 0) return;
	get_data_int(IDC_SBITMAP_B, &b_bank, 0, 359);
	OnBitmapChange();
}

void bg_bitmap_dlg::OnKillfocusSbitmapH() 
{
	if (b_index < 0) return;
	get_data_int(IDC_SBITMAP_H, &b_heading, 0, 359);
	OnBitmapChange();
}

void bg_bitmap_dlg::OnDeltaposSun1PSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	if (s_index < 0) return;
	//G5K - why?	pNMUpDown->iDelta *= -1;
	get_data_spinner(pNMUpDown, IDC_SUN1_P, &s_pitch, 0, 359);
	OnSunChange();
	*pResult = 0;
}

void bg_bitmap_dlg::OnDeltaposSun1BSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	if (s_index < 0) return;
	//G5K - why?	pNMUpDown->iDelta *= -1;
	get_data_spinner(pNMUpDown, IDC_SUN1_B, &s_bank, 0, 359);
	OnSunChange();
	*pResult = 0;
}

void bg_bitmap_dlg::OnDeltaposSun1HSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	if (s_index < 0) return;
	//G5K - why?	pNMUpDown->iDelta *= -1;
	get_data_spinner(pNMUpDown, IDC_SUN1_H, &s_heading, 0, 359);
	OnSunChange();
	*pResult = 0;
}

void bg_bitmap_dlg::OnKillfocusSun1P() 
{
	if (s_index < 0) return;
	get_data_int(IDC_SUN1_P, &s_pitch, 0, 359);
	OnSunChange();
}

void bg_bitmap_dlg::OnKillfocusSun1H() 
{
	if (s_index < 0) return;
	get_data_int(IDC_SUN1_H, &s_heading, 0, 359);
	OnSunChange();
}

void bg_bitmap_dlg::OnKillfocusSun1B() 
{
	if (s_index < 0) return;
	get_data_int(IDC_SUN1_B, &s_bank, 0, 359);
	OnSunChange();
}

void bg_bitmap_dlg::OnKillfocusSun1Scale() 
{
	if (s_index < 0) return;
	get_data_float(IDC_SUN1_SCALE, &s_scale, 0.1f, 50.0f);
	OnSunChange();
}


extern void parse_one_background(background_t *background);

void bg_bitmap_dlg::OnImportBackground() 
{
	CFileDialog cfd(TRUE, ".fs2", NULL, 0, "FreeSpace2 Missions (*.fs2)|*.fs2||\0");
	char filename[256], error_str[1024];
	int rval, temp, count;
	char *saved_mp;

	//warn on pressing the button
	if (MessageBox("This action will erase any nebulae and suns already placed.  Continue?", "Fred2", MB_ICONWARNING | MB_YESNO) == IDNO)
		return;

	//check if cancel was pressed
	if (cfd.DoModal() == IDCANCEL)
		return;

	strcpy_s(filename, cfd.GetPathName());

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("BGBITMAPDLG: Unable to parse '%s'!  Error code = %i.\n", filename, rval));
		sprintf(error_str, "Could not parse file: %s", filename);

		MessageBox((LPCTSTR) error_str, (LPCTSTR) "Unable to import mission background!", MB_ICONERROR | MB_OK);
		return;
	}

	// parse in the new file
	read_file_text(filename);
	reset_parse();

	if (!skip_to_start_of_string("#Background bitmaps"))
		return;

	// skip beginning stuff
	required_string("#Background bitmaps");
	required_string("$Num stars:");
	stuff_int(&temp);
	required_string("$Ambient light level:");
	stuff_int(&temp);

	saved_mp = Mp;

	// see if we have more than one background in this mission
	count = 0;
	while(skip_to_string("$Bitmap List:"))
		count++;

	Mp = saved_mp;

	// pick one (if count is 0, it's retail with just one background)
	if (count > 0)
	{
		int i, which = 0;

		if (count > 1)
		{
			BackgroundChooser dlg(count);
			if (dlg.DoModal() == IDCANCEL)
				return;

			which = dlg.GetChosenBackground();
		}

		for (i = 0; i < which + 1; i++)
			skip_to_string("$Bitmap List:");
	}

	// now parse the background we've selected
	parse_one_background(&Backgrounds[get_active_background()]);

	reinitialize_lists();
}

void bg_bitmap_dlg::reinitialize_lists()
{
	b_index = -1;
	s_index = -1;

	// repopulate
	sun_data_init();
	bitmap_data_init();

	// refresh the background
	stars_load_background(get_active_background());
}

int bg_bitmap_dlg::get_active_background()
{
	// find out which background we're editing
	int idx = ((CComboBox *) GetDlgItem(IDC_BACKGROUND_NUM))->GetCurSel();
	if (idx < 0 || idx >= MAX_BACKGROUNDS)
		idx = 0;

	return idx;
}

int bg_bitmap_dlg::get_swap_background()
{
	// find out which background we're swapping
	int idx = ((CComboBox *) GetDlgItem(IDC_BACKGROUND_SWAP_NUM))->GetCurSel();
	if (idx < 0 || idx >= MAX_BACKGROUNDS)
		idx = 0;

	return idx;
}

void bg_bitmap_dlg::OnBackgroundDropdownChange()
{
	reinitialize_lists();
}

void bg_bitmap_dlg::OnSwapBackground() 
{
	int idx1 = get_active_background();
	int idx2 = get_swap_background();

	// don't swap if they're the same
	if (idx1 == idx2)
	{
		MessageBox("Cannot swap a background with itself.", "FRED2", MB_OK);
		return;
	}

	// swap
	stars_swap_backgrounds(idx1, idx2);

	// refresh dialog
	reinitialize_lists();
}


char *Model_file_ext =	"Model Files (*.pof)|*.pof|"
						"|";

char *Image_file_ext =	"DDS Files (*.dds)|*.dds|"
						"|";

void bg_bitmap_dlg::OnSkyboxBrowse()
{
	CString filename;
	int z;

	UpdateData(TRUE);

	// get list of
	z = cfile_push_chdir(CF_TYPE_DATA);
	CFileDialog dlg(TRUE, NULL, filename, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, Model_file_ext);

	// if we have a result
	if (dlg.DoModal() == IDOK) {
		m_skybox_model = dlg.GetFileName();		
//	} else {
//		m_skybox_model = _T("");
	}

	UpdateData(FALSE);		

	// restore directory
	if ( !z )
		cfile_pop_dir();

	// load/display new skybox model (if one was selected)
	stars_set_background_model( (char*)(LPCTSTR)m_skybox_model, NULL );
}

void bg_bitmap_dlg::OnEnvmapBrowse()
{
	CString filename;
	int z;

	UpdateData(TRUE);

	ENVMAP = -1;

	// get list of
	z = cfile_push_chdir(CF_TYPE_DATA);
	CFileDialog dlg(TRUE, NULL, filename, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, Image_file_ext);

	// if we have a result
	if (dlg.DoModal() == IDOK) {
		m_envmap = dlg.GetFileName();		
//	} else {
//		m_envmap = _T("");
	}

	UpdateData(FALSE);		

	// restore directory
	if ( !z )
		cfile_pop_dir();

	// set the new envmap
	ENVMAP = bm_load( (char*)(LPCTSTR)m_envmap );
}
