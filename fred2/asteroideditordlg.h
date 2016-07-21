/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// AsteroidEditorDlg.h : header file
//

#include "starfield/starfield.h"
#include "asteroid/asteroid.h"


/////////////////////////////////////////////////////////////////////////////
// asteroid_editor dialog

class asteroid_editor : public CDialog
{
// Construction
public:
	void update_init();
	int query_modified();
	void OnCancel();
	void OnOK();
	void OnEnableField();
	int validate_data();

	asteroid_editor(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(asteroid_editor)
	enum { IDD = IDD_ASTEROID_EDITOR };
	CSpinButtonCtrl	m_density_spin;
	int				m_avg_speed;
	int				m_density;
	BOOL				m_enable_asteroids;
	CString			m_max_x;
	CString			m_max_y;
	CString			m_max_z;
	CString			m_min_x;
	CString			m_min_y;
	CString			m_min_z;
	BOOL				m_enable_inner_bounds;
	field_type_t	m_field_type;		// active or passive
	debris_genre_t	m_debris_genre;		// ship or asteroid
	int				m_field_debris_type[3];	// species and size of ship debris
	CString			m_box_max_x;
	CString			m_box_max_y;
	CString			m_box_max_z;
	CString			m_box_min_x;
	CString			m_box_min_y;
	CString			m_box_min_z;
	//}}AFX_DATA

	int cur_field, last_field;
	asteroid_field a_field[1 /*MAX_ASTEROID_FIELDS*/];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(asteroid_editor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(asteroid_editor)
	virtual BOOL OnInitDialog();
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnEnableAsteroids();
	afx_msg void OnClose();
	afx_msg void OnEnableInnerBox();
	afx_msg void OnPassiveField();
	afx_msg void OnFieldShip();
	afx_msg void OnActiveField();
	afx_msg void OnFieldAsteroid();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
