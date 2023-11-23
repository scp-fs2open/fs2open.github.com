
// CheckBoxListDlg.h : header file
//

#include <globalincs/vmallocator.h>
#include "shipchecklistbox.h"


/////////////////////////////////////////////////////////////////////////////
// CheckBoxListDlg dialog

class CheckBoxListDlg : public CDialog
{
// Construction
public:
	CheckBoxListDlg(CWnd* pParent = nullptr);   // standard constructor

	void SetOptions(const SCP_vector<CString> &options);
	void SetOptions(const SCP_vector<std::pair<CString, bool>> &options);

	bool IsChecked(int index);
	void SetChecked(int index, bool checked);

	void SetCaption(const CString &caption);

// Dialog Data
	//{{AFX_DATA(CheckBoxListDlg)
	enum { IDD = IDD_CHECKLIST_VIEW };
	ShipCheckListBox m_checklist;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CheckBoxListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CheckBoxListDlg)
	afx_msg BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	SCP_vector<std::pair<CString, bool>> m_offline_options;
	CString m_caption;
};
