#if !defined(_LISTITEM_CHOOSER_H_)
#define _LISTITEM_CHOOSER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListItemChooser.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ListItemChooser dialog

class ListItemChooser : public CDialog
{
	// Construction
public:
	ListItemChooser(const SCP_vector<SCP_string>& listItems);   // standard constructor
	void OnOK();
	void OnCancel();

	int GetChosenIndex();

	// Dialog Data
		//{{AFX_DATA(ListItemChooser)
	enum { IDD = IDD_LISTITEM_CHOOSER };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ListItemChooser)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	SCP_vector<SCP_string> m_listItems;
	int m_chosenItem;

	// Generated message map functions
	//{{AFX_MSG(ListItemChooser)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(_LISTITEM_CHOOSER_H_)
