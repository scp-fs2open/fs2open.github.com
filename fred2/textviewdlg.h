/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// TextViewDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// TextViewDlg dialog

class TextViewDlg : public CDialog
{
// Construction
public:
	TextViewDlg(CWnd* pParent = nullptr);   // standard constructor

	void LoadShipsTblText(const ship_info *sip);
	void LoadMusicTblText();
	void SetText(const CString &text);
	void GetText(CString &text);

	void SetCaption(const CString &caption);
	void SetEditable(bool editable);

// Dialog Data
	//{{AFX_DATA(TextViewDlg)
	enum { IDD = IDD_TEXT_VIEW };
	CString	m_edit;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TextViewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(TextViewDlg)
	afx_msg BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSetfocusEdit1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CString m_original_text;
	CString m_caption;
	bool m_editable;
};
