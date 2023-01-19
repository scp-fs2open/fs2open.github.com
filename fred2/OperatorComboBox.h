#ifndef _OPERATOR_COMBO_BOX_H
#define _OPERATOR_COMBO_BOX_H

#include "stdafx.h"
#include "globalincs/vmallocator.h"

/////////////////////////////////////////////////////////////////////////////
// OperatorComboBox control

class OperatorComboBox : public CComboBox
{
public:
	OperatorComboBox(const char* (*help_callback)(int));
	virtual ~OperatorComboBox();

	// we need to reference the components of the combo box
	CEdit      m_edit;
	CListBox   m_listbox;

	// for tooltips
	INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	BOOL OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);

	void refresh_popup_operators();

protected:
	virtual void PreSubclassWindow();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();

	SCP_vector<std::pair<SCP_string, int>> m_sorted_operators;
	const char* (*m_help_callback)(int);

	DECLARE_MESSAGE_MAP()
};

#endif