#if !defined(AFX_TABREGOPTIONS_H__9C7C6E18_A6CC_4BC5_A680_8A17C8C90BEE__INCLUDED_)
#define AFX_TABREGOPTIONS_H__9C7C6E18_A6CC_4BC5_A680_8A17C8C90BEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabRegOptions.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabRegOptions dialog

class CTabRegOptions : public CDialog
{
// Construction
public:
	void FillRegList();
	void InsertItem(char *name, int type, void *data, DWORD size);
	CTabRegOptions(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTabRegOptions)
	enum { IDD = IDD_REG_OPTIONS };
	CListCtrl	m_reg_option_list;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabRegOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabRegOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnSet();
	afx_msg void OnItemchangedRegList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_list_count;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABREGOPTIONS_H__9C7C6E18_A6CC_4BC5_A680_8A17C8C90BEE__INCLUDED_)
