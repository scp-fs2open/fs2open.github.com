/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/WeaponEditorDlg.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:32 $
 * $Author: Goober5000 $
 *
 * Weapon editor dialog box handling code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.3  2004/09/29 17:26:33  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:03  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:02p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 8     2/22/98 1:32a Hoffoss
 * Changed editor to use raw ammo counts isntead of percentages, and make
 * it select first item in list by default.
 * 
 * 7     5/30/97 4:50p Hoffoss
 * Added code to allow marked ship editing of data in child dialogs of
 * ship editor dialog.
 * 
 * 6     4/21/97 5:02p Hoffoss
 * Player/player status editing supported, and both saved and loaded from
 * Mission files.
 * 
 * 5     4/16/97 1:59p Hoffoss
 * Weapon editor now fully functional.
 * 
 * 4     4/09/97 11:48a Hoffoss
 * Initial work to weapon editor.
 * 
 * 3     3/31/97 6:07p Hoffoss
 * Fixed several errors, including BG editor not graying fields, BG editor
 * not updating image when changed, Removed obsolete data from Weapon
 * editor, priority not being saved when missions saved, priority not
 * editable in initial orders editor.
 * 
 * 2     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

#include "ship/ship.h"

/////////////////////////////////////////////////////////////////////////////
// WeaponEditorDlg dialog

class WeaponEditorDlg : public CDialog
{
// Construction
public:
	void update_pilot();
	void OnCancel();
	void change_selection();
	void OnOK();
	WeaponEditorDlg(CWnd* pParent = NULL);   // standard constructor

	int m_ammo_max1;
	int m_ammo_max2;
	int m_ammo_max3;
	int m_ammo_max4;
	int m_last_item;
	int m_ship;
	int m_ship_class;
	int m_multi_edit;
	ship_weapon pilot, *cur_weapon;

// Dialog Data
	//{{AFX_DATA(WeaponEditorDlg)
	enum { IDD = IDD_WEAPON_EDITOR };
	CSpinButtonCtrl	m_spin4;
	CSpinButtonCtrl	m_spin3;
	CSpinButtonCtrl	m_spin2;
	CSpinButtonCtrl	m_spin1;
	int		m_ai_class;
	int		m_ammo1;
	int		m_ammo2;
	int		m_ammo3;
	int		m_ammo4;
	int		m_gun1;
	int		m_gun2;
	int		m_gun3;
	int		m_missile1;
	int		m_missile2;
	int		m_missile3;
	int		m_missile4;
	int		m_cur_item;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WeaponEditorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(WeaponEditorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList();
	afx_msg void OnClose();
	afx_msg void OnSelchangeMissile1();
	afx_msg void OnSelchangeMissile2();
	afx_msg void OnSelchangeMissile3();
	afx_msg void OnSelchangeMissile4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
