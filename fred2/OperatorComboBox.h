#ifndef _OPERATOR_COMBO_BOX_H
#define _OPERATOR_COMBO_BOX_H

#include "stdafx.h"
#include "globalincs/vmallocator.h"
#include "globalincs/pstypes.h"
#include "parse/sexp.h"

/////////////////////////////////////////////////////////////////////////////
// OperatorComboBox control

class OperatorComboBoxList : public CListBox
{
public:
	OperatorComboBoxList(const char* (*help_callback)(int), int opf_type);

	BOOL OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	BOOL IsItemEnabled(UINT) const;

	void SetOpfType(int opf_type);

protected:
	const char* (*m_help_callback)(int);
	CStringA m_tooltiptextA;
	CStringW m_tooltiptextW;

	int m_expected_opr_type;

	DECLARE_MESSAGE_MAP()
};

class OperatorComboBox : public CComboBox
{
public:
	OperatorComboBox(const char* (*help_callback)(int));
	virtual ~OperatorComboBox();
	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

	// we need to reference the components of the combo box
	CEdit                  m_edit;
	OperatorComboBoxList   m_listbox;

	// for tooltips
	INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	void refresh_popup_operators(int opf_type = OPF_NONE);
	int GetOpConst(int index) const;
	bool IsItemEnabled(int index) const;

protected:
	virtual void PreSubclassWindow();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();

	SCP_vector<std::pair<SCP_string, int>> m_sorted_operators;
	const char* (*m_help_callback)(int);

	DECLARE_MESSAGE_MAP()
};

#endif