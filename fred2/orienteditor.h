/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "object/object.h"

/////////////////////////////////////////////////////////////////////////////
// orient_editor dialog

class orient_editor : public CDialog
{
// Construction
public:
	bool query_modified();
	void OnCancel();
	void OnOK();
	orient_editor(CWnd* pParent = NULL);   // standard constructor

	static float to_degrees(float radians);

// Dialog Data
	//{{AFX_DATA(orient_editor)
	enum { IDD = IDD_ORIENT_EDITOR };
	CSpinButtonCtrl	m_spin1;
	CSpinButtonCtrl	m_spin2;
	CSpinButtonCtrl	m_spin3;
	CSpinButtonCtrl	m_spin4;
	CSpinButtonCtrl	m_spin5;
	CSpinButtonCtrl	m_spin6;
	CSpinButtonCtrl	m_spin11;
	CSpinButtonCtrl	m_spin12;
	CSpinButtonCtrl	m_spin13;
	int	m_object_index;
	BOOL	m_point_to;
	CString	m_position_x;
	CString	m_position_y;
	CString	m_position_z;
	CString	m_location_x;
	CString	m_location_y;
	CString	m_location_z;
	CString	m_orientation_p;
	CString	m_orientation_b;
	CString	m_orientation_h;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(orient_editor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(orient_editor)
	virtual BOOL OnInitDialog();
	afx_msg void OnPointTo();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	float convert(const CString &str);
	bool close(float val, const CString &str);
	int total;
	int index[MAX_OBJECTS];
	void actually_point_object(object *ptr);
};
