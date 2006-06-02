// AltShipClassDlg.h
// Karajorma
#if !defined(AFX_ALTSHIPCLASSDLG_H__90E85121_BB19_4E95_A46F_C8AD22CD80A6__INCLUDED_)
#define AFX_ALTSHIPCLASSDLG_H__90E85121_BB19_4E95_A46F_C8AD22CD80A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AltShipClassDlg.h : header file
//

#define ALT_SHIP_CLASS_COMBO_OFFSET	1

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
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

private:

	
	CComboBox *type1_class_selections[MAX_ALT_CLASS_1];
	CComboBox *type1_variable_selections[MAX_ALT_CLASS_1];
	CComboBox *type2_class_selections[MAX_ALT_CLASS_2];
	CComboBox *type2_variable_selections[MAX_ALT_CLASS_2];

	int type1_radio_button_selection[MAX_ALT_CLASS_1];
	int type1_class_radio_button[MAX_ALT_CLASS_1];
	int type1_variable_radio_button[MAX_ALT_CLASS_1];

	int type2_radio_button_selection[MAX_ALT_CLASS_2];
	int type2_class_radio_button[MAX_ALT_CLASS_2];
	int type2_variable_radio_button[MAX_ALT_CLASS_2];

	bool multi_edit;
//	int m_ASCT1_group_one ;
	int m_selected_ships[MAX_SHIPS];
	int num_selected_ships;

	void DisableComponents();

	int GetVariableIndexFromControl(CComboBox *variable_ptr);
	int GetStringVariableIndex(int sexp_variable_index);

	void FillClassComboBox(CComboBox *ptr);
	void FillVariableComboBox(CComboBox *ptr); 
	void SetupType1ComboBoxes(CComboBox *class_ptr, CComboBox *variable_ptr, int alt_class_index);
	void SetupType2ComboBoxes(CComboBox *class_ptr, CComboBox *variable_ptr, int alt_class_index);

	void WriteType1DataToShip(CComboBox *class_ptr, CComboBox *variable_ptr, int alt_class_index);
	void WriteType2DataToShip(CComboBox *class_ptr, CComboBox *variable_ptr, int alt_class_index);



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
	afx_msg void OnASCT1ClassRadio();
	afx_msg void OnASCT1VariablesRadio();
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnAsct1ClassRadio2();
	afx_msg void OnAsct1ClassRadio3();
	afx_msg void OnAsct1VariablesRadio2();
	afx_msg void OnAsct1VariablesRadio3();
	afx_msg void OnAsct2ClassRadio1();
	afx_msg void OnAsct2VariablesRadio1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALTSHIPCLASSDLG_H__90E85121_BB19_4E95_A46F_C8AD22CD80A6__INCLUDED_)
