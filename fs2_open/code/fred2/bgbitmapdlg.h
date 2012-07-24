/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _BG_BITMAP_DLG_H
#define _BG_BITMAP_DLG_H

/////////////////////////////////////////////////////////////////////////////
// bg_bitmap_dlg dialog

class bg_bitmap_dlg : public CDialog
{
// Construction
public:
	void update_data(int update = 1);
	void create();

	// sun data functions
	void sun_data_init();
	void sun_data_close();
	void sun_data_save_current();

	// bitmap data functions
	void bitmap_data_init();
	void bitmap_data_close();
	void bitmap_data_save_current();

	void get_data_spinner(NM_UPDOWN* pUD, int id, int *var, int min, int max);
	void get_data_int(int id, int *var, int min, int max);
	void get_data_float(int id, float *var, float max, float min);

	bg_bitmap_dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(bg_bitmap_dlg)
	enum { IDD = IDD_BG_BITMAP };
	CSliderCtrl	m_amb_blue;
	CSliderCtrl	m_amb_green;
	CSliderCtrl	m_amb_red;
	CString	m_neb_intensity;	
	int		m_nebula_color;
	int		m_nebula_index;
	int		m_bank;
	int		m_heading;
	int		m_pitch;
	CSliderCtrl			m_slider;
	int		m_neb2_texture;
	BOOL		m_subspace;
	BOOL		m_fullneb;
	int		m_poof_0;
	int		m_poof_1;
	int		m_poof_2;
	int		m_poof_3;
	int		m_poof_4;
	int		m_poof_5;
	BOOL	m_toggle_trails;
	CString	m_storm_name;
	CString s_name;
	int s_pitch;
	int s_bank;
	int s_heading;
	float s_scale;
	int s_index;
	CString b_name;
	int b_pitch;
	int b_bank;
	int b_heading;
	float b_scale_x;
	float b_scale_y;
	int b_div_x;
	int b_div_y;
	int b_index;
	int m_sky_flag_1;
	int m_sky_flag_2;
	int m_sky_flag_3;
	int m_sky_flag_4;
	int m_sky_flag_5;
	int m_sky_flag_6;
	CString m_skybox_model;
	CString m_envmap;
	int m_skybox_pitch;
	int m_skybox_bank;
	int m_skybox_heading;
	float m_neb_near_multi;
	float m_neb_far_multi;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(bg_bitmap_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// clear and build the nebula filename list appropriately
	void build_nebfile_list();

	int get_active_background();
	int get_swap_background();
	void reinitialize_lists();

	void OnOrientationChange();

	// Generated message map functions
	//{{AFX_MSG(bg_bitmap_dlg)
	afx_msg void OnClose();
	afx_msg void OnCancel();	
	afx_msg void OnOK();	
	afx_msg void OnSelchangeNebcolor();
	afx_msg void OnSelchangeNebpattern();
	afx_msg void OnFullNeb();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSunChange();
	afx_msg void OnAddSun();
	afx_msg void OnDelSun();
	afx_msg void OnSunDropdownChange();
	afx_msg void OnBitmapChange();
	afx_msg void OnAddBitmap();
	afx_msg void OnDelBitmap();
	afx_msg void OnBitmapDropdownChange();
	afx_msg void OnDeltaposSbitmapPSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSbitmapBSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSbitmapHSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusSbitmapScaleX();
	afx_msg void OnKillfocusSbitmapScaleY();
	afx_msg void OnKillfocusSbitmapDivX();
	afx_msg void OnKillfocusSbitmapDivY();
	afx_msg void OnKillfocusSbitmapP();
	afx_msg void OnKillfocusSbitmapB();
	afx_msg void OnKillfocusSbitmapH();
	afx_msg void OnDeltaposSun1PSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSun1HSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSun1BSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusSun1P();
	afx_msg void OnKillfocusSun1H();
	afx_msg void OnKillfocusSun1B();
	afx_msg void OnKillfocusSun1Scale();
	afx_msg void OnImportBackground();
	afx_msg void OnSwapBackground();
	afx_msg void OnBackgroundDropdownChange();
	afx_msg void OnSkyboxBrowse();
	afx_msg void OnDeltaposSkyboxPSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSkyboxBSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSkyboxHSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusSkyboxP();
	afx_msg void OnKillfocusSkyboxB();
	afx_msg void OnKillfocusSkyboxH();
	afx_msg void OnEnvmapBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:	
};

#endif
