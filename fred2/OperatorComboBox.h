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
	OperatorComboBoxList(const char* (*help_callback)(int), sexp_opf_t opf_type);

	BOOL OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	BOOL IsItemEnabled(UINT) const;

	sexp_opf_t GetOpfType() const;
	void SetOpfType(sexp_opf_t opf_type);

protected:
	const char* (*m_help_callback)(int);
	CStringA m_tooltiptextA;
	CStringW m_tooltiptextW;

	sexp_opf_t m_opf_type;

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

	// for tracking key presses
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// for tooltips
	INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	void refresh_popup_operators(sexp_opf_t opf_type = OPF_NONE, const SCP_string &filter_string = "");
	void filter_popup_operators(const SCP_string &filter_string = "");
	void cleanup(bool confirm);

	bool PressedEnter() const;
	int GetOpIndex(int index) const;
	bool IsItemEnabled(int index) const;

protected:
	virtual void PreSubclassWindow();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEditChange();

	const char* (*m_help_callback)(int);
	bool m_pressed_enter;

	DECLARE_MESSAGE_MAP()
};

#endif