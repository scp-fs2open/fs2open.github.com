#ifndef _PROPDLG_H
#define _PROPDLG_H

class prop_dlg : public CDialog {
	// Construction
  public:
	int bypass_errors;
	BOOL Create();
	int update_data();
	void initialize_data(int full_update);
	void OnOK();
	prop_dlg(CWnd* pParent = NULL); // standard constructor

	// Dialog Data
	enum {
		IDD = IDD_PROP_EDITOR
	}; // You’ll define this in the resource editor
	CString m_name;
	CListBox m_flags_list;

	// Overrides
  protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	void select_prop_from_object_list(object* start, bool forward);

	// Implementation
  protected:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPropNext();
	afx_msg void OnPropPrev();
	DECLARE_MESSAGE_MAP()
};

#endif
