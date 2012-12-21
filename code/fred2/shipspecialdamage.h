/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#if !defined(AFX_SHIPSPECIALDAMAGE_H__89610237_C0F4_11D2_A8B6_0060088FAE88__INCLUDED_)
#define AFX_SHIPSPECIALDAMAGE_H__89610237_C0F4_11D2_A8B6_0060088FAE88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShipSpecialDamage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ShipSpecialDamage dialog

class ShipSpecialDamage : public CDialog
{
// Construction
public:
	ShipSpecialDamage(CWnd* pParent = NULL);   // standard constructor
	void update_ship(int ship);

// Dialog Data
	//{{AFX_DATA(ShipSpecialDamage)
	enum { IDD = IDD_SPECIAL_DAMAGE };
	int		m_shock_enabled;
	int		m_duration_enabled;
	BOOL	m_special_exp_enabled;
	int		m_inner_rad;
	int		m_outer_rad;
	int		m_damage;
	int		m_shock_speed;
	int		m_duration;
	int		m_blast;
	int		m_ship_num;

private:
	// variables to handle selection of multiple ships
	int m_selected_ships[MAX_SHIPS];
	int num_selected_ships;

	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ShipSpecialDamage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ShipSpecialDamage)
	afx_msg void OnEnableShockwave();
	afx_msg void OnEnableDeathrollTime();
	afx_msg void OnEnableSpecialExp();
	virtual BOOL OnInitDialog();
	afx_msg void DoGray();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHIPSPECIALDAMAGE_H__89610237_C0F4_11D2_A8B6_0060088FAE88__INCLUDED_)
