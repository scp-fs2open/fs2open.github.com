#pragma once
#include "resource.h"

class volumetrics_dlg : public CDialog
{

public:
	volumetrics_dlg(CWnd* pParent = nullptr);   
	virtual ~volumetrics_dlg();

	enum { IDD = IDD_VOLUMETRICS };

	afx_msg void OnClose();

protected:
	CToolTipCtrl m_toolTip;

	BOOL m_enabled;
	CString m_volumetrics_hull;
	vec3d m_position;
	std::array<int, 3> m_color;
	float m_opacity;
	float m_distance;
	int m_steps;

	virtual void DoDataExchange(CDataExchange* pDX) override; 
	virtual BOOL OnInitDialog() override; 
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;
	virtual void OnCancel() override {}

	void handle_spinner_vec3d(LPNMUPDOWN spinner, vec3d& vec, float decltype(vec3d::xyz)::*dimension);
	void handle_spinner_color(LPNMUPDOWN spinner, std::array<int, 3>& color, size_t idx);
	template<typename T> void handle_spinner(LPNMUPDOWN spinner, T& data)
	{
		UpdateData(TRUE);
		data -= static_cast<T>(spinner->iDelta);
		UpdateData(FALSE);
	}

	DECLARE_MESSAGE_MAP()
  public:
	afx_msg void OnBnClickedSetHull();
	afx_msg void OnDeltaposSpinPosX(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinPosY(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinPosZ(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposColorRSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposColorGSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposColorBSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinOpacity(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinOpacityDistance(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinSteps(NMHDR* pNMHDR, LRESULT* pResult);
};
