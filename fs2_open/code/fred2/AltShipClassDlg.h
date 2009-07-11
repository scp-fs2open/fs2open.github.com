/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#if !defined(AFX_AltShipClassDlg_H__2FE2C917_C7DA_4C05_A11F_DCC46BFB26BB__INCLUDED_)
#define AFX_AltShipClassDlg_H__2FE2C917_C7DA_4C05_A11F_DCC46BFB26BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AltShipClassDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// AltShipClassDlg dialog

class AltShipClassDlg : public CDialog
{
// Construction
public:
	AltShipClassDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(AltShipClassDlg)
	enum { IDD = IDD_ALT_SHIP_CLASS };
	CButton	m_default_to_class;
	CListBox	m_alt_class_list;
	CComboBox	m_set_from_ship_class;
	CComboBox	m_set_from_variables;
	int		m_selected_variable;
	int		m_selected_class;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AltShipClassDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AltShipClassDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnAltClassAdd();
	afx_msg void OnAltClassInsert();
	afx_msg void OnAltClassDelete();
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnSelendokSetFromShipClass();
	afx_msg void OnSelendokSetFromVariables();
	afx_msg void OnSelchangeAltClassList();
	afx_msg void OnDefaultToClass();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	
	bool player_ships_only;		// Whether we only add player ships to the alt classes or if we can add any ship

	SCP_vector<alt_class> alt_class_pool;
	int num_string_variables;							// Number of string variables in the mission
	int string_variable_indices[MAX_SEXP_VARIABLES];	// maps string variables to their index in Sexp_variables
	int ship_class_indices[MAX_SHIP_CLASSES];		// maps ships in the ships combobox to their index in Ship_info

	
	// variables to handle selection of multiple ships
	bool multi_edit;
	int m_selected_ships[MAX_SHIPS];
	int num_selected_ships;

	void AltShipClassDlg::alt_class_list_rebuild();	
	void AltShipClassDlg::alt_class_update_entry(alt_class &list_item);	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AltShipClassDlg_H__2FE2C917_C7DA_4C05_A11F_DCC46BFB26BB__INCLUDED_)
