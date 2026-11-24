#ifndef _PROPDLG_H
#define _PROPDLG_H

#include "sexp_tree_view.h"

class prop_dlg : public CDialog {
	// Construction
  public:
	int bypass_errors;
	BOOL Create();
	int update_data(int redraw = 1);
	void initialize_data(int full_update);
	void OnOK();
	prop_dlg(CWnd* pParent = NULL); // standard constructor

	// Dialog Data
	enum {
		IDD = IDD_PROP_EDITOR
	}; // You’ll define this in the resource editor
	CString m_name;
	CListBox m_flags_list;
	int m_spawn_delay;
	int m_despawn_delay;
	sexp_tree_view m_spawn_tree;
	sexp_tree_view m_despawn_tree;
	int m_cues_modified;
	int select_sexp_node;

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
	afx_msg void OnRclickSpawnTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickDespawnTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditSpawnTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditDespawnTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditSpawnTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditDespawnTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedSpawnTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedDespawnTree(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

#endif
