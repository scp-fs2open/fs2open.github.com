/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Fred2/BgBitmapDlg.h $
 * $Revision: 1.3 $
 * $Date: 2006-08-06 18:47:29 $
 * $Author: Goober5000 $
 *
 * Background space images manager dialog
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/04/20 06:32:01  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.9  2005/12/21 00:28:43  phreak
 * Import background button for mission bitmaps.
 *
 * Revision 1.8  2005/07/25 06:51:33  Goober5000
 * mission flag changes for FRED
 * --Goober5000
 *
 * Revision 1.7  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.6  2005/04/12 02:18:52  phreak
 * Ambient lighting sliders now work in the background editor.
 *
 * Revision 1.5  2005/03/29 03:13:06  phreak
 * Made some changes to the background bitmap dialog to show the user updates
 * whenever any information on the position or size changes, or another bitmap is
 * used instead of whats currently selected.  The spinners for the bitmaps also wrap
 * from 359 - > 0 and from 0 -> 359.  These enhancements should make creating
 * backgrounds much quicker and easier.
 *
 * Also fixed a bug where if a user entered "39" in either the pitch, bank, or heading
 * boxes, then the "39" would be set to "38" due to truncation from converting between
 * radians and degrees.
 *
 * Revision 1.4  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.3  2003/10/23 23:52:26  phreak
 * changed the background bitmap dialog so the user can specify custom skyboxes
 *
 * Revision 1.2  2003/01/13 02:07:26  wmcoolmon
 * Removed the override checkbox + flag for ship trails and added a checkbox + flag to disable ship trails within the nebula, NO_NEB_TRAILS, to the Background dialog.
 *
 * Revision 1.1.1.1  2002/07/15 03:10:52  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 8     7/02/99 4:30p Dave
 * Much more sophisticated lightning support.
 * 
 * 7     6/03/99 6:37p Dave
 * More TNT fun. Made perspective bitmaps more flexible.
 * 
 * 6     4/26/99 8:47p Dave
 * Made all pof related nebula stuff customizable through Fred.
 * 
 * 5     4/07/99 6:21p Dave
 * Fred and FreeSpace support for multiple background bitmaps and suns.
 * Fixed link errors on all subprojects. Moved encrypt_init() to
 * cfile_init() and lcl_init(), since its safe to call twice.
 * 
 * 4     1/25/99 5:03a Dave
 * First run of stealth, AWACS and TAG missile support. New mission type
 * :)
 * 
 * 3     11/14/98 5:37p Dave
 * Put in support for full nebulas.
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:01p Dave
 * 
 * 1     10/07/98 2:59p Dave
 * 
 * 12    4/13/98 10:25p Hoffoss
 * Added a flag for subspace missions, and for aboard the Galatea or
 * Bastion.
 * 
 * 11    12/08/97 4:48p Hoffoss
 * Moved starfield editor controls to background editor.
 * 
 * 10    11/25/97 11:40a Hoffoss
 * Added support for nebula placement editing.
 * 
 * 9     11/21/97 2:55p Hoffoss
 * Added Nebula support to Fred.  Implemented loading and saving nebula
 * info to/from mission files.
 * 
 * 8     3/31/97 6:07p Hoffoss
 * Fixed several errors, including BG editor not graying fields, BG editor
 * not updating image when changed, Removed obsolete data from Weapon
 * editor, priority not being saved when missions saved, priority not
 * editable in initial orders editor.
 * 
 * 7     3/27/97 2:24p Hoffoss
 * Fixed bug in image not updating when new image selected from listbox of
 * combo box.
 * 
 * 6     3/21/97 4:24p Hoffoss
 * Fixed bug in changing image to an external image file.
 * 
 * 5     3/17/97 1:54p Hoffoss
 * fixed bugs in BG editor, and added delete button functionality.
 * 
 * 4     3/12/97 4:33p Hoffoss
 * added spin controls to orient editor, light intensity level can be
 * specified in BG editor.
 * 
 * 3     2/04/97 3:09p Hoffoss
 * Background bitmap editor implemented fully.
 * 
 * 2     1/30/97 2:24p Hoffoss
 * Added remaining mission file structures and implemented load/save of
 * them.
 *
 * $NoKeywords: $
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
	CString m_skybox_model;
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:	
};

#endif
