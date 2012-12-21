// ShipSpecialHitpoints.h
// Goober5000

#if !defined(AFX_SHIPSPECIALHITPOINTS_H__D612D530_131C_4339_A417_AFCAA090258A__INCLUDED_)
#define AFX_SHIPSPECIALHITPOINTS_H__D612D530_131C_4339_A417_AFCAA090258A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShipSpecialHitpoints.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ShipSpecialHitpoints dialog

class ShipSpecialHitpoints : public CDialog
{
// Construction
public:
	ShipSpecialHitpoints(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ShipSpecialHitpoints)
	enum { IDD = IDD_SPECIAL_HITPOINTS };
	BOOL	m_special_hitpoints_enabled;
	BOOL	m_special_shield_enabled;
	int		m_shields;
	int		m_hull;
	//}}AFX_DATA

private:
	// variables to handle selection of multiple ships
	int m_selected_ships[MAX_SHIPS];
	int num_selected_ships;
	int m_ship_num;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ShipSpecialHitpoints)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ShipSpecialHitpoints)
	afx_msg void OnEnableSpecialHitpoints();
	afx_msg void OnEnableSpecialShieldpoints();
	virtual BOOL OnInitDialog();
	afx_msg void DoGray();
	virtual void OnCancel();
	virtual void OnOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHIPSPECIALHITPOINTS_H__D612D530_131C_4339_A417_AFCAA090258A__INCLUDED_)
