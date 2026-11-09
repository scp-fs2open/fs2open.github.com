// reorderdlg.h : header file

#ifndef _REORDERDLG_H
#define _REORDERDLG_H

class reorder_dlg : public CDialog
{
public:
	reorder_dlg(CWnd *pParent = nullptr);

	// Dialog Data
	//{{AFX_DATA(reorder_dlg)
	enum { IDD = IDD_REORDER };
	CComboBox	m_type_combo;
	CListBox	m_list;
	//}}AFX_DATA

protected:
	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

	//{{AFX_MSG(reorder_dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeReorderType();
	afx_msg void OnSelchangeReorderList();
	afx_msg void OnMoveToTop();
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnMoveToBottom();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	bool ships_selected() const;
	void populate_list();
	void update_buttons();
	void move_selected(bool up, bool all_the_way);

	// occupied Ships[] or Wings[] slot indexes, in list order
	SCP_vector<int> m_slots;
};
#endif	// _REORDERDLG_H
